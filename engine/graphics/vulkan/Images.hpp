#pragma once
#include <engine/graphics/vulkan/defines.hpp>
namespace VK
{
	struct ImageView
	{
		VkImageView handle;
		VkFormat format;
		VkImageSubresourceRange range;
		VkComponentMapping mapping =
		{
			VK_COMPONENT_SWIZZLE_R ,
			VK_COMPONENT_SWIZZLE_G ,
			VK_COMPONENT_SWIZZLE_B ,
			VK_COMPONENT_SWIZZLE_A
		};
	};
	struct Image
	{
		VkImage handle;
		VkFormat format;
		mutable VkImageLayout layout;
		uint memory_offset;
		uint memory_size;
		uint width , height , depth;
		uint mip_levels;
		uint layers;
		VkImageUsageFlags usage;
	};
}