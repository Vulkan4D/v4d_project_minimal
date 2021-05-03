#include <string>
#include <iostream>
#include <sstream>

#include <v4d.h>
#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/Window.h"
#include "utilities/graphics/Renderer.h"
#include "utilities/graphics/vulkan/Image.h"
#include "utilities/graphics/vulkan/RasterShaderPipeline.h"
#include "utilities/graphics/vulkan/RenderPass.h"


#define CALCULATE_AVG_FRAMERATE(varRef) {\
	static v4d::Timer t(true);\
	static double elapsedTime = 0.01;\
	static int nbFrames = 0;\
	++nbFrames;\
	elapsedTime = t.GetElapsedSeconds();\
	if (elapsedTime > 1.0) {\
		varRef = nbFrames / elapsedTime;\
		nbFrames = 0;\
		t.Reset();\
	}\
}

class MyRenderer : public v4d::graphics::Renderer {
public:
	
	static constexpr int NB_FRAMES_IN_FLIGHT = 2;
	uint32_t currentFrameInFlight = 0;

	// Semaphores
	std::array<VkSemaphore, NB_FRAMES_IN_FLIGHT> renderSemaphore;
	std::array<VkSemaphore, NB_FRAMES_IN_FLIGHT> presentSemaphore;

	// Fences
	std::array<VkFence, NB_FRAMES_IN_FLIGHT> frameFence;

	// Images
	std::array<v4d::graphics::Image, NB_FRAMES_IN_FLIGHT> renderTarget;

	// Render Pipelines
	v4d::graphics::vulkan::PipelineLayout renderPipelineLayout;
	v4d::graphics::vulkan::RasterShaderPipeline triangleShader {renderPipelineLayout, {"assets/shaders/triangle.vert", "assets/shaders/triangle.frag"}};
	v4d::graphics::vulkan::RenderPass renderPass;

	// Command buffers
	std::array<VkCommandBuffer, NB_FRAMES_IN_FLIGHT> commandBuffer;


	/////////////////////////////////////////////////////////
	
	using v4d::graphics::Renderer::Renderer;

	// Device

	virtual void ScorePhysicalDeviceSelection(int& score, v4d::graphics::PhysicalDevice*) {
		
	}
	virtual void InitDeviceFeatures(v4d::graphics::PhysicalDevice::DeviceFeatures* deviceFeaturesToEnable, const v4d::graphics::PhysicalDevice::DeviceFeatures* supportedDeviceFeatures) override {
		
	}
	
	// Renderer
	virtual void ConfigureRenderer() override {
		
	}
	virtual void InitLayouts() override {
		
	}

	// Shaders
	virtual void ConfigureShaders() override {
		// triangleShader.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		// triangleShader.depthStencilState.depthTestEnable = VK_FALSE;
		// triangleShader.depthStencilState.depthWriteEnable = VK_FALSE;
		// triangleShader.rasterizer.cullMode = VK_CULL_MODE_NONE;
		triangleShader.SetData(3);
	}
	virtual void ReadShaders() override {
		triangleShader.ReadShaders();
	}

	// Command buffers
	virtual void CreateCommandBuffers() override {
		VkCommandBufferAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandBufferCount = commandBuffer.size();
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = renderingDevice->GetQueue("graphics").commandPool;
		if (renderingDevice->AllocateCommandBuffers(&allocInfo, commandBuffer.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffer");
		}
	}
	virtual void DestroyCommandBuffers() override {
		renderingDevice->FreeCommandBuffers(renderingDevice->GetQueue("graphics").commandPool, commandBuffer.size(), commandBuffer.data());
	}

	// Synchronization
	virtual void CreateSyncObjects() override {
		// Semaphores
		VkSemaphoreCreateInfo semaphoreInfo {};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		for (auto&s : renderSemaphore) renderingDevice->CreateSemaphore(&semaphoreInfo, nullptr, &s);
		for (auto&s : presentSemaphore) renderingDevice->CreateSemaphore(&semaphoreInfo, nullptr, &s);
		
		// Fences
		VkFenceCreateInfo fenceInfo = {};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Initialize in the signaled state so that we dont get stuck on the first frame
		for (auto&f : frameFence) {
			if (renderingDevice->CreateFence(&fenceInfo, nullptr, &f) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create fence");
			}
		}
	}
	virtual void DestroySyncObjects() override {
		for (auto&s : renderSemaphore) renderingDevice->DestroySemaphore(s, nullptr);
		for (auto&s : presentSemaphore) renderingDevice->DestroySemaphore(s, nullptr);
		for (auto&f : frameFence) renderingDevice->DestroyFence(f, nullptr);
	}

	// Resources
	virtual void CreateResources() override {
		for (auto&i : renderTarget) {
			i = v4d::graphics::Image{ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT ,1,1, { VK_FORMAT_R16G16B16A16_SFLOAT }};
			i.Create(renderingDevice, swapChain->extent.width, swapChain->extent.height);
			RunSingleTimeCommands(renderingDevice->GetQueue("graphics"), [&](VkCommandBuffer commandBuffer){
				TransitionImageLayout(commandBuffer, i, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
			});
		}
	}
	virtual void DestroyResources() override {
		for (auto&i : renderTarget) i.Destroy(renderingDevice);
	}

	// Buffers
	virtual void AllocateBuffers() override {
		
	}
	virtual void FreeBuffers() override {
		
	}

	// Pipelines
	virtual void CreatePipelines() override {
		renderPipelineLayout.Create(renderingDevice);
		
		// Render to SwapChain image
		VkAttachmentDescription attachment {};
			attachment.format = swapChain->format.format;
			attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		
		// SubPasses
		VkAttachmentReference colorAttachmentRef {renderPass.AddAttachment(attachment), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		{
			VkSubpassDescription subpass {};
				subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subpass.colorAttachmentCount = 1;
				subpass.pColorAttachments = &colorAttachmentRef;
			renderPass.AddSubpass(subpass);
		}
		
		VkSubpassDependency subPassDependency {
			VK_SUBPASS_EXTERNAL,// srcSubpass;
			0,// dstSubpass;
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,// srcStageMask;
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,// dstStageMask;
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,// srcAccessMask;
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT|VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,// dstAccessMask;
			0// dependencyFlags;
		};
		renderPass.renderPassInfo.dependencyCount = 1;
		renderPass.renderPassInfo.pDependencies = &subPassDependency;
		
		// Create the render pass
		renderPass.Create(renderingDevice);
		renderPass.CreateFrameBuffers(renderingDevice, swapChain);
		
		// Shaders
		triangleShader.SetRenderPass(swapChain, renderPass.handle);
		triangleShader.AddColorBlendAttachmentState();
		triangleShader.CreatePipeline(renderingDevice);
	}
	virtual void DestroyPipelines() override {
		triangleShader.DestroyPipeline(renderingDevice);
		renderPass.DestroyFrameBuffers(renderingDevice);
		renderPass.Destroy(renderingDevice);
		
		renderPipelineLayout.Destroy(renderingDevice);
	}

	virtual void Render() override {
		// Aquire swapchain image
		uint32_t imageIndex;
		{
			VkResult result = renderingDevice->AcquireNextImageKHR(
				swapChain->GetHandle(), // swapChain
				1000UL * 1000 * 1000, // timeout in nanoseconds (using max disables the timeout)
				renderSemaphore[currentFrameInFlight], // semaphore
				VK_NULL_HANDLE, // fence
				&imageIndex // output the index of the swapchain image in there
			);
			if (result == VK_ERROR_OUT_OF_DATE_KHR) {
				RecreateSwapChain();
				return;
			} else if (result != VK_SUCCESS) {
				throw std::runtime_error("Failed to acquire swap chain images");
			}
		}
		
		// Wait for frame fence
		if (renderingDevice->WaitForFences(1, &frameFence[currentFrameInFlight], VK_TRUE, /*timeout*/1000UL * 1000 * 1000) != VK_SUCCESS) {
			throw std::runtime_error("Failed to Wait for beforeUpdate fence");
		}
		
		{// Command buffer
			const std::vector<VkSemaphore> waitSemaphores {
				renderSemaphore[currentFrameInFlight],
			};
			const std::vector<VkPipelineStageFlags> waitStages {
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
			};
			const std::vector<VkSemaphore> signalSemaphores {
				presentSemaphore[currentFrameInFlight],
			};
			const VkFence& triggerFence = frameFence[currentFrameInFlight];
			
			VkCommandBufferBeginInfo beginInfo = {};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			VkSubmitInfo submitInfo {};
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submitInfo.waitSemaphoreCount = waitSemaphores.size();
				submitInfo.pWaitSemaphores = waitSemaphores.data();
				submitInfo.pWaitDstStageMask = waitStages.data();
				submitInfo.commandBufferCount = 1;
				submitInfo.pCommandBuffers = &commandBuffer[currentFrameInFlight];
				submitInfo.signalSemaphoreCount = signalSemaphores.size();
				submitInfo.pSignalSemaphores = signalSemaphores.data();
			
			if (triggerFence) renderingDevice->ResetFences(1, &triggerFence);
			renderingDevice->ResetCommandBuffer(commandBuffer[currentFrameInFlight], 0);
			if (renderingDevice->BeginCommandBuffer(commandBuffer[currentFrameInFlight], &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("Faild to begin recording command buffer");
			}
			
			{// Draw
				renderPass.Begin(renderingDevice, commandBuffer[currentFrameInFlight], swapChain, {{.0,.0,.0,.0}}, imageIndex);
				
					// Draw triangle
					triangleShader.Execute(renderingDevice, commandBuffer[currentFrameInFlight]);
					
				renderPass.End(renderingDevice, commandBuffer[currentFrameInFlight]);
			}
			
			if (renderingDevice->EndCommandBuffer(commandBuffer[currentFrameInFlight]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to record command buffer");
			}
			
			VkResult result = renderingDevice->QueueSubmit(renderingDevice->GetQueue("graphics").handle, 1, &submitInfo, triggerFence);
			if (result != VK_SUCCESS) {
				if (result == VK_ERROR_DEVICE_LOST) {
					throw std::runtime_error("Failed to submit command buffer : VK_ERROR_DEVICE_LOST");
				}
				throw std::runtime_error("Failed to submit command buffer");
			}
		}
		
		{// Present on screen
			VkPresentInfoKHR presentInfo = {};
				presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
				// Specify which semaphore to wait on before presentation can happen
				presentInfo.waitSemaphoreCount = 1;
				presentInfo.pWaitSemaphores = &presentSemaphore[currentFrameInFlight];
				// Specify the swap chains to present images to and the index for each swap chain. (almost always a single one)
				VkSwapchainKHR swapChains[] = {swapChain->GetHandle()};
				presentInfo.swapchainCount = 1;
				presentInfo.pSwapchains = swapChains;
				presentInfo.pImageIndices = &imageIndex;
				// The next param allows to specify an array of VkResult values to check for every individual swap chain if presentation was successful.
				// its not necessary if only using a single swap chain, because we can simply use the return value of the present function.
				presentInfo.pResults = nullptr;
				// Send the present info to the presentation queue !
				// This submits the request to present an image to the swap chain.
				
			VkResult result = renderingDevice->QueuePresentKHR(renderingDevice->GetQueue("present").handle, &presentInfo);

			// Check for errors
			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR/* || window.WasFrameBufferResized()*/) {
				RecreateSwapChain();
				return;
			} else if (result != VK_SUCCESS) {
				throw std::runtime_error("Failed to present to swap chain image");
			}
		}
		
		currentFrameInFlight %= NB_FRAMES_IN_FLIGHT;
	}

};


int main(const int argc, const char** argv) {
	
	// Load V4D Core
	if (!v4d::Init()) throw std::runtime_error("Failed to init V4D Core");
	
	// Load vulkan driver
	v4d::graphics::vulkan::Loader vulkanLoader;
	if(!vulkanLoader()) throw std::runtime_error("Failed to load Vulkan library");
	
	// Create a window
	v4d::graphics::Window window("TEST", 1280, 720);
	
	// Create vulkan instance
	window.GetRequiredVulkanInstanceExtensions(vulkanLoader.requiredInstanceExtensions);
	MyRenderer renderer(&vulkanLoader, "V4D minimal project", VK_MAKE_VERSION(1, 0, 0));
	
	// Create presentation surface using window and vulkan instance
	renderer.surface = window.CreateVulkanSurface(renderer.handle);
	
	// Input callbacks
	window.AddKeyCallback("default", [&](int key, int scancode, int action, int mods){
		switch (key) {
			
			// Quit
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window.GetHandle(), 1);
				break;
				
		}
	});
	
	// Load renderer
	renderer.preferredPresentModes = {
		VK_PRESENT_MODE_MAILBOX_KHR,	// TripleBuffering (No Tearing, low latency)
		VK_PRESENT_MODE_FIFO_KHR,	// VSync ON (No Tearing, more latency)
		// VK_PRESENT_MODE_IMMEDIATE_KHR,	// VSync OFF (With Tearing, no latency)
	};
	renderer.InitRenderer();
	renderer.ReadShaders();
	renderer.LoadRenderer();
	
	// Game Loop
	SET_THREAD_HIGHEST_PRIORITY()
	while(window.IsActive()) {
		
		static double avgFramerate = 0;
		CALCULATE_AVG_FRAMERATE(avgFramerate)
		
		glfwPollEvents();
		
		renderer.Render();
		
		std::stringstream fps {"TEST "};
		fps << avgFramerate << " FPS";
		window.SetTitle(fps.str().c_str());
	}
	
	// Wait for queues to finish
	renderer.renderingDevice->DeviceWaitIdle();
	
	// Destroy stuff
	renderer.UnloadRenderer();
	renderer.DestroySurfaceKHR(renderer.surface, nullptr);
	
}
