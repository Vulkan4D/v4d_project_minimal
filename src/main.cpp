#include <v4d.h>

using namespace v4d::graphics;

#define APPLICATION_NAME "V4D Test"
#define WINDOW_TITLE "TEST"
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define APPLICATION_VERSION VK_MAKE_VERSION(1, 0, 0)

Loader vulkanLoader;

int main() {
	// Load V4D Core
	if (!v4d::Init()) return -1;
	
	// Validation layers
	#if defined(_DEBUG) && defined(_LINUX)
		vulkanLoader.requiredInstanceLayers.push_back("VK_LAYER_KHRONOS_validation");
	#endif
	
	// Vulkan
	if (!vulkanLoader()) 
		throw std::runtime_error("Failed to load Vulkan library");
	
	// Create Window and Init Vulkan
	Window* window = new Window(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	window->GetRequiredVulkanInstanceExtensions(vulkanLoader.requiredInstanceExtensions);

	// Create Renderer (and Vulkan Instance)
	auto* renderer = new Renderer(&vulkanLoader, APPLICATION_NAME, APPLICATION_VERSION);
	renderer->surface = window->CreateVulkanSurface(renderer->handle);
	
	// Load renderer
	renderer->preferredPresentModes = {
		VK_PRESENT_MODE_MAILBOX_KHR,	// TripleBuffering (No Tearing, low latency)
		VK_PRESENT_MODE_IMMEDIATE_KHR,	// VSync OFF (With Tearing, no latency)
		VK_PRESENT_MODE_FIFO_KHR,		// VSync ON (No Tearing, more latency)
	};
	renderer->InitRenderer();
	renderer->ReadShaders();
	renderer->LoadScene();
	renderer->LoadRenderer();
	
	
	
	
	
	
	
	// Vulkan Renderer is ready, you may have render loops here
	
	
	
	
	
	
	
	
	renderer->UnloadRenderer();
	renderer->UnloadScene();
	
	// Close Window and delete Vulkan
	renderer->DestroySurfaceKHR(renderer->surface, nullptr);
	delete renderer;
	delete window;

	LOG("\n\nApplication terminated\n\n");
}
