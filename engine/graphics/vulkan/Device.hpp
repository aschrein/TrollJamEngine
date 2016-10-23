#pragma once
#include <engine/graphics/vulkan/defines.hpp>
#include <engine/graphics/vulkan/Unique.hpp>
#include <engine/graphics/vulkan/Instance.hpp>
#include <engine/graphics/vulkan/Images.hpp>
#include <engine/graphics/vulkan/Pass.hpp>
#include <engine/graphics/vulkan/Pipeline.hpp>
#include <engine/graphics/vulkan/Buffers.hpp>
#include <engine/graphics/Graphics.hpp>
namespace VK
{
	class Device
	{
		VK_OBJECT( Device );
	private:
		
		VkDevice handle;
		uint32_t graphics_queue_count;
		Instance::PhysicalDevice pdev;
		LocalArray< VkQueue , 10 > graphics_queues , compute_queues;
		mutable Memory dev_mem;
		mutable Memory dev_texture_mem;
		mutable Memory host_mem;
	public:
		Memory &getMemory( MemoryType mem_type ) const
		{
			Memory *memory = nullptr;
			switch( mem_type )
			{
			case MemoryType::HOST:
				memory = &host_mem;
				break;
			case MemoryType::DEV_BUFFER:
				memory = &dev_mem;
				break;
			case MemoryType::DEV_TEXTURE:
				memory = &dev_texture_mem;
				break;
			}
			return *memory;
		}
		static Device createGraphicsDevice( Instance const &instance , uint32_t graphics_queue_count = 1 , uint compute_queue_count = 1 )
		{
			float priorities[ 10 ] = { 0.0f };
			VkDeviceQueueCreateInfo vkdevqueueinfo[ 2 ];
			Allocator::zero( vkdevqueueinfo , 2 );
			vkdevqueueinfo[ 0 ].queueFamilyIndex = instance.getPhysicalDevice().getGraphicsQueueFamily();
			vkdevqueueinfo[ 0 ].queueCount = graphics_queue_count;
			vkdevqueueinfo[ 0 ].pQueuePriorities = priorities;
			vkdevqueueinfo[ 1 ].queueFamilyIndex = instance.getPhysicalDevice().getComputeQueueFamily();
			vkdevqueueinfo[ 1 ].queueCount = compute_queue_count;
			vkdevqueueinfo[ 1 ].pQueuePriorities = priorities;
			VkDeviceCreateInfo vkdevinfo;
			Allocator::zero( &vkdevinfo );
			vkdevinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			vkdevinfo.queueCreateInfoCount = 2;
			vkdevinfo.pQueueCreateInfos = vkdevqueueinfo;
			Device out;
			out.pdev = instance.getPhysicalDevice();
			out.handle = Factory< VkDevice >::create( instance.getPhysicalDevice().getHandle() , vkdevinfo );
			ito( graphics_queue_count )
			{
				out.graphics_queues.push( out.getGraphicsQueue( i ) );
			}
			ito( compute_queue_count )
			{
				out.compute_queues.push( out.getComputeQueue( i ) );
			}
			VkMemoryAllocateInfo alloc_info;
			Allocator::zero( &alloc_info );
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.allocationSize = 256 * 100000;
			alloc_info.memoryTypeIndex = instance.getPhysicalDevice().getMemoryIndex( 1665 , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );
			out.host_mem.handle = Factory< VkDeviceMemory >::create( out.handle , alloc_info );
			out.host_mem.size = alloc_info.allocationSize;
			out.host_mem.rm.setLimit( alloc_info.allocationSize );

			alloc_info.allocationSize = 256 * 100000;
			alloc_info.memoryTypeIndex = instance.getPhysicalDevice().getMemoryIndex( 1665 , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
			out.dev_mem.handle = Factory< VkDeviceMemory >::create( out.handle , alloc_info );
			out.dev_mem.size = alloc_info.allocationSize;
			out.dev_mem.rm.setLimit( alloc_info.allocationSize );

			alloc_info.allocationSize = 256 * 100000;
			alloc_info.memoryTypeIndex = instance.getPhysicalDevice().getMemoryIndex( 258 , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
			out.dev_texture_mem.handle = Factory< VkDeviceMemory >::create( out.handle , alloc_info );
			out.dev_texture_mem.size = alloc_info.allocationSize;
			out.dev_texture_mem.rm.setLimit( alloc_info.allocationSize );

			out.graphics_queue_count = graphics_queue_count;
			return out;
		}
		Instance::PhysicalDevice const &getPhysicalDevice() const
		{
			return pdev;
		}
		VkQueue getGraphicsQueue( uint index = 0 ) const
		{
			VkQueue queue;
			vkGetDeviceQueue( handle , pdev.getGraphicsQueueFamily() , index , &queue );
			return queue;
		}
		VkQueue getComputeQueue( uint index = 0 ) const
		{
			VkQueue queue;
			vkGetDeviceQueue( handle , pdev.getComputeQueueFamily() , index , &queue );
			return queue;
		}
		void *map( MemoryType mem_type , uint offset , uint size )
		{
			void *data;
			vkMapMemory( handle , getMemory( mem_type ).handle , offset , size , 0 , &data );
			return data;
		}
		void unmap( MemoryType mem_type )
		{
			vkUnmapMemory( handle , getMemory( mem_type ).handle );
		}
		void submitGraphicsCommandBuffer( uint queue_index , CommandBuffer const &command_buffer , VkSemaphore wait_semaphore = VK_NULL_HANDLE , VkSemaphore signal_semaphore = VK_NULL_HANDLE ) const
		{
			submitCommandBuffer( graphics_queues[ queue_index ] , command_buffer , wait_semaphore , signal_semaphore );
		}
		void submitComputeCommandBuffer( uint queue_index , CommandBuffer const &command_buffer , VkSemaphore wait_semaphore = VK_NULL_HANDLE , VkSemaphore signal_semaphore = VK_NULL_HANDLE ) const
		{
			submitCommandBuffer( compute_queues[ queue_index ] , command_buffer , wait_semaphore , signal_semaphore );
		}
		void submitCommandBuffer( VkQueue queue , CommandBuffer const &command_buffer , VkSemaphore wait_semaphore = VK_NULL_HANDLE , VkSemaphore signal_semaphore = VK_NULL_HANDLE ) const
		{
			VkSubmitInfo submit_info;
			Allocator::zero( &submit_info );
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.pNext = NULL;
			VkPipelineStageFlags pipeline_stage_flag = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			submit_info.pWaitDstStageMask = &pipeline_stage_flag;

			if( signal_semaphore )
			{
				submit_info.pSignalSemaphores = &signal_semaphore;
				submit_info.signalSemaphoreCount = 1;
			}
			if( wait_semaphore )
			{
				submit_info.pWaitSemaphores = &wait_semaphore;
				submit_info.waitSemaphoreCount = 1;
			}
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = &command_buffer.getHandle();
			VKASSERTLOG( vkQueueSubmit( queue , 1 , &submit_info , VK_NULL_HANDLE ) );
		}
		void present( uint queue_index , VkSwapchainKHR swap_chain , uint image_index , VkSemaphore wait_semaphore = VK_NULL_HANDLE ) const
		{
			VkPresentInfoKHR present_info;
			Allocator::zero( &present_info );
			present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			if( wait_semaphore )
			{
				present_info.waitSemaphoreCount = 1;
				present_info.pWaitSemaphores = &wait_semaphore;
			}
			present_info.pSwapchains = &swap_chain;
			present_info.swapchainCount = 1;
			present_info.pImageIndices = &image_index;
			VKASSERTLOG( vkQueuePresentKHR( graphics_queues[ queue_index ] , &present_info ) );
		}
		void waitIdleGraphics( uint queue_index ) const
		{
			vkQueueWaitIdle( graphics_queues[ queue_index ] );
		}
		void waitIdleCompute( uint queue_index ) const
		{
			vkQueueWaitIdle( compute_queues[ queue_index ] );
		}
		VkDevice getHandle() const
		{
			return handle;
		}
	};
	
}