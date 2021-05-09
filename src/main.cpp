#include <string>
#include <sstream>

#include <v4d.h>
#include <utilities/graphics/vulkan/Loader.h>
#include <utilities/graphics/Window.h>
#include <utilities/graphics/Renderer.h>

class MyRenderer : public v4d::graphics::Renderer {
	
	// Synchronization objects
	FrameBufferedObject<v4d::graphics::vulkan::SemaphoreObject> renderSemaphore;
	FrameBufferedObject<v4d::graphics::vulkan::SemaphoreObject> presentSemaphore;
	FrameBufferedObject<v4d::graphics::vulkan::FenceObject> frameFence;

	// Command buffers
	FrameBufferedObject<v4d::graphics::vulkan::CommandBufferObject> commandBuffer;
	
	// Pipeline Layouts
	v4d::graphics::vulkan::PipelineLayoutObject renderPipelineLayout;

	// Shaders
	v4d::graphics::vulkan::RasterShaderPipelineObject triangleShader {&renderPipelineLayout, "assets/shaders/triangle"};

	// Render passes
	v4d::graphics::vulkan::RenderPassObject renderPass;
	
	// Base Constructor
	using v4d::graphics::Renderer::Renderer;
	
	// Vulkan device setup
	void ConfigureDeviceExtensions() override {}
	void ScorePhysicalDeviceSelection(int& score, v4d::graphics::PhysicalDevice* device) override {}
	void ConfigureDeviceFeatures(v4d::graphics::PhysicalDevice::DeviceFeatures* deviceFeaturesToEnable, const v4d::graphics::PhysicalDevice::DeviceFeatures* availableDeviceFeatures) override {}
	
	// Renderer configuration
	void ConfigureRenderer() override {}
	void ConfigureLayouts() override {}
	
	// Shaders
	void ConfigureShaders() override {
		triangleShader.SetData(3);
		triangleShader.SetColorBlendAttachmentStates(1);
		#ifdef _DEBUG
			WatchModifiedShadersForReload({
				triangleShader,
			});
		#endif
	}

	// Render passes
	void ConfigureRenderPasses() override {
		renderPass.ConfigureFrameBuffers(swapChain);
		
		// Fragment shader output attachments
		uint32_t out_color = renderPass.AddAttachment(swapChain);
		
		// SubPasses
		v4d::graphics::vulkan::RenderPassObject::Subpass subpass {};
			subpass.colorAttachments = renderPass.AddColorAttachmentRefs({
				{out_color, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}
			});
		renderPass.AddSubpass(subpass, VK_SUBPASS_EXTERNAL);
		
		// Shaders
		renderPass.AddShader(&triangleShader, swapChain);
	}
	
	// Resources
	void AllocateResources() override {}
	void FreeResources() override {}

public:

	// Render Commands
	void Render() override {
		if(!BeginFrame(renderSemaphore[currentFrame])) return;
		
		// Frame Synchronization
		WaitForFence(frameFence[currentFrame]);
		
		// Render
		RecordAndSubmitCommandBuffer(commandBuffer[currentFrame],
			[&](auto cmdBuffer){// Commands to record
				
				renderPass.Begin(cmdBuffer, swapChainImageIndex);
				
					// Draw triangle
					triangleShader.Execute(cmdBuffer);
					
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
