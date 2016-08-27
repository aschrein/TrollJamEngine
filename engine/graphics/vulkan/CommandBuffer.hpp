#pragma once
#include <engine/graphics/vulkan/defines.hpp>
#include <engine/graphics/vulkan/Images.hpp>
#include <engine/graphics/vulkan/Device.hpp>
#include <engine/math/vec.hpp>
namespace VK
{
	using namespace Math;
	class CommandBuffer
	{
		friend class CommandPool;
	private:
		uint32_t family;
		Unique< VkCommandBuffer > handle;
	public:
		VkCommandBuffer const &getHandle() const
		{
			return *handle;
		}
		void begin() const
		{
			VkCommandBufferBeginInfo cmd_begin_info;
			Allocator::zero( &cmd_begin_info );
			cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			vkBeginCommandBuffer( *handle , &cmd_begin_info );
		}
		void clearImage( Image const &image , f4 const &clear_color ) const
		{
			VkClearColorValue cv[] =
			{
				clear_color.x , clear_color.y , clear_color.z , clear_color.w
			};
			vkCmdClearColorImage( *handle , image.getHandle() ,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL , cv , 1 , &image.getView().getSubresourceRange() );
		}
		void ImageBarrier( Image const &image , ImageState const &from , ImageState const &to ) const
		{
			VkImageMemoryBarrier barrier;
			Allocator::zero( &barrier );
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcAccessMask = getVK( from.access_mask );
			barrier.dstAccessMask = getVK( to.access_mask );
			barrier.oldLayout = getVK( from.layout );
			barrier.newLayout = getVK( to.layout );
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.srcQueueFamilyIndex = family;
			barrier.dstQueueFamilyIndex = family;
			barrier.image = image.getHandle();
			barrier.subresourceRange = image.getView().getSubresourceRange();
			vkCmdPipelineBarrier( *handle , VK_PIPELINE_STAGE_TRANSFER_BIT , VK_PIPELINE_STAGE_TRANSFER_BIT ,
				0 , 0 , nullptr , 0 , nullptr , 1 , &barrier );
		}
		void end() const
		{
			VKASSERTLOG( vkEndCommandBuffer( *handle ) );
		}
	};
	class CommandPool
	{
		friend class Device;
		friend class Instance;
	private:
		Unique< VkCommandPool > handle;
		VkDevice dev_raw;
		uint32_t family;
	public:
		static CommandPool create( Device const &dev , uint queue_family )
		{
			VkCommandPoolCreateInfo command_pool_create_info = {};
			command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			command_pool_create_info.queueFamilyIndex = queue_family;
			command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			CommandPool out;
			out.handle.create( dev.getHandle() , command_pool_create_info );
			out.family = queue_family;
			out.dev_raw = dev.getHandle();
			return out;
		}
		CommandBuffer createCommandBuffer() const
		{
			VkCommandBufferAllocateInfo cmd_buf_alloc_info = {};
			cmd_buf_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmd_buf_alloc_info.commandPool = *handle;
			cmd_buf_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmd_buf_alloc_info.commandBufferCount = 1;
			CommandBuffer out;
			out.handle.create( dev_raw , *handle , cmd_buf_alloc_info );
			out.family = family;
			return out;
		}
	};
}