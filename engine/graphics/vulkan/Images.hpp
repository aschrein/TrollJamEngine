#pragma once
#include <engine/graphics/vulkan/defines.hpp>
namespace VK
{
	class ImageView
	{
		friend class SwapChain;
	private:
		Unique< VkImageView > handle;
		VkImageViewCreateInfo info;
	public:
		VkImageSubresourceRange const &getSubresourceRange() const
		{
			return info.subresourceRange;
		}
		VkImageView const &getHandle() const
		{
			return *handle;
		}
	};
	class Image
	{
		friend class SwapChain;
	private:
		Unique< VkImage > handle;
		ImageView image_view;
		VkFormat format;
	public:
		VkImage const &getHandle() const
		{
			return *handle;
		}
		ImageView const &getView() const
		{
			return image_view;
		}
	};
	class ImageSrc
	{

	};
	class ImageTarget
	{

	};
}