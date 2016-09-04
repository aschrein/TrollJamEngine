#pragma once
#include <engine/graphics/vulkan/defines.hpp>
#include <engine/graphics/vulkan/Buffers.hpp>
namespace VK
{
	class ImageView
	{
		friend class Image;
		friend class SwapChain;
	private:
		Unique< VkImageView > handle;
		VkFormat format;
		VkImageSubresourceRange range;
		VkComponentMapping mapping =
		{
			VK_COMPONENT_SWIZZLE_R ,
			VK_COMPONENT_SWIZZLE_G ,
			VK_COMPONENT_SWIZZLE_B ,
			VK_COMPONENT_SWIZZLE_A
		};
	public:
		VK_OBJECT( ImageView );
		VkImageSubresourceRange const &getSubresourceRange() const
		{
			return range;
		}
		VkImageView const &getHandle() const
		{
			return *handle;
		}
		VkFormat getFormat() const
		{
			return format;
		}
	};
	class Image
	{
		friend class SwapChain;
	private:
		VkDevice dev_raw;
		Unique< VkImage > handle;
		VkFormat format;
		mutable VkImageLayout layout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		mutable VkAccessFlags access = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		MemoryContainer memory;
		uint offset = 0;
		uint width , height , depth = 1;
		uint mip_levels = 1;
		uint layers = 1;
		VkImageUsageFlags usage;
	public:
		VK_OBJECT( Image );
		static Image create(
			Device const &device , Memory *memory , uint width , uint height ,
			uint mip_levels , uint layers , VkImageLayout initial_layout , VkFormat format , VkImageUsageFlags usage_flags
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

			VkMemoryRequirements mem_req;
			
			VkResult err;
			Image out;
			out.handle.create( device.getHandle() , image_info );
			vkGetImageMemoryRequirements( device.getHandle() , out.getHandle() , &mem_req );
			out.depth = 1;
			out.dev_raw = device.getHandle();
			out.format = format;
			out.height = height;
			out.width = width;
			out.layers = layers;
			out.mip_levels = mip_levels;
			out.offset = memory->allocate( mem_req.size );
			out.usage = usage_flags;
			vkBindImageMemory( device.getHandle() , out.getHandle() , memory->getHandle() , out.offset );
			out.memory.ptr = memory;
			return out;
		}
		VkImage const &getHandle() const
		{
			return *handle;
		}
		ImageView createView( VkComponentMapping mapping , VkImageSubresourceRange subresource_range ) const
		{
			VkImageViewCreateInfo image_view_info;
			Allocator::zero( &image_view_info );
			image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_info.format = format;
			image_view_info.components = mapping;
			image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			image_view_info.subresourceRange = subresource_range;
			image_view_info.image = *handle;
			ImageView image_view;
			image_view.handle.create( dev_raw , image_view_info );
			image_view.range = image_view_info.subresourceRange;
			image_view.format = format;
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
		uint3 getSize() const
		{
			return{ width , height , depth };
		}
		~Image()
		{
			if( memory.ptr )
			{
				memory.ptr->free( offset );
			}
		}
	};
	class Attachment
	{
	private:
		friend class SwapChain;
		Image image;
		ImageView view;
		Unique< VkSampler > sampler;
	public:
		VK_OBJECT( Attachment );
		static Attachment createRenderTarget( Device const &device , Memory *memory , Graphics::RenderTargetInfo const &info )
		{
			Attachment out;
			out.image = Image::create( device , memory , info.size.x , info.size.y , 1 , 1 ,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL , getVK( info.component_mapping ) ,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
			);
			out.view = out.image.createView(
			{ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A } ,
			{ VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 }
			);
			VkSamplerCreateInfo sampler;
			Allocator::zero( &sampler );
			sampler.magFilter = VK_FILTER_NEAREST;
			sampler.minFilter = VK_FILTER_NEAREST;
			sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			sampler.compareOp = VK_COMPARE_OP_NEVER;
			sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			out.sampler.create( device.getHandle() , sampler );
			return out;
		}
		ImageView const &getView() const
		{
			return view;
		}
		Image const &getImage() const
		{
			return image;
		}
		VkSampler const &getSampler() const
		{
			return *sampler;
		}
	};
}