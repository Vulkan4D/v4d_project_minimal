#include <string>
#include <sstream>

#include <v4d.h>
#include <utilities/graphics/vulkan/Loader.h>
#include <utilities/graphics/Window.h>
#include <utilities/graphics/Renderer.h>
#include <utilities/graphics/vulkan/RasterShaderPipeline.h>
#include <utilities/graphics/vulkan/RenderPass.h>

class MyRenderer : public v4d::graphics::Renderer {
	
	// Synchronization objects
	FrameBuffered_Semaphore renderSemaphore;
	FrameBuffered_Semaphore presentSemaphore;
	FrameBuffered_Fence frameFence;

	// Command buffers
	FrameBuffered_CommandBuffer commandBuffer;

	// Render Pipelines, Shaders, Render passes
	v4d::graphics::vulkan::PipelineLayout renderPipelineLayout;
	v4d::graphics::vulkan::RasterShaderPipeline triangleShader {renderPipelineLayout, "assets/shaders/triangle"};
	v4d::graphics::vulkan::RenderPass renderPass;

	// Base Constructor
	using v4d::graphics::Renderer::Renderer;
	
	// Vulkan device setup (all of these extensions and features are not used in this example, they may be removed)
	virtual void ConfigureDeviceExtensions() override {
		// Device Extensions
		OptionalDeviceExtension(VK_KHR_SHADER_CLOCK_EXTENSION_NAME);
		OptionalDeviceExtension(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
		if (v4d::graphics::vulkan::Loader::VULKAN_API_VERSION >= VK_API_VERSION_1_2) {
			OptionalDeviceExtension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
			// RayTracing extensions
			OptionalDeviceExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
			OptionalDeviceExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
			OptionalDeviceExtension(VK_KHR_RAY_QUERY_EXTENSION_NAME);
			OptionalDeviceExtension(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
			OptionalDeviceExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
			OptionalDeviceExtension(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
		}
	}
	virtual void ScorePhysicalDeviceSelection(int& score, v4d::graphics::PhysicalDevice* device) {}
	virtual void ConfigureDeviceFeatures(v4d::graphics::PhysicalDevice::DeviceFeatures* deviceFeaturesToEnable, const v4d::graphics::PhysicalDevice::DeviceFeatures* availableDeviceFeatures) override {
		V4D_ENABLE_DEVICE_FEATURES(
			deviceFeatures2.features.shaderFloat64,
			deviceFeatures2.features.shaderInt64,
			deviceFeatures2.features.shaderInt16,
			deviceFeatures2.features.depthClamp,
			deviceFeatures2.features.fillModeNonSolid,
			deviceFeatures2.features.geometryShader,
			deviceFeatures2.features.wideLines,
			deviceFeatures2.features.largePoints,
			deviceFeatures2.features.shaderTessellationAndGeometryPointSize,
			shaderClockFeatures.shaderDeviceClock,
			shaderClockFeatures.shaderSubgroupClock,
			_16bitStorageFeatures.storageBuffer16BitAccess
		)
		// Vulkan 1.2
		if (v4d::graphics::vulkan::Loader::VULKAN_API_VERSION >= VK_API_VERSION_1_2) {
			V4D_ENABLE_DEVICE_FEATURES(
				vulkan12DeviceFeatures.bufferDeviceAddress,
				vulkan12DeviceFeatures.shaderFloat16,
				vulkan12DeviceFeatures.shaderInt8,
				vulkan12DeviceFeatures.descriptorIndexing,
				vulkan12DeviceFeatures.storageBuffer8BitAccess,
				vulkan12DeviceFeatures.shaderOutputLayer,
				// Ray-tracing features
				accelerationStructureFeatures.accelerationStructure,
				accelerationStructureFeatures.descriptorBindingAccelerationStructureUpdateAfterBind,
				rayTracingPipelineFeatures.rayTracingPipeline,
				rayTracingPipelineFeatures.rayTracingPipelineTraceRaysIndirect,
				rayQueryFeatures.rayQuery
			)
		} else {
			LOG_WARN("Vulkan 1.2 is not supported")
		}
	}
	
	// Renderer configuration
	virtual void ConfigureRenderer() override {}
	virtual void ConfigureLayouts() override {}
	virtual void ConfigureShaders() override {
		triangleShader.SetData(3);
		#ifdef _DEBUG
			WatchModifiedShadersForReload({
				triangleShader,
			});
		#endif
	}
	virtual void ReadShaders() override {
		triangleShader.ReadShaders();
	}

	// Command buffers
	virtual void CreateCommandBuffers() override {
		commandBuffer.Allocate(renderingDevice);
	}
	virtual void DestroyCommandBuffers() override {
		commandBuffer.Free(renderingDevice);
	}

	// Synchronization objects
	virtual void CreateSyncObjects() override {
		renderSemaphore.Create(renderingDevice);
		presentSemaphore.Create(renderingDevice);
		frameFence.Create(renderingDevice);
	}
	virtual void DestroySyncObjects() override {
		renderSemaphore.Destroy(renderingDevice);
		presentSemaphore.Destroy(renderingDevice);
		frameFence.Destroy(renderingDevice);
	}

	// Resources & Buffers
	virtual void CreateResources() override {}
	virtual void DestroyResources() override {}
	virtual void AllocateBuffers() override {}
	virtual void FreeBuffers() override {}

	// Pipelines
	virtual void CreatePipelines() override {
		// Pipeline layout
		renderPipelineLayout.Create(renderingDevice);
		
		// FrameBuffers
		renderPass.SetFrameBufferCount(swapChain->images.size());
		
		// Fragment shader output attachments
		uint32_t out_color = renderPass.AddAttachment(swapChain);
		
		// SubPasses
		v4d::graphics::vulkan::RenderPass::Subpass subpass {};
			subpass.colorAttachments = renderPass.AddColorAttachmentRefs({{out_color, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}});
		renderPass.AddSubpass(subpass, VK_SUBPASS_EXTERNAL);
		
		// Create the render pass
		renderPass.Create(renderingDevice, swapChain->extent.width, swapChain->extent.height);
		
		// Shaders
		triangleShader.SetRenderPass(swapChain, renderPass.handle);
		triangleShader.AddColorBlendAttachmentState();
		triangleShader.CreatePipeline(renderingDevice);
	}
	virtual void DestroyPipelines() override {
		triangleShader.DestroyPipeline(renderingDevice);
		renderPass.Destroy();
		renderPipelineLayout.Destroy(renderingDevice);
	}

public:

	// Render Commands
	virtual void Render() override {
		if(!BeginFrame(renderSemaphore[currentFrame])) return;
		
		// Frame Synchronization
		WaitForFence(frameFence[currentFrame]);
		
		// Render
		RecordAndSubmitCommandBuffer(commandBuffer[currentFrame],
			[&](auto cmdBuffer){// Commands to record
				
				renderPass.Begin(cmdBuffer, swapChainImageIndex);
				
					// Draw triangle
					triangleShader.Execute(renderingDevice, cmdBuffer);
					
				renderPass.End(cmdBuffer);
				
			},
			{// Wait Semaphores
				renderSemaphore[currentFrame],
			},
			{// Wait stages
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			},
			{// Signal Semaphores
				presentSemaphore[currentFrame],
			},
			// Trigger fence
			frameFence[currentFrame]
		);
	
		// Present on screen
		EndFrame({presentSemaphore[currentFrame]});
	}
};

int main(const int argc, const char** argv) {
	LOG("Application started")
	
	// Load V4D Core
	if (!v4d::Init()) throw std::runtime_error("Failed to init V4D Core");
	
	// Load vulkan driver dynamically
	v4d::graphics::vulkan::Loader vulkanLoader;
	if(!vulkanLoader()) throw std::runtime_error("Failed to load Vulkan library");
	
	// Create a window
	v4d::graphics::Window window("TEST", 1280, 720);
	
	// Create a Renderer (vulkan instance)
	window.FillRequiredVulkanInstanceExtensions(vulkanLoader.requiredInstanceExtensions);
	MyRenderer renderer(&vulkanLoader, "V4D minimal project", VK_MAKE_VERSION(1, 0, 0));
	renderer.InitRenderer();
	
	// Input callbacks
	window.AddKeyCallback("default", [&](int key, int scancode, int action, int mods){
		switch (key) {
			// Quit on ESC key
			case GLFW_KEY_ESCAPE:
				window.Close();
				break;
			// Reload renderer
			case GLFW_KEY_R:
				if (action == GLFW_PRESS) {
					renderer.RunSynchronized([&renderer](){renderer.ReloadRenderer();});
				}
				break;
		}
	});
	
	// Create presentation surface
	renderer.AssignSurface(window.CreateVulkanSurface(renderer.handle));
	
	// Load renderer
	renderer.LoadRenderer();
	
	// Game Loop
	while(window.IsActive()) {
		
		{// FPS Counter
			static v4d::FPSCounter fps;
			window.SetTitle(std::to_string(int(fps.Tick())) + " FPS");
		}
		
		// Poll Window Events
		glfwPollEvents();
		
		// Render on screen
		renderer.Render();
	}
	
	// Unload Renderer
	renderer.UnloadRenderer();
	
	LOG("Application terminated")
}
