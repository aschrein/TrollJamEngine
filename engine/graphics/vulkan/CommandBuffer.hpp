#pragma once
#include <engine/graphics/vulkan/defines.hpp>
#include <engine/graphics/vulkan/Images.hpp>
#include <engine/graphics/vulkan/Device.hpp>
#include <engine/graphics/vulkan/Buffers.hpp>
#include <engine/graphics/vulkan/Pass.hpp>
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
		void clearImage( Image const &image , VkImageSubresourceRange range , float4 const &clear_color ) const
		{
			VkClearColorValue cv[] =
			{
				clear_color.x , clear_color.y , clear_color.z , clear_color.w
			};
			vkCmdClearColorImage( *handle , image.getHandle() ,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL , cv , 1 , &range );
		}
		void ImageBarrier( Image const &image , VkImageSubresourceRange range , VkAccessFlags access_flags , VkImageLayout layout ) const
		{
			VkImageMemoryBarrier barrier;
			Allocator::zero( &barrier );
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcAccessMask = image.getAccessFlags();
			barrier.dstAccessMask = access_flags;
			barrier.oldLayout = image.getLayout();
			barrier.newLayout = layout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.srcQueueFamilyIndex = family;
			barrier.dstQueueFamilyIndex = family;
			barrier.image = image.getHandle();
			barrier.subresourceRange = range;
			vkCmdPipelineBarrier( *handle , VK_PIPELINE_STAGE_ALL_COMMANDS_BIT , VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT ,
				0 , 0 , nullptr , 0 , nullptr , 1 , &barrier );
		}
		void copy( Buffer const &dst , Buffer const &src , uint size ) const
		{
			VkBufferCopy copy;
			Allocator::zero( &copy );
			copy.size = size;
			vkCmdCopyBuffer( *handle , src.getHandle() , dst.getHandle() , 1 , &copy );
		}
		void copy( Image const &dst , Buffer const &src , VkBufferImageCopy const *ranges , uint range_count ) const
		{
			vkCmdCopyBufferToImage(
				*handle ,
				src.getHandle() ,
				dst.getHandle() ,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ,
				range_count ,
				ranges
			);
		}
		void beginPass( Pass const &pass , uint4 render_area , VkClearValue const *clear_color ) const
		{
			VkRenderPassBeginInfo render_pass_begin_info;
			Allocator::zero( &render_pass_begin_info );
			render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_begin_info.renderPass = pass.getPass();
			render_pass_begin_info.framebuffer = pass.getFrameBuffer();
			render_pass_begin_info.renderArea =
			{
				{ render_area.x , render_area.y } ,
				{ render_area.z , render_area.w }
			};
			render_pass_begin_info.clearValueCount = pass.getAttachmentCount();
			render_pass_begin_info.pClearValues = clear_color;
			vkCmdBeginRenderPass( getHandle() , &render_pass_begin_info , VK_SUBPASS_CONTENTS_INLINE );
			vkCmdBindPipeline( getHandle() , VK_PIPELINE_BIND_POINT_GRAPHICS , pass.getPipeline() );
			vkCmdBindDescriptorSets( getHandle() , VK_PIPELINE_BIND_POINT_GRAPHICS , pass.getPipelineLayout() , 0 , 1 , &pass.getDescriptorSet() , 0 , NULL );
		}
		void blit( Image const &dst , Image const &src , VkImageBlit const *blits , uint blit_count , VkFilter filter ) const
		{
			vkCmdBlitImage(
				getHandle() ,
				src.getHandle() ,
				src.getLayout() ,
				dst.getHandle() ,
				dst.getLayout() ,
				blit_count ,
				blits ,
				filter
			);
		}
		void endPass() const
		{
			vkCmdEndRenderPass( getHandle() );
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