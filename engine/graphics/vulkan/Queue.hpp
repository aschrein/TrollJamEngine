#pragma once
#include <engine/graphics/vulkan/defines.hpp>
#include <engine/graphics/vulkan/Unique.hpp>
#include <engine/graphics/vulkan/CommandBuffer.hpp>
#include <engine/graphics/vulkan/Device.hpp>
#include <engine/graphics/vulkan/SwapChain.hpp>
namespace VK
{
	class Queue
	{
		VK_OBJECT( Queue );
	private:
		uint32_t family;
		VkQueue handle;
	public:
		static Queue createGraphicsQueue( Device const &dev , uint index = 0 )
		{
			Queue out;
			out.handle = dev.getGraphicsQueue( index );
			out.family = dev.getGraphicsQueueFamily();
			return out;
		}
		void submitCommandBuffer( CommandBuffer const &command_buffer , VkSemaphore wait_semaphore , VkSemaphore signal_semaphore ) const
		{
			VkSubmitInfo submit_info;
			Allocator::zero( &submit_info );
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.pNext = NULL;
			VkPipelineStageFlags pipeline_stage_flag = VK_PIPELINE_STAGE_TRANSFER_BIT;
			submit_info.pWaitDstStageMask = &pipeline_stage_flag;
			submit_info.pSignalSemaphores = &signal_semaphore;
			submit_info.signalSemaphoreCount = 1;
			submit_info.pWaitSemaphores = &wait_semaphore;
			submit_info.waitSemaphoreCount = 1;
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = &command_buffer.getHandle();
			VKASSERTLOG( vkQueueSubmit( handle , 1 , &submit_info , VK_NULL_HANDLE ) );
		}
		void present( SwapChain const &swap_chain , VkSemaphore wait_semaphore ) const
		{
			VkPresentInfoKHR present_info;
			Allocator::zero( &present_info );
			present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			present_info.waitSemaphoreCount = 1;
			present_info.pWaitSemaphores = &wait_semaphore;
			present_info.pSwapchains = &swap_chain.getHandle();
			present_info.swapchainCount = 1;
			present_info.pImageIndices = &swap_chain.getCurrentImageIndex();
			VKASSERTLOG( vkQueuePresentKHR( handle , &present_info ) );
		}
	};
}