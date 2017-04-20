#include "RenderState.h"

#include <cassert>
#include <cstring>

#include <limits>
#include <string>

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

Camera *R::current_cam = nullptr;

namespace R {
	// from Texture.cpp
	extern float anisotropy;
	// from Mesh.cpp
	extern int max_uniform_vec4;
	extern int max_gpu_bones;

    int w = 0, h = 0;

    void ReleaseAllTextures();
    void ReleaseAllMaterials();
    void ReleaseAllPrograms();
    void ReleaseAllMeshes();
}

#ifdef USE_GL_RENDER
namespace R {
	// from ProgramGL.cpp
	extern std::string glsl_defines;
}


uint32_t R::current_program = std::numeric_limits<uint32_t>::max();
uint32_t R::invalid_texture = std::numeric_limits<uint32_t>::max();
uint32_t R::binded_textures[NUM_TEXTURE_SLOTS] = { R::invalid_texture };

void R::Init(int w, int h) {
    R::w = w;
    R::h = h;

	printf("===========================================\n");
	printf("Device info:\n");

	// print device info
#if !defined(EMSCRIPTEN) && !defined(__ANDROID__)
	GLint gl_version;
	glGetIntegerv(GL_MAJOR_VERSION, &gl_version);
	printf("\tOpenGL version\t: %i\n", int(gl_version));
#endif

	printf("\tVendor\t\t: %s\n",		glGetString(GL_VENDOR));
	printf("\tRenderer\t: %s\n",		glGetString(GL_RENDERER));
	printf("\tGLSL version\t: %s\n",	glGetString(GL_SHADING_LANGUAGE_VERSION));

	printf("Capabilities:\n");

	// determine if anisotropy supported
	if (R::IsExtensionSupported("GL_EXT_texture_filter_anisotropic")) {
		GLfloat f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &f);
		R::anisotropy = f;
		printf("\tAnisotropy\t: %f\n", R::anisotropy);
	}

	// how many uniform vec4 vectors can be used
	GLint i;
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &i);
	R::max_uniform_vec4 = i;
	printf("\tMax uniforms\t: %i\n", R::max_uniform_vec4);

	// how many bones(mat4) can be used at time
	R::max_gpu_bones = R::max_uniform_vec4 / 8;
	printf("\tBones per pass\t: %i\n", R::max_gpu_bones);
	char buff[16];
	sprintf(buff, "%i", R::max_gpu_bones);
	R::glsl_defines += "#define MAX_GPU_BONES ";
	R::glsl_defines += buff;
	R::glsl_defines += "\r\n";

	printf("===========================================\n\n");
}

void R::Resize(int w, int h) {
    R::w = w;
    R::h = h;

	glViewport(0, 0, w, h);
}

int R::IsExtensionSupported(const char *ext) {
	const GLubyte *extensions = NULL;
	const GLubyte *start;
	GLubyte *where, *terminator;
	
	where = (GLubyte *)strchr(ext, ' ');
	if (where || *ext == '\0')
		return 0;
	extensions = glGetString(GL_EXTENSIONS);
    if (!extensions) return 0;
	assert(ext);
	
	start = extensions;
	for (;;) {
		where = (GLubyte *)strstr((const char *)start, ext);
		if (!where)
			break;
		terminator = where + strlen(ext);
		if (where == start || *(where - 1) == ' ') if (*terminator == ' ' || *terminator == '\0')
			return 1;
		start = terminator;
	}
	return 0;
}

void R::Deinit() {
    R::ReleaseAllTextures();
    R::ReleaseAllMaterials();
    R::ReleaseAllPrograms();
    R::ReleaseAllMeshes();
}

#endif

#ifdef USE_SW_RENDER
uint32_t R::current_program = std::numeric_limits<uint32_t>::max();
uint32_t R::binded_textures[16] = { std::numeric_limits<uint32_t>::max() };

/*===========================================
Device info:
Vendor		: GenuineIntel
Renderer	: Software
Capabilities:
        CPU Model		: Intel(R) Core(TM) i3 CPU         540  @ 3.07GHz
Num CPUs		: 4
Physical memory	: 5.820683 GB
        Max uniforms	: 32
Bones per pass	: 16
===========================================*/

void R::Init(int w, int h) {
    R::w = w;
    R::h = h;

    printf("===========================================\n");
    printf("Device info:\n");

    // TODO: get cpu name and memory
    // print device info
    printf("\tVendor\t\t: %s\n",		swGetString(SW_CPU_VENDOR));
    printf("\tRenderer\t: %s\n",		"Software");

    printf("Capabilities:\n");

    printf("\tCPU Model\t\t: %s\n",         swGetString(SW_CPU_MODEL));
    printf("\tNum CPUs\t\t: %i\n",          (int)swGetInteger(SW_NUM_CPUS));
	printf("\tPhysical memory\t: %f GB\n",  (float)swGetFloat(SW_PHYSICAL_MEMORY));

    // how many uniform vec4 vectors can be used
    R::max_uniform_vec4 = swGetInteger(SW_MAX_VERTEX_UNIFORM_VECTORS);
    printf("\tMax uniforms\t: %i\n", R::max_uniform_vec4);

    // how many bones(mat4) can be used at time
    R::max_gpu_bones = R::max_uniform_vec4 / 2;
    printf("\tBones per pass\t: %i\n", R::max_gpu_bones);

    printf("===========================================\n\n");
}

void R::Resize(int w, int h) {
    R::w = w;
    R::h = h;

    SWint cur = swGetCurFramebuffer();
    assert(cur == 0);
    swDeleteFramebuffer(cur);
    swCreateFramebuffer(SW_BGRA8888, w, h, true);
}

void R::Deinit() {
    R::ReleaseAllTextures();
    R::ReleaseAllMaterials();
    R::ReleaseAllPrograms();
	R::ReleaseAllMeshes();
}

#endif

#ifdef USE_VK_RENDER

#include <cassert>

namespace R {
	VkResult	InitGlobalLayerProperties(std::vector<layer_properties> &instance_layer_properties);
	void		InitInstanceExtensionNames(std::vector<const char *> &instance_extension_names);
	void		InitDeviceExtensionNames(std::vector<const char *> &device_extension_names);
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

	void SetImageLayout(VkCommandBuffer cmd_buf, VkQueue queue, VkImage image, VkImageAspectFlags aspect_mask, VkImageLayout old_image_layout, VkImageLayout new_image_layout);
	bool MemoryTypeFromProperties(VkPhysicalDeviceMemoryProperties &memory_properties, uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);
}

void R::Init(int w, int h) {
	auto &info = R::app_info;
	VkResult res;

	res = InitGlobalLayerProperties(info.instance_layer_properties);
	assert(res == VK_SUCCESS);

	InitInstanceExtensionNames(info.instance_extension_names);
	InitDeviceExtensionNames(info.device_extension_names);
	
	res = InitInstance("App", info.instance);
	assert(res == VK_SUCCESS);
	res = InitEnumerateDevices(info.instance, info.queue_count, info.queue_props, info.device_info);
	assert(res == VK_SUCCESS);

	printf("Driver Version: %d\n", info.device_info.gpu_properties.driverVersion);
	printf("Device Name:    %s\n", info.device_info.gpu_properties.deviceName);
	printf("Device Type:    %d\n", info.device_info.gpu_properties.deviceType);
	printf("API Version:    %d.%d.%d\n",
		// See note below regarding this:
		(info.device_info.gpu_properties.apiVersion >> 22) & 0x3FF,
		(info.device_info.gpu_properties.apiVersion >> 12) & 0x3FF,
		(info.device_info.gpu_properties.apiVersion & 0x3FF));

	info.w = w;
	info.h = h;

#ifndef _WIN32
	//#error "implement init connection or something"
#endif

	InitWindow(info);

	res = InitSwapchainExtension(info);
	assert(res == VK_SUCCESS);

	res = InitDevice(info);
	assert(res == VK_SUCCESS);

	res = InitCommandPool(info.device_info.device, info.graphics_queue_family_index, info.cmd_pool);
	assert(res == VK_SUCCESS);

	res = InitCommandBuffer(info.device_info.device, info.cmd_pool, info.cmd_buf);
	assert(res == VK_SUCCESS);

	res = ExecBeginCommandBuffer(info.cmd_buf);
	assert(res == VK_SUCCESS);

	vkGetDeviceQueue(info.device_info.device, info.graphics_queue_family_index, 0, &info.queue);

	res = InitSwapChain(info.device_info.device, info.device_info.gpu, info.surface, info.cmd_buf, info.queue, 
						info.format, info.w, info.h, info.swapchain_image_count, info.swapchain, info.buffers);
	assert(res == VK_SUCCESS);
	info.current_buffer = 0;

	res = InitDepthBuffer(info.device_info.gpu, info.device_info.device, info.queue, info.cmd_buf, info.w, info.h,
		info.depth, info.device_info.memory_properties);
	assert(res == VK_SUCCESS);

	res = InitUniformBuffer(info.device_info.device, info.device_info.memory_properties, info.u_buf, info.u_mem, info.u_buffer_info);
	assert(res == VK_SUCCESS);

	res = InitRenderPass(info.device_info.device, info.format, info.depth.format, true, info.render_pass);
	assert(res == VK_SUCCESS);

	res = InitFramebuffers(info.device_info.device, info.render_pass, info.depth.view, info.w, info.h, info.swapchain_image_count, info.buffers, info.framebuffers);
	assert(res == VK_SUCCESS);

	res = InitPipelineCache(info.device_info.device, info.pipeline_cache);
	assert(res == VK_SUCCESS);

#if 0
	res = [&info](){
		VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
		VkPipelineDynamicStateCreateInfo dynamicState = {};
		memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pNext = NULL;
		dynamicState.pDynamicStates = dynamicStateEnables;
		dynamicState.dynamicStateCount = 0;

		VkPipelineVertexInputStateCreateInfo vi;
		vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vi.pNext = NULL;
		vi.flags = 0;
		vi.vertexBindingDescriptionCount = 1;
		vi.pVertexBindingDescriptions = &info.vi_binding;
		vi.vertexAttributeDescriptionCount = 2;
		vi.pVertexAttributeDescriptions = info.vi_attribs;

		VkPipelineInputAssemblyStateCreateInfo ia;
		ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		ia.pNext = NULL;
		ia.flags = 0;
		ia.primitiveRestartEnable = VK_FALSE;
		ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipelineRasterizationStateCreateInfo rs;
		rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rs.pNext = NULL;
		rs.flags = 0;
		rs.polygonMode = VK_POLYGON_MODE_FILL;
		rs.cullMode = VK_CULL_MODE_BACK_BIT;
		rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rs.depthClampEnable = include_depth;

		rs.rasterizerDiscardEnable = VK_FALSE;
		rs.depthBiasEnable = VK_FALSE;
		rs.depthBiasConstantFactor = 0;
		rs.depthBiasClamp = 0;
		rs.depthBiasSlopeFactor = 0;
		rs.lineWidth = 0;

		VkPipelineColorBlendStateCreateInfo cb;
		cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		cb.flags = 0;
		cb.pNext = NULL;
		VkPipelineColorBlendAttachmentState att_state[1];
		att_state[0].colorWriteMask = 0xf;
		att_state[0].blendEnable = VK_FALSE;
		att_state[0].alphaBlendOp = VK_BLEND_OP_ADD;
		att_state[0].colorBlendOp = VK_BLEND_OP_ADD;
		att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		cb.attachmentCount = 1;
		cb.pAttachments = att_state;
		cb.logicOpEnable = VK_FALSE;
		cb.logicOp = VK_LOGIC_OP_NO_OP;
		cb.blendConstants[0] = 1.0f;
		cb.blendConstants[1] = 1.0f;
		cb.blendConstants[2] = 1.0f;
		cb.blendConstants[3] = 1.0f;

		VkPipelineViewportStateCreateInfo vp = {};
		vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		vp.pNext = NULL;
		vp.flags = 0;
		vp.viewportCount = NUM_VIEWPORTS;
		dynamicStateEnables[dynamicState.dynamicStateCount++] =
			VK_DYNAMIC_STATE_VIEWPORT;
		vp.scissorCount = NUM_SCISSORS;
		dynamicStateEnables[dynamicState.dynamicStateCount++] =
			VK_DYNAMIC_STATE_SCISSOR;
		vp.pScissors = NULL;
		vp.pViewports = NULL;

		VkPipelineDepthStencilStateCreateInfo ds;
		ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		ds.pNext = NULL;
		ds.flags = 0;
		ds.depthTestEnable = include_depth;
		ds.depthWriteEnable = include_depth;
		ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		ds.depthBoundsTestEnable = VK_FALSE;
		ds.stencilTestEnable = VK_FALSE;
		ds.back.failOp = VK_STENCIL_OP_KEEP;
		ds.back.passOp = VK_STENCIL_OP_KEEP;
		ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
		ds.back.compareMask = 0;
		ds.back.reference = 0;
		ds.back.depthFailOp = VK_STENCIL_OP_KEEP;
		ds.back.writeMask = 0;
		ds.minDepthBounds = 0;
		ds.maxDepthBounds = 0;
		ds.stencilTestEnable = VK_FALSE;
		ds.front = ds.back;

		VkPipelineMultisampleStateCreateInfo ms;
		ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		ms.pNext = NULL;
		ms.flags = 0;
		ms.pSampleMask = NULL;
		ms.rasterizationSamples = NUM_SAMPLES;
		ms.sampleShadingEnable = VK_FALSE;
		ms.alphaToCoverageEnable = VK_FALSE;
		ms.alphaToOneEnable = VK_FALSE;
		ms.minSampleShading = 0.0;

		VkGraphicsPipelineCreateInfo pipeline;
		pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline.pNext = NULL;
		pipeline.layout = info.pipeline_layout;
		pipeline.basePipelineHandle = VK_NULL_HANDLE;
		pipeline.basePipelineIndex = 0;
		pipeline.flags = 0;
		pipeline.pVertexInputState = include_vi ? &vi : NULL;
		pipeline.pInputAssemblyState = &ia;
		pipeline.pRasterizationState = &rs;
		pipeline.pColorBlendState = &cb;
		pipeline.pTessellationState = NULL;
		pipeline.pMultisampleState = &ms;
		pipeline.pDynamicState = &dynamicState;
		pipeline.pViewportState = &vp;
		pipeline.pDepthStencilState = &ds;
		pipeline.pStages = info.shaderStages;
		pipeline.stageCount = 2;
		pipeline.renderPass = info.render_pass;
		pipeline.subpass = 0;

		return vkCreateGraphicsPipelines(info.device, info.pipelineCache, 1,
			&pipeline, NULL, &info.pipeline);
	}();
	assert(res == VK_SUCCESS);
#endif
}

void R::Deinit() {
	auto &info = R::app_info;

	//vkDestroySemaphore(info.device, presentCompleteSemaphore, NULL);
	//vkDestroyFence(info.device, drawFence, NULL);

	// Destory pipeline
	//vkDestroyPipeline(info.device, info.pipeline, NULL);

	// Destroy pipeline cache
	//vkDestroyPipelineCache(info.device, info.pipelineCache, NULL);

	// Destroy descriptor pool
	//vkDestroyDescriptorPool(info.device, info.desc_pool, NULL);

	// Destroy frambuffers
	for (uint32_t i = 0; i < info.swapchain_image_count; i++) {
		vkDestroyFramebuffer(info.device_info.device, info.framebuffers[i], NULL);
	}
	free(info.framebuffers);

	// Destory renderpass
	vkDestroyRenderPass(info.device_info.device, info.render_pass, NULL);

	// Destroy uniform buffer
	vkDestroyBuffer(info.device_info.device, info.u_buf, NULL);
	vkFreeMemory(info.device_info.device, info.u_mem, NULL);

	// Destroy depth buffer
	vkDestroyImageView(info.device_info.device, info.depth.view, NULL);
	vkDestroyImage(info.device_info.device, info.depth.image, NULL);
	vkFreeMemory(info.device_info.device, info.depth.mem, NULL);

	// Destroy swap chain
	for (uint32_t i = 0; i < info.swapchain_image_count; i++) {
		vkDestroyImageView(info.device_info.device, info.buffers[i].view, NULL);
	}
	vkDestroySwapchainKHR(info.device_info.device, info.swapchain, NULL);

	// Destroy command buffer
	VkCommandBuffer cmd_bufs[1] = { info.cmd_buf };
	vkFreeCommandBuffers(info.device_info.device, info.cmd_pool, 1, cmd_bufs);

	// Destroy command pool
	vkDestroyCommandPool(info.device_info.device, info.cmd_pool, NULL);

	// Destroy window
	DestroyWindow(info);

	// Destroy device
	vkDestroyDevice(info.device_info.device, NULL);

	// Destroy instance
	vkDestroyInstance(info.instance, NULL);
}

#endif


