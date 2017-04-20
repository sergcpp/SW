#ifndef RENDER_STATE_VK
#define RENDER_STATE_VK

#include <vector>

#include <vulkan/vulkan.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <xcb/xcb.h>
#endif

namespace R {
	struct layer_properties {
		VkLayerProperties properties;
		std::vector<VkExtensionProperties> extensions;
	};

	struct DeviceInfo {
		VkPhysicalDevice					gpu;
		VkPhysicalDeviceProperties			gpu_properties;
		VkPhysicalDeviceMemoryProperties	memory_properties;
		VkDevice							device;
	};

	struct SwapChainBuffer {
		VkImage		image;
		VkImageView view;
	};

	struct Depth {
		VkFormat		format;

		VkImage			image;
		VkDeviceMemory	mem;
		VkImageView		view;
	};

	struct AppInfoVK {
#ifdef _WIN32
		HINSTANCE connection;
		HWND window;
#else // _WIN32
		xcb_connection_t *connection;
		xcb_screen_t *screen;
		xcb_window_t window;
		xcb_intern_atom_reply_t *atom_wm_delete_window;
#endif // _WIN32

		std::vector<const char *>		device_layer_names;
		std::vector<const char *>		device_extension_names;
		std::vector<const char *>		instance_extension_names;
		std::vector<layer_properties>	instance_layer_properties;
		VkInstance inst;

		VkInstance		instance;
		VkSurfaceKHR	surface;
		int				w, h;
		VkFormat		format;

		uint32_t queue_count;
		DeviceInfo device_info;

		std::vector<VkQueueFamilyProperties> queue_props;
		
		uint32_t graphics_queue_family_index;

		VkCommandPool	cmd_pool;
		VkCommandBuffer cmd_buf;

		VkQueue			queue;

		std::vector<SwapChainBuffer>	buffers;
		uint32_t						current_buffer;

		VkSwapchainKHR					swapchain;
		uint32_t						swapchain_image_count;

		Depth							depth;

		VkRenderPass					render_pass;

		VkFramebuffer					*framebuffers;

		VkPipelineCache					pipeline_cache;

		// uniform buffer
		VkBuffer				u_buf;
		VkDeviceMemory			u_mem;
		VkDescriptorBufferInfo	u_buffer_info;
	};
	extern AppInfoVK app_info;
}

#endif // RENDER_STATE_VK