#pragma once
#pragma once
#pragma once
#include <engine/graphics/common/RendererImpl.hpp>
#include <engine/graphics/Renderer.hpp>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <engine/data_struct/Buffer.hpp>
namespace VK
{
	using namespace Graphics;
	using namespace Collections;
	class RendererVK : public RendererImpl , public Renderer
	{
	public:
#ifdef _WIN32
		HDC hdc;
#endif
		VkInstance instance;
		VkPhysicalDevice pdev;
		VkDevice dev;
		VkQueue dev_queue;
		VkPhysicalDeviceProperties pdev_prop;
		VkPhysicalDeviceFeatures pdev_features;
		VkPhysicalDeviceMemoryProperties pdev_mem_prop;
		VkFormat depth_format;
		VkFormat color_format;
		VkColorSpaceKHR color_space;
		VkSurfaceKHR surface;
		VkSwapchainKHR swap_chain;
		uint pushCreationQueue( CreationDesc desc );
		LocalArray< VkImage , 10 > swap_chain_images;
		LocalArray< VkImageView , 10 > swap_chain_images_views;
		LocalArray< VkCommandBuffer , 10 > cmd_buffers_per_image;
		struct
		{
			VkSemaphore render_complete;
			VkSemaphore present_complete;
		} semaphores;
		VkSubmitInfo submit_info_skelet;
		VkCommandPool command_pool;
		void mainloop();
	};
}