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
	
	
	
	
	
	
	
	
	
		#define TEST_BUFFER_ALLOC(type, size) {\
			auto timer = v4d::Timer(true);\
			Buffer testBuffer {VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, size_t(size)*1024*1024};\
			testBuffer.Allocate(renderer->renderingDevice, VK_MEMORY_PROPERTY_ ## type ## _BIT);\
			double allocTime = timer.GetElapsedMilliseconds();\
			timer.Reset();\
			testBuffer.Free(renderer->renderingDevice);\
			LOG("" << # type << " " << size << " mb Allocated in " << allocTime << " ms, Freed in " << timer.GetElapsedMilliseconds() << " ms")\
		}
		
		TEST_BUFFER_ALLOC(HOST_COHERENT, 512)
		TEST_BUFFER_ALLOC(HOST_COHERENT, 1024)
		TEST_BUFFER_ALLOC(HOST_COHERENT, 2048)
		TEST_BUFFER_ALLOC(HOST_CACHED, 512)
		TEST_BUFFER_ALLOC(HOST_CACHED, 1024)
		TEST_BUFFER_ALLOC(HOST_CACHED, 2048)
		TEST_BUFFER_ALLOC(DEVICE_LOCAL, 512)
		TEST_BUFFER_ALLOC(DEVICE_LOCAL, 1024)
		TEST_BUFFER_ALLOC(DEVICE_LOCAL, 2048)
		
	
	
	
	
	
	
	renderer->UnloadRenderer();
	renderer->UnloadScene();
	
	// Close Window and delete Vulkan
	renderer->DestroySurfaceKHR(renderer->surface, nullptr);
	delete renderer;
	delete window;

	LOG("\n\nApplication terminated\n\n");
}
