#include "RenderStateVK.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <vector>

#define UNIFORM_BUFFER_SIZE 128

namespace R {
	AppInfoVK app_info;

	VkResult	InitGlobalExtensionProperties(layer_properties &layer_props);
	void		InitInstanceExtensionNames(std::vector<const char *> &instance_extension_names);
	void		InitDeviceExtensionNames(std::vector<const char *> &device_extension_names);

	VkResult	InitGlobalLayerProperties(std::vector<layer_properties> &instance_layer_properties);
	VkResult	InitInstance(const char *app_name, VkInstance &instance);
	VkResult	InitEnumerateDevices(VkInstance instance, uint32_t &queue_count, std::vector<VkQueueFamilyProperties> &queue_props, DeviceInfo &info);
	void		InitWindow(AppInfoVK &info);
	VkResult	InitSwapchainExtension(AppInfoVK &info);
	VkResult	InitDevice(AppInfoVK &info);

	VkResult	InitCommandPool(VkDevice device, uint32_t graphics_queue_family_index, VkCommandPool &cmd_pool);
	VkResult	InitCommandBuffer(VkDevice device, VkCommandPool cmd_pool, VkCommandBuffer &cmd_buf);

	VkResult	ExecBeginCommandBuffer(VkCommandBuffer cmd_buf);

	VkResult	InitSwapChain(VkDevice device, VkPhysicalDevice gpu, VkSurfaceKHR surface, VkCommandBuffer cmd_buf, VkQueue queue, VkFormat format, int w, int h, uint32_t &swapchain_image_count, VkSwapchainKHR &_swap_chain, std::vector<SwapChainBuffer> &buffers);
	VkResult	InitDepthBuffer(VkPhysicalDevice gpu, VkDevice device, VkQueue queue, VkCommandBuffer cmd_buf, int w, int h, Depth &depth, VkPhysicalDeviceMemoryProperties &memory_properties);

	VkResult	InitUniformBuffer(VkDevice device, VkPhysicalDeviceMemoryProperties memory_properties, VkBuffer &buf, VkDeviceMemory &mem, VkDescriptorBufferInfo &buffer_info);

	VkResult	InitRenderPass(VkDevice device, VkFormat format, VkFormat depth_format, bool clear, VkRenderPass &render_pass);
	VkResult	InitFramebuffers(VkDevice device, VkRenderPass render_pass, VkImageView depth_view, int w, int h, uint32_t swapchain_image_count, std::vector<SwapChainBuffer> &buffers, VkFramebuffer *&framebuffers);

	void		DestroyWindow(AppInfoVK &info);

	VkResult	InitPipelineCache(VkDevice device, VkPipelineCache &_pipeline_cache);

#if 0
	// MS-Windows event handling function:
	LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		struct sample_info *info = reinterpret_cast<struct sample_info *>(
			GetWindowLongPtr(hWnd, GWLP_USERDATA));

		switch (uMsg) {
		case WM_CLOSE:
			PostQuitMessage(0);
			break;
		case WM_PAINT:
			//run(info);
			return 0;
		default:
			break;
		}
		return (DefWindowProc(hWnd, uMsg, wParam, lParam));
	}
#endif

	void SetImageLayout(VkCommandBuffer cmd_buf, VkQueue queue, VkImage image, VkImageAspectFlags aspect_mask, VkImageLayout old_image_layout, VkImageLayout new_image_layout) {
		assert(cmd_buf != VK_NULL_HANDLE);
		assert(queue != VK_NULL_HANDLE);

		VkImageMemoryBarrier image_memory_barrier = {};
		image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_memory_barrier.pNext = NULL;
		image_memory_barrier.srcAccessMask = 0;
		image_memory_barrier.dstAccessMask = 0;
		image_memory_barrier.oldLayout = old_image_layout;
		image_memory_barrier.newLayout = new_image_layout;
		image_memory_barrier.image = image;
		image_memory_barrier.subresourceRange.aspectMask = aspect_mask;
		image_memory_barrier.subresourceRange.baseMipLevel = 0;
		image_memory_barrier.subresourceRange.levelCount = 1;
		image_memory_barrier.subresourceRange.layerCount = 1;

		if (old_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
			image_memory_barrier.srcAccessMask =
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}

		if (new_image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			/* Make sure anything that was copying from this image has completed */
			image_memory_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		}

		if (new_image_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			/* Make sure any Copy or CPU writes to image are flushed */
			image_memory_barrier.srcAccessMask =
				VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}

		if (new_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
			image_memory_barrier.dstAccessMask =
				VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		}

		if (new_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			image_memory_barrier.dstAccessMask =
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		}

		VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		VkPipelineStageFlags dest_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

		vkCmdPipelineBarrier(cmd_buf, src_stages, dest_stages, 0, 0, NULL, 0, NULL, 1, &image_memory_barrier);
	}

	bool MemoryTypeFromProperties(VkPhysicalDeviceMemoryProperties &memory_properties, uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex) {
		// Search memtypes to find first index with those properties
		for (uint32_t i = 0; i < 32; i++) {
			if ((typeBits & 1) == 1) {
				// Type is available, does it match user properties?
				if ((memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
					*typeIndex = i;
					return true;
				}
			}
			typeBits >>= 1;
		}
		// No memory types matched, return failure
		return false;
	}
}

VkResult R::InitGlobalLayerProperties(std::vector<layer_properties> &instance_layer_properties) {
	uint32_t instance_layer_count;
	VkLayerProperties *vk_props = NULL;
	VkResult res;

	/*
	* It's possible, though very rare, that the number of
	* instance layers could change. For example, installing something
	* could include new layers that the loader would pick up
	* between the initial query for the count and the
	* request for VkLayerProperties. The loader indicates that
	* by returning a VK_INCOMPLETE status and will update the
	* the count parameter.
	* The count parameter will be updated with the number of
	* entries loaded into the data pointer - in case the number
	* of layers went down or is smaller than the size given.
	*/
	do {
		res = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
		if (res) {
			return res;
		}

		if (instance_layer_count == 0) {
			return VK_SUCCESS;
		}

		vk_props = (VkLayerProperties *)realloc(
			vk_props, instance_layer_count * sizeof(VkLayerProperties));

		res =
			vkEnumerateInstanceLayerProperties(&instance_layer_count, vk_props);
	} while (res == VK_INCOMPLETE);

	/*
	* Now gather the extension list for each instance layer.
	*/
	for (uint32_t i = 0; i < instance_layer_count; i++) {
		layer_properties layer_props;
		layer_props.properties = vk_props[i];
		res = InitGlobalExtensionProperties(layer_props);
		if (res) {
			return res;
		}
		instance_layer_properties.push_back(layer_props);
	}
	free(vk_props);

	return res;
}

VkResult R::InitInstance(const char *app_name, VkInstance &instance) {
	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = NULL;
	app_info.pApplicationName = app_name;
	app_info.applicationVersion = 1;
	app_info.pEngineName = app_name;
	app_info.engineVersion = 1;
	app_info.apiVersion = VK_API_VERSION;

	VkInstanceCreateInfo inst_info{};
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_info.pNext = NULL;
	inst_info.flags = 0;
	inst_info.pApplicationInfo = &app_info;
	// TODO: ???
	inst_info.enabledLayerCount = 0;
	inst_info.ppEnabledExtensionNames = NULL;
	inst_info.enabledExtensionCount = 0;
	inst_info.ppEnabledExtensionNames = NULL;

	return vkCreateInstance(&inst_info, NULL, &instance);
}

VkResult R::InitEnumerateDevices(VkInstance instance, 
									uint32_t &queue_count, 
									std::vector<VkQueueFamilyProperties> &queue_props, 
									DeviceInfo &info) {
	// Query how many devices are present in the system
	uint32_t device_count = 0;
	VkResult result = vkEnumeratePhysicalDevices(instance, &device_count, NULL);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "Failed to query the number of physical devices present: %d\n", result);
		return result;
	}

	// There has to be at least one device present
	if (device_count == 0) {
		fprintf(stderr, "Couldn't detect any device present with Vulkan support: %d\n", result);
		return VK_RESULT_MAX_ENUM;
	}

	// Get the physical devices
	std::vector<VkPhysicalDevice> physical_devices(device_count);
	result = vkEnumeratePhysicalDevices(instance, &device_count, &physical_devices[0]);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "Faied to enumerate physical devices present: %d\n", result);
		return result;
	}

	info.gpu = physical_devices[0];

	vkGetPhysicalDeviceQueueFamilyProperties(info.gpu, &queue_count, NULL);
	assert(queue_count >= 1);
	queue_props.resize(queue_count);

	vkGetPhysicalDeviceQueueFamilyProperties(info.gpu, &queue_count, queue_props.data());
	assert(queue_count >= 1);

	memset(&info.memory_properties, 0, sizeof(VkPhysicalDeviceMemoryProperties));
	vkGetPhysicalDeviceMemoryProperties(info.gpu, &info.memory_properties);

	memset(&info.gpu_properties, 0, sizeof(VkPhysicalDeviceProperties));
	vkGetPhysicalDeviceProperties(info.gpu, &info.gpu_properties);

	return VK_SUCCESS;
}

VkResult R::InitGlobalExtensionProperties(layer_properties &layer_props) {
	VkExtensionProperties *instance_extensions;
	uint32_t instance_extension_count;
	VkResult res;
	char *layer_name = NULL;

	layer_name = layer_props.properties.layerName;

	do {
		res = vkEnumerateInstanceExtensionProperties(
			layer_name, &instance_extension_count, NULL);
		if (res)
			return res;

		if (instance_extension_count == 0) {
			return VK_SUCCESS;
		}

		layer_props.extensions.resize(instance_extension_count);
		instance_extensions = layer_props.extensions.data();
		res = vkEnumerateInstanceExtensionProperties(
			layer_name, &instance_extension_count, instance_extensions);
	} while (res == VK_INCOMPLETE);

	return res;
}

void R::InitInstanceExtensionNames(std::vector<const char *> &instance_extension_names) {
	instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef _WIN32
	instance_extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
	instance_extension_names.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
}

void R::InitDeviceExtensionNames(std::vector<const char *> &device_extension_names) {
	device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

void R::InitWindow(AppInfoVK &info) {
#ifdef _WIN32
	WNDCLASSEX win_class;
	assert(info.w > 0);
	assert(info.h > 0);

	info.connection = GetModuleHandle(NULL);
	//sprintf(info.name, "Sample");

	// Initialize the window class structure:
	win_class.cbSize		= sizeof(WNDCLASSEX);
	win_class.style			= CS_HREDRAW | CS_VREDRAW;
	win_class.lpfnWndProc	= WndProc;
	win_class.cbClsExtra	= 0;
	win_class.cbWndExtra	= 0;
	win_class.hInstance		= info.connection; // hInstance
	win_class.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	win_class.hCursor		= LoadCursor(NULL, IDC_ARROW);
	win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	win_class.lpszMenuName	= NULL;
	win_class.lpszClassName = "W";//info.name;
	win_class.hIconSm		= LoadIcon(NULL, IDI_WINLOGO);
	// Register window class:
	if (!RegisterClassEx(&win_class)) {
		// It didn't work, so try to give a useful error:
		printf("Unexpected error trying to start the application!\n");
		fflush(stdout);
		exit(1);
	}
	// Create window with the registered class:
	RECT wr = { 0, 0, info.w, info.h };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	info.window = CreateWindowEx(0,
		"W",            // class name
		"W",            // app name
		WS_OVERLAPPEDWINDOW | // window style
		WS_VISIBLE | WS_SYSMENU,
		100, 100,           // x/y coords
		wr.right - wr.left, // width
		wr.bottom - wr.top, // height
		NULL,               // handle to parent
		NULL,               // handle to menu
		info.connection,    // hInstance
		NULL);              // no extra parameters
	if (!info.window) {
		// It didn't work, so try to give a useful error:
		printf("Cannot create a window in which to draw!\n");
		fflush(stdout);
		exit(1);
	}
	SetWindowLongPtr(info.window, GWLP_USERDATA, (LONG_PTR)&info);
#else
//#error "implement window creation"
#endif
}

VkResult R::InitSwapchainExtension(AppInfoVK &info) {
	VkResult res;
	// Construct the surface description:
#ifdef _WIN32
	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.hinstance = info.connection;
	createInfo.hwnd = info.window;
	res = vkCreateWin32SurfaceKHR(info.instance, &createInfo, NULL, &info.surface);
#else  // _WIN32
	VkXcbSurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.connection = info.connection;
	createInfo.window = info.window;
	res = vkCreateXcbSurfaceKHR(info.inst, &createInfo, NULL, &info.surface);
#endif // _WIN32
	if (res != VK_SUCCESS) return res;

	// Iterate over each queue to learn whether it supports presenting:
	VkBool32 *supports_present = (VkBool32 *)malloc(info.queue_count * sizeof(VkBool32));
	for (uint32_t i = 0; i < info.queue_count; i++) {
		vkGetPhysicalDeviceSurfaceSupportKHR(info.device_info.gpu, i, info.surface, &supports_present[i]);
	}

	// Search for a graphics queue and a present queue in the array of queue
	// families, try to find one that supports both
	uint32_t graphics_queue_node_index = UINT32_MAX;
	for (uint32_t i = 0; i < info.queue_count; i++) {
		if ((info.queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
			if (supports_present[i] == VK_TRUE) {
				graphics_queue_node_index = i;
				break;
			}
		}
	}
	free(supports_present);

	// Generate error if could not find a queue that supports both a graphics
	// and present
	if (graphics_queue_node_index == UINT32_MAX) {
		fprintf(stderr, "Could not find a queue that supports both graphics and present");
		return VK_RESULT_MAX_ENUM;
	}

	info.graphics_queue_family_index = graphics_queue_node_index;

	// Get the list of VkFormats that are supported:
	uint32_t format_count;
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(info.device_info.gpu, info.surface, &format_count, NULL);
	if (res != VK_SUCCESS) return res;
	VkSurfaceFormatKHR *surf_formats = (VkSurfaceFormatKHR *)malloc(format_count * sizeof(VkSurfaceFormatKHR));
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(info.device_info.gpu, info.surface, &format_count, surf_formats);
	if (res != VK_SUCCESS) return res;
	// If the format list includes just one entry of VK_FORMAT_UNDEFINED,
	// the surface has no preferred format.  Otherwise, at least one
	// supported format will be returned.
	if (format_count == 1 && surf_formats[0].format == VK_FORMAT_UNDEFINED) {
		info.format = VK_FORMAT_B8G8R8A8_UNORM;
	} else {
		if (format_count < 1) return VK_RESULT_MAX_ENUM;
		info.format = surf_formats[0].format;
	}
	return VK_SUCCESS;
}

VkResult R::InitDevice(AppInfoVK &info) {
	VkDeviceQueueCreateInfo queue_info = {};

	float queue_priorities[1] = { 0.0 };

	queue_info.sType			= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info.pNext			= NULL;
	queue_info.queueCount		= 1;
	queue_info.pQueuePriorities = queue_priorities;
	queue_info.queueFamilyIndex = info.graphics_queue_family_index;

	VkDeviceCreateInfo device_info = {};
	device_info.sType					= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pNext					= NULL;
	device_info.queueCreateInfoCount	= 1;
	device_info.pQueueCreateInfos		= &queue_info;
	device_info.enabledLayerCount		= info.device_layer_names.size();
	device_info.ppEnabledLayerNames		= device_info.enabledLayerCount ? info.device_layer_names.data() : NULL;
	device_info.enabledExtensionCount	= info.device_extension_names.size();
	device_info.ppEnabledExtensionNames = device_info.enabledExtensionCount ? info.device_extension_names.data() : NULL;
	device_info.pEnabledFeatures		= NULL;

	return vkCreateDevice(info.device_info.gpu, &device_info, NULL, &info.device_info.device);
}

VkResult R::InitCommandPool(VkDevice device, uint32_t graphics_queue_family_index, VkCommandPool &cmd_pool) {
	VkCommandPoolCreateInfo cmd_pool_info = {};
	cmd_pool_info.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_info.pNext				= NULL;
	cmd_pool_info.queueFamilyIndex	= graphics_queue_family_index;
	cmd_pool_info.flags				= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	return vkCreateCommandPool(device, &cmd_pool_info, NULL, &cmd_pool);
}

VkResult R::InitCommandBuffer(VkDevice device, VkCommandPool cmd_pool, VkCommandBuffer &cmd_buf) {
	VkCommandBufferAllocateInfo cmd = {};
	cmd.sType		= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd.pNext		= NULL;
	cmd.commandPool = cmd_pool;
	cmd.level		= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd.commandBufferCount = 1;

	return vkAllocateCommandBuffers(device, &cmd, &cmd_buf);
}

VkResult R::ExecBeginCommandBuffer(VkCommandBuffer cmd_buf) {
	VkCommandBufferBeginInfo cmd_buf_info = {};
	cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_buf_info.pNext = NULL;
	cmd_buf_info.flags = 0;
	cmd_buf_info.pInheritanceInfo = NULL;

	return vkBeginCommandBuffer(cmd_buf, &cmd_buf_info);
}

VkResult R::InitSwapChain(VkDevice device, VkPhysicalDevice gpu, VkSurfaceKHR surface, VkCommandBuffer cmd_buf, VkQueue queue, VkFormat format, int w, int h, uint32_t &swapchain_image_count, VkSwapchainKHR &_swap_chain, std::vector<SwapChainBuffer> &buffers) {
	VkResult res;
	VkSurfaceCapabilitiesKHR surf_capabilities;

	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surf_capabilities);
	if (res != VK_SUCCESS) return res;

	uint32_t present_mode_count;
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &present_mode_count, NULL);
	if (res != VK_SUCCESS) return res;

	VkPresentModeKHR *present_modes = (VkPresentModeKHR *)malloc(present_mode_count * sizeof(VkPresentModeKHR));

	res = vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &present_mode_count, present_modes);
	if (res != VK_SUCCESS) return res;

	VkExtent2D swap_chain_extent;
	// width and height are either both -1, or both not -1.
	if (surf_capabilities.currentExtent.width == (uint32_t)-1) {
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		swap_chain_extent.width = w;
		swap_chain_extent.height = h;
	} else {
		// If the surface size is defined, the swap chain size must match
		swap_chain_extent = surf_capabilities.currentExtent;
	}

	// If mailbox mode is available, use it, as is the lowest-latency non-
	// tearing mode.  If not, try IMMEDIATE which will usually be available,
	// and is fastest (though it tears).  If not, fall back to FIFO which is
	// always available.
	VkPresentModeKHR swapchain_present_mode = VK_PRESENT_MODE_FIFO_KHR;
	for (size_t i = 0; i < present_mode_count; i++) {
		if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			swapchain_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		if ((swapchain_present_mode != VK_PRESENT_MODE_MAILBOX_KHR) &&
			(present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)) {
			swapchain_present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	// Determine the number of VkImage's to use in the swap chain (we desire to
	// own only 1 image at a time, besides the images being displayed and
	// queued for display):
	uint32_t desired_number_of_swap_chain_images = surf_capabilities.minImageCount + 1;
	if ((surf_capabilities.maxImageCount > 0) &&
		(desired_number_of_swap_chain_images > surf_capabilities.maxImageCount)) {
		// Application must settle for fewer images than desired:
		desired_number_of_swap_chain_images = surf_capabilities.maxImageCount;
	}

	VkSurfaceTransformFlagBitsKHR pre_transform;
	if (surf_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	} else {
		pre_transform = surf_capabilities.currentTransform;
	}

	VkSwapchainCreateInfoKHR swap_chain = {};
	swap_chain.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swap_chain.pNext = NULL;
	swap_chain.surface = surface;
	swap_chain.minImageCount = desired_number_of_swap_chain_images;
	swap_chain.imageFormat = format;
	swap_chain.imageExtent.width = swap_chain_extent.width;
	swap_chain.imageExtent.height = swap_chain_extent.height;
	swap_chain.preTransform = pre_transform;
	swap_chain.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swap_chain.imageArrayLayers = 1;
	swap_chain.presentMode = swapchain_present_mode;
	swap_chain.oldSwapchain = VK_NULL_HANDLE;
	swap_chain.clipped = true;
	swap_chain.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	swap_chain.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swap_chain.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swap_chain.queueFamilyIndexCount = 0;
	swap_chain.pQueueFamilyIndices = NULL;

	res = vkCreateSwapchainKHR(device, &swap_chain, NULL, &_swap_chain);
	if (res != VK_SUCCESS) return res;

	res = vkGetSwapchainImagesKHR(device, _swap_chain, &swapchain_image_count, NULL);
	if (res != VK_SUCCESS) return res;

	VkImage *swapchain_images = (VkImage *)malloc(swapchain_image_count * sizeof(VkImage));
	if (res != VK_SUCCESS) return res;

	res = vkGetSwapchainImagesKHR(device, _swap_chain, &swapchain_image_count, swapchain_images);
	assert(res == VK_SUCCESS);

	for (uint32_t i = 0; i < swapchain_image_count; i++) {
		SwapChainBuffer sc_buffer;

		VkImageViewCreateInfo color_image_view = {};
		color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		color_image_view.pNext = NULL;
		color_image_view.format = format;
		color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;
		color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;
		color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;
		color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;
		color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		color_image_view.subresourceRange.baseMipLevel = 0;
		color_image_view.subresourceRange.levelCount = 1;
		color_image_view.subresourceRange.baseArrayLayer = 0;
		color_image_view.subresourceRange.layerCount = 1;
		color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		color_image_view.flags = 0;

		sc_buffer.image = swapchain_images[i];

		SetImageLayout(cmd_buf, queue, sc_buffer.image, VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		color_image_view.image = sc_buffer.image;

		res = vkCreateImageView(device, &color_image_view, NULL, &sc_buffer.view);
		buffers.push_back(sc_buffer);
		if (res != VK_SUCCESS) return res;
	}

	if (NULL != present_modes) {
		free(present_modes);
	}
	return VK_SUCCESS;
}

VkResult R::InitDepthBuffer(VkPhysicalDevice gpu, VkDevice device, VkQueue queue, VkCommandBuffer cmd_buf, int w, int h, Depth &depth, VkPhysicalDeviceMemoryProperties &memory_properties) {
	VkResult res;
	bool pass;
	VkImageCreateInfo image_info = {};

	/* allow custom depth formats */
	if (depth.format == VK_FORMAT_UNDEFINED)
		depth.format = VK_FORMAT_D16_UNORM;

	const VkFormat depth_format = depth.format;

	VkFormatProperties props;
	vkGetPhysicalDeviceFormatProperties(gpu, depth_format, &props);
	if (props.linearTilingFeatures &
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		image_info.tiling = VK_IMAGE_TILING_LINEAR;
	} else if (props.optimalTilingFeatures &
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	} else {
		/* Try other depth formats? */
		fprintf(stderr, "depth_format %i Unsupported.\n", int(depth_format));
		exit(-1);
	}

	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.pNext = NULL;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.format = depth_format;
	image_info.extent.width = w;
	image_info.extent.height = h;
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.queueFamilyIndexCount = 0;
	image_info.pQueueFamilyIndices = NULL;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	image_info.flags = 0;

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.pNext = NULL;
	mem_alloc.allocationSize = 0;
	mem_alloc.memoryTypeIndex = 0;

	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.pNext = NULL;
	view_info.image = VK_NULL_HANDLE;
	view_info.format = depth_format;
	view_info.components.r = VK_COMPONENT_SWIZZLE_R;
	view_info.components.g = VK_COMPONENT_SWIZZLE_G;
	view_info.components.b = VK_COMPONENT_SWIZZLE_B;
	view_info.components.a = VK_COMPONENT_SWIZZLE_A;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.flags = 0;

	if (depth_format == VK_FORMAT_D16_UNORM_S8_UINT ||
		depth_format == VK_FORMAT_D24_UNORM_S8_UINT ||
		depth_format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
		view_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	VkMemoryRequirements mem_reqs;

	/* Create image */
	res = vkCreateImage(device, &image_info, NULL, &depth.image);
	if (res != VK_SUCCESS) return res;

	vkGetImageMemoryRequirements(device, depth.image, &mem_reqs);

	mem_alloc.allocationSize = mem_reqs.size;
	/* Use the memory properties to determine the type of memory required */
	pass = MemoryTypeFromProperties(memory_properties, mem_reqs.memoryTypeBits,
		0, /* No requirements */
		&mem_alloc.memoryTypeIndex);
	if (!pass) return (VkResult)-1;

	/* Allocate memory */
	res = vkAllocateMemory(device, &mem_alloc, NULL, &depth.mem);
	if (res != VK_SUCCESS) return res;

	/* Bind memory */
	res = vkBindImageMemory(device, depth.image, depth.mem, 0);
	if (res != VK_SUCCESS) return res;

	/* Set the image layout to depth stencil optimal */
	SetImageLayout(cmd_buf, queue, depth.image,
		view_info.subresourceRange.aspectMask,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	/* Create image view */
	view_info.image = depth.image;
	res = vkCreateImageView(device, &view_info, NULL, &depth.view);
	if (res != VK_SUCCESS) return res;

	return VK_SUCCESS;
}

VkResult R::InitUniformBuffer(VkDevice device, VkPhysicalDeviceMemoryProperties memory_properties, VkBuffer &buf, VkDeviceMemory &mem, VkDescriptorBufferInfo &buffer_info) {
	VkResult res;
	bool pass;

	/* VULKAN_KEY_START */
	VkBufferCreateInfo buf_info = {};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	buf_info.size = UNIFORM_BUFFER_SIZE;
	buf_info.queueFamilyIndexCount = 0;
	buf_info.pQueueFamilyIndices = NULL;
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buf_info.flags = 0;
	res = vkCreateBuffer(device, &buf_info, NULL, &buf);
	if (res != VK_SUCCESS) return res;

	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(device, buf, &mem_reqs);

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.pNext = NULL;
	alloc_info.memoryTypeIndex = 0;

	alloc_info.allocationSize = mem_reqs.size;
	pass = MemoryTypeFromProperties(memory_properties, mem_reqs.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		&alloc_info.memoryTypeIndex);
	if (!pass) return (VkResult)-1;

	res = vkAllocateMemory(device, &alloc_info, NULL, &mem);
	if (res != VK_SUCCESS) return res;

	res = vkBindBufferMemory(device, buf, mem, 0);
	if (res != VK_SUCCESS) return res;

	buffer_info.buffer = buf;
	buffer_info.offset = 0;
	buffer_info.range = UNIFORM_BUFFER_SIZE;

	return VK_SUCCESS;
}

VkResult R::InitRenderPass(VkDevice device, VkFormat format, VkFormat depth_format, bool clear, VkRenderPass &render_pass) {
	/* Need attachments for render target and depth buffer */
	VkAttachmentDescription attachments[2];
	attachments[0].format = format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].flags = 0;

	attachments[1].format = depth_format;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[1].flags = 0;

	VkAttachmentReference color_reference = {};
	color_reference.attachment = 0;
	color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_reference = {};
	depth_reference.attachment = 1;
	depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.flags = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_reference;
	subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = &depth_reference;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;

	VkRenderPassCreateInfo rp_info = {};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_info.pNext = NULL;
	rp_info.attachmentCount = 2;
	rp_info.pAttachments = attachments;
	rp_info.subpassCount = 1;
	rp_info.pSubpasses = &subpass;
	rp_info.dependencyCount = 0;
	rp_info.pDependencies = NULL;

	return vkCreateRenderPass(device, &rp_info, NULL, &render_pass);
}

VkResult R::InitFramebuffers(VkDevice device, VkRenderPass render_pass, VkImageView depth_view, int w, int h, uint32_t swapchain_image_count, std::vector<SwapChainBuffer> &buffers, VkFramebuffer *&framebuffers) {
	VkResult res;
	VkImageView attachments[2];
	attachments[1] = depth_view;

	VkFramebufferCreateInfo fb_info = {};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.pNext = NULL;
	fb_info.renderPass = render_pass;
	fb_info.attachmentCount = 2;
	fb_info.pAttachments = attachments;
	fb_info.width = w;
	fb_info.height = h;
	fb_info.layers = 1;

	uint32_t i;

	framebuffers = (VkFramebuffer *)malloc(swapchain_image_count * sizeof(VkFramebuffer));

	for (i = 0; i < swapchain_image_count; i++) {
		attachments[0] = buffers[i].view;
		res = vkCreateFramebuffer(device, &fb_info, NULL, &framebuffers[i]);
		if (res != VK_SUCCESS) return res;
	}
	return VK_SUCCESS;
}

void R::DestroyWindow(AppInfoVK &info) {
#ifdef _WIN32
	vkDestroySurfaceKHR(info.instance, info.surface, NULL);
	DestroyWindow(info.window);
#else
	vkDestroySurfaceKHR(info.instance, info.surface, NULL);
	xcb_destroy_window(info.connection, info.window);
	xcb_disconnect(info.connection);
#endif
}

VkResult R::InitPipelineCache(VkDevice device, VkPipelineCache &_pipeline_cache) {
	VkPipelineCacheCreateInfo pipeline_cache;
	pipeline_cache.sType			= VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipeline_cache.pNext			= NULL;
	pipeline_cache.initialDataSize	= 0;
	pipeline_cache.pInitialData		= NULL;
	pipeline_cache.flags			= 0;
	return vkCreatePipelineCache(device, &pipeline_cache, NULL, &_pipeline_cache);
}