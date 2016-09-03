#pragma once
#include <engine/graphics/vulkan/defines.hpp>
#include <engine/graphics/vulkan/Device.hpp>
#include <engine/mem/RangeMaster.hpp>
namespace VK
{
	class Memory
	{
		VK_OBJECT( Memory );
	private:
		Unique< VkDeviceMemory > memory;
		uint size = 0;
		VkDevice dev_raw = VK_NULL_HANDLE;
		RangeManager rm;
	public:
		static Memory create( Device const &device , uint size , uint type_index )
		{
			Memory out;
			
			VkMemoryAllocateInfo alloc_info;
			Allocator::zero( &alloc_info );
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.allocationSize = size;
			alloc_info.memoryTypeIndex = type_index;
			out.memory.create( device.getHandle() , alloc_info );
			out.size = size;
			out.rm.setLimit( size );
			out.dev_raw = device.getHandle();
			return out;
		}
		VkDeviceMemory const &getHandle() const
		{
			return *memory;
		}
		uint allocate( uint size )
		{
			return rm.allocate( size );
		}
		void free( uint offset )
		{
			rm.free( offset );
		}
	};
	struct MemoryContainer
	{
		Memory *ptr = nullptr;
		MemoryContainer() = default;
		MemoryContainer( Memory *ptr ) :
			ptr( ptr )
		{}
		MemoryContainer( MemoryContainer  const & ) = delete;
		MemoryContainer &operator=( MemoryContainer  const & ) = delete;
		MemoryContainer( MemoryContainer &&mc )
		{
			*this = std::move( mc );
		}
		MemoryContainer &operator=( MemoryContainer &&mc )
		{
			ptr = mc.ptr;
			mc.ptr = nullptr;
			return *this;
		}
	};
	class Buffer
	{
		VK_OBJECT( Buffer );
	private:
		Unique< VkBuffer > buffer;
		Optional< MemoryContainer , Unique< VkDeviceMemory > > memory;
		uint offset = 0;
		uint size = 0;
		VkDevice dev_raw = VK_NULL_HANDLE;
	public:
		static Buffer create(
			Device const &device , VkBufferUsageFlags usage ,
			uint mem_type , uint size ,
			void const *data = nullptr
		)
		{
			Buffer out;
			VkBufferCreateInfo info;
			Allocator::zero( &info );
			info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			info.size = size;
			info.usage = usage;
			out.buffer.create( device.getHandle() , info );
			VkMemoryRequirements mem_req;
			vkGetBufferMemoryRequirements( device.getHandle() , *out.buffer , &mem_req );
			out.size = mem_req.size;
			out.dev_raw = device.getHandle();
			out.offset = 0;
			VkMemoryAllocateInfo alloc_info;
			Allocator::zero( &alloc_info );
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.allocationSize = mem_req.size;
			alloc_info.memoryTypeIndex = device.getPhysicalDevice().getMemoryIndex( mem_req.memoryTypeBits , mem_type );
			Unique< VkDeviceMemory > self_memory;
			self_memory.create( device.getHandle() , alloc_info );
			vkBindBufferMemory( device.getHandle() , *out.buffer , *self_memory , 0 );
			out.memory.setSecond( std::move( self_memory ) );
			if( data )
			{
				auto map = out.map();
				Allocator::copy< byte >( ( byte* )map , ( byte* )data , size );
				out.unmap();
			}
			return out;
		}
		static Buffer create(
			Device const &device , VkBufferUsageFlags usage , Memory *memory , uint size )
		{
			Buffer out;
			VkBufferCreateInfo info;
			Allocator::zero( &info );
			info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			info.size = size;
			info.usage = usage;
			out.buffer.create( device.getHandle() , info );
			VkMemoryRequirements mem_req;
			vkGetBufferMemoryRequirements( device.getHandle() , *out.buffer , &mem_req );
			out.size = mem_req.size;
			out.dev_raw = device.getHandle();
			out.offset = memory->allocate( mem_req.size );
			vkBindBufferMemory( device.getHandle() , *out.buffer , memory->getHandle() , out.offset );
			out.memory.setFirst( MemoryContainer( memory ) );
			return out;
		}
		~Buffer()
		{
			if( memory.isFirst() && memory.getFirst().ptr )
			{
				memory.getFirst().ptr->free( offset );
			}
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
			vkMapMemory( dev_raw , memory.isFirst() ? memory.getFirst().ptr->getHandle() : *memory.getSecond() , offset , size , 0 , &data );
			return data;
		}
		void unmap() const
		{
			vkUnmapMemory( dev_raw , memory.isFirst() ? memory.getFirst().ptr->getHandle() : *memory.getSecond() );
		}
	};
}