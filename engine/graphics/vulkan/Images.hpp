#pragma once
#include <engine/graphics/vulkan/defines.hpp>
#include <engine/graphics/vulkan/Buffers.hpp>
namespace VK
{
	class ImageView
	{
		friend class Image;
		friend class SwapChain;
		//VK_OBJECT( ImageView );
	private:
		Unique< VkImageView > handle;
		VkImageSubresourceRange range;
		VkComponentMapping mapping =
		{
			VK_COMPONENT_SWIZZLE_R ,
			VK_COMPONENT_SWIZZLE_G ,
			VK_COMPONENT_SWIZZLE_B ,
			VK_COMPONENT_SWIZZLE_A
		};
	public:
		VkImageSubresourceRange const &getSubresourceRange() const
		{
			return range;
		}
		VkImageView const &getHandle() const
		{
			return *handle;
		}
	};
	class Image
	{
		friend class SwapChain;
		//VK_OBJECT( Image );
		
	private:
		VkDevice dev_raw;
		Unique< VkImage > handle;
		VkFormat format;
		mutable VkImageLayout layout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		mutable VkAccessFlags access = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		Optional< MemoryContainer , Unique< VkDeviceMemory > > memory;
		uint offset = 0;
		uint width , height , depth = 1;
		uint mip_levels = 1;
		uint layers = 1;
		VkImageUsageFlags usage;
	public:
		Image() = default;
		Image( Image && ) = default;
		Image &operator=( Image && ) = default;
		static Image create2D(
			Device const &device , uint mem_type , uint width , uint height ,
			uint mip_levels , uint layers , VkFormat format , VkImageUsageFlags usage_flags
		)
		{
			VkImageCreateInfo image_info;
			Allocator::zero( &image_info );
			image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image_info.pNext = NULL;
			image_info.imageType = VK_IMAGE_TYPE_2D;
			image_info.format = format;
			image_info.extent = { width, height, 1 };
			image_info.mipLevels = mip_levels;
			image_info.arrayLayers = layers;
			image_info.samples = VK_SAMPLE_COUNT_1_BIT;
			image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
			image_info.usage = usage_flags;
			image_info.flags = 0;
			image_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
			VkMemoryAllocateInfo mem_alloc = {};
			mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			mem_alloc.pNext = NULL;
			mem_alloc.allocationSize = 0;
			mem_alloc.memoryTypeIndex = 0;

			VkMemoryRequirements memReqs;
			VkResult err;
			Image out;
			out.depth = 1;
			out.dev_raw = device.getHandle();
			out.format = format;
			out.height = height;
			out.width = width;
			out.layers = layers;
			out.mip_levels = mip_levels;
			out.offset = 0;
			out.usage = usage_flags;
			out.handle.create( device.getHandle() , image_info );
			vkGetImageMemoryRequirements( device.getHandle() , out.getHandle() , &memReqs );
			mem_alloc.allocationSize = memReqs.size;
			Unique< VkDeviceMemory > dev_mem;
			dev_mem.create( device.getHandle() , mem_alloc );
			vkBindImageMemory( device.getHandle() , out.getHandle() , *dev_mem , 0 );
			out.memory.setSecond( std::move( dev_mem ) );
			return out;
		}
		VkImage const &getHandle() const
		{
			return *handle;
		}
		ImageView createView( VkComponentMapping mapping , VkImageAspectFlags aspect_flags ) const
		{
			VkImageViewCreateInfo image_view_info;
			Allocator::zero( &image_view_info );
			image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_info.format = format;
			image_view_info.components = mapping;
			image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			{
				VkImageSubresourceRange subresource_range;
				VkImageAspectFlags aspect;
				subresource_range.aspectMask = aspect_flags;
				subresource_range.baseMipLevel = 0;
				subresource_range.levelCount = mip_levels;
				subresource_range.layerCount = layers;
				subresource_range.baseArrayLayer = 0;
				image_view_info.subresourceRange = subresource_range;
			}
			image_view_info.image = *handle;
			ImageView image_view;
			image_view.handle.create( dev_raw , image_view_info );
			image_view.range = image_view_info.subresourceRange;
			return image_view;
		}
		VkImageLayout getLayout() const
		{
			return layout;
		}
		VkAccessFlags getAccessFlags() const
		{
			return access;
		}
		void setAccessFlags( VkAccessFlags flags ) const
		{
			access = flags;
		}
		void setLayout( VkImageLayout l ) const
		{
			layout = l;
		}
		~Image()
		{
			if( memory.isFirst() && memory.getFirst().ptr )
			{
				memory.getFirst().ptr->free( offset );
			}
		}
	};
}