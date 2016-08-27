#pragma once
#include <engine/graphics/vulkan/defines.hpp>
#include <engine/graphics/vulkan/Device.hpp>
#include <engine/graphics/vulkan/CommandBuffer.hpp>
#include <engine/graphics/vulkan/Queue.hpp>
namespace VK
{
	class Buffer
	{
	private:
		Unique< VkBuffer > buffer;
		Unique< VkDeviceMemory > memory;
		uint size = 0;
		VkDevice dev_raw;
	public:
		static Buffer createStatic(
			Device const &device , BufferTarget usage ,
			MemoryType mem_type , CommandBuffer const &copy_cmd_buffer , Queue const &transfer_queue ,
			void const *data , uint size
		)
		{
			Buffer out;
			Buffer tmp_buffer;
			{
				VkBufferCreateInfo info;
				Allocator::zero( &info );
				info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				info.size = size;
				info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				tmp_buffer.buffer.create( device.getHandle() , info );
				VkMemoryRequirements mem_req;
				tmp_buffer.size = size;
				tmp_buffer.dev_raw = device.getHandle();
				vkGetBufferMemoryRequirements( device.getHandle() , *tmp_buffer.buffer , &mem_req );
				VkMemoryAllocateInfo alloc_info;
				Allocator::zero( &alloc_info );
				alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				alloc_info.allocationSize = mem_req.size;
				alloc_info.memoryTypeIndex = device.getPhysicalDevice().getMemoryIndex( mem_req.memoryTypeBits , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );
				tmp_buffer.memory.create( device.getHandle() , alloc_info );
				vkBindBufferMemory( device.getHandle() , *tmp_buffer.buffer , *tmp_buffer.memory , 0 );
				auto map = tmp_buffer.map();
				Allocator::copy< byte >( ( byte* )map , ( byte* )data , size );
				tmp_buffer.unmap();
			}
			VkBufferCreateInfo info;
			Allocator::zero( &info );
			info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			info.size = size;
			info.usage = getVK( usage );
			out.buffer.create( device.getHandle() , info );
			VkMemoryRequirements mem_req;
			out.size = size;
			out.dev_raw = device.getHandle();
			vkGetBufferMemoryRequirements( device.getHandle() , *out.buffer , &mem_req );
			VkMemoryAllocateInfo alloc_info;
			Allocator::zero( &alloc_info );
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.allocationSize = mem_req.size;
			alloc_info.memoryTypeIndex = device.getPhysicalDevice().getMemoryIndex( mem_req.memoryTypeBits , getVK( mem_type ) );
			out.memory.create( device.getHandle() , alloc_info );
			vkBindBufferMemory( device.getHandle() , *out.buffer , *out.memory , 0 );
			copy_cmd_buffer.begin();
			VkBufferCopy copy;
			Allocator::zero( &copy );
			copy.size = size;
			vkCmdCopyBuffer( copy_cmd_buffer.getHandle() , tmp_buffer.getHandle() , out.getHandle() , 1 , &copy );
			copy_cmd_buffer.end();
			transfer_queue.submitCommandBuffer( copy_cmd_buffer );
			transfer_queue.wait();
			return out;
		}
		uint getSize() const
		{
			return size;
		}
		VkBuffer const &getHandle() const
		{
			return *buffer;
		}
		void *map() const
		{
			void *data;
			vkMapMemory( dev_raw , *memory , 0 , size , 0 , &data );
			return data;
		}
		void unmap() const
		{
			vkUnmapMemory( dev_raw , *memory );
		}
	};
}