#pragma once
#include <engine/graphics/vulkan/defines.hpp>
#include <engine/graphics/vulkan/Images.hpp>
#include <engine/graphics/vulkan/Buffers.hpp>
#include <engine/graphics/vulkan/Pass.hpp>
#include <engine/math/vec.hpp>
#include <engine/graphics/vulkan/Unique.hpp>
namespace VK
{
	using namespace Math;
	struct CommandBuffer
	{
		uint32_t family;
		Unique< VkCommandBuffer > handle;
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
			vkCmdClearColorImage( *handle , image.handle ,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL , cv , 1 , &range );
		}
		void ImageBarrier( Image const &image , VkImageSubresourceRange range ,
			VkAccessFlags src_access_flags , VkAccessFlags dst_access_flags , VkImageLayout layout ) const
		{
			VkImageMemoryBarrier barrier;
			Allocator::zero( &barrier );
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcAccessMask = src_access_flags;
			barrier.dstAccessMask = dst_access_flags;
			barrier.oldLayout = image.layout;
			barrier.newLayout = layout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.srcQueueFamilyIndex = family;
			barrier.dstQueueFamilyIndex = family;
			barrier.image = image.handle;
			barrier.subresourceRange = range;
			vkCmdPipelineBarrier( *handle , VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT , VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT ,
				0 , 0 , nullptr , 0 , nullptr , 1 , &barrier );
			image.layout = layout;
		}
		void execSecondary( CommandBuffer const &cb ) const
		{
			vkCmdExecuteCommands( getHandle() , 1 , &cb.getHandle() );
		}
		void copy( Buffer const &dst , Buffer const &src , VkBufferCopy copy_desc ) const
		{
			vkCmdCopyBuffer( *handle , src.handle , dst.handle , 1 , &copy_desc );
		}
		void copy( Image const &dst , Buffer const &src , VkBufferImageCopy const *ranges , uint range_count ) const
		{
			vkCmdCopyBufferToImage(
				*handle ,
				src.handle ,
				dst.handle ,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ,
				range_count ,
				ranges
			);
		}
		void beginPass( VkRenderPass pass , VkFramebuffer framebuffer , int4 render_area , VkClearValue const *clear_color , uint attachment_count ) const
		{
			VkRenderPassBeginInfo render_pass_begin_info;
			Allocator::zero( &render_pass_begin_info );
			render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_begin_info.renderPass = pass;
			render_pass_begin_info.framebuffer = framebuffer;
			render_pass_begin_info.renderArea =
			{
				{ render_area.x , render_area.y } ,
				{ ( uint )render_area.z , ( uint )render_area.w }
			};
			render_pass_begin_info.clearValueCount = attachment_count;
			render_pass_begin_info.pClearValues = clear_color;
			vkCmdBeginRenderPass( getHandle() , &render_pass_begin_info , VK_SUBPASS_CONTENTS_INLINE );
		}
		void bindGraphicsPipeline( VkPipeline pipeline ) const
		{
			vkCmdBindPipeline( getHandle() , VK_PIPELINE_BIND_POINT_GRAPHICS , pipeline );
		}
		void bindGraphicsDescriptorSet( VkPipelineLayout pipeline_layout , VkDescriptorSet desc_set , uint set_id ) const
		{
			vkCmdBindDescriptorSets( getHandle() , VK_PIPELINE_BIND_POINT_GRAPHICS , pipeline_layout , set_id , 1 , &desc_set , 0 , NULL );
		}
		void blit( Image const &dst , Image const &src , VkImageBlit const *blits , uint blit_count , VkFilter filter ) const
		{
			vkCmdBlitImage(
				getHandle() ,
				src.handle ,
				src.layout ,
				dst.handle ,
				dst.layout ,
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
		static CommandPool create( VkDevice dev , uint queue_family )
		{
			VkCommandPoolCreateInfo command_pool_create_info = {};
			command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			command_pool_create_info.queueFamilyIndex = queue_family;
			command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			CommandPool out;
			out.handle.create( dev , command_pool_create_info );
			out.family = queue_family;
			out.dev_raw = dev;
			return out;
		}
		CommandBuffer createPrimaryCommandBuffer() const
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
		CommandBuffer createSecondaryCommandBuffer() const
		{
			VkCommandBufferAllocateInfo cmd_buf_alloc_info = {};
			cmd_buf_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmd_buf_alloc_info.commandPool = *handle;
			cmd_buf_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			cmd_buf_alloc_info.commandBufferCount = 1;
			CommandBuffer out;
			out.handle.create( dev_raw , *handle , cmd_buf_alloc_info );
			out.family = family;
			return out;
		}
	};
}