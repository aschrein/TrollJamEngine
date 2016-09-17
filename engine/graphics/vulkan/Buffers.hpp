#pragma once
#include <engine/graphics/vulkan/defines.hpp>
#include <engine/mem/RangeMaster.hpp>
namespace VK
{
	struct Memory
	{
		VkDeviceMemory handle;
		uint size = 0;
		RangeManager rm;
		uint allocate( uint size )
		{
			return rm.allocate( size );
		}
		void free( uint offset )
		{
			rm.free( offset );
		}
	};
	struct Buffer
	{
		VkBuffer handle;
		MemoryType mem_type;
		uint offset;
		uint size;
		VkBufferUsageFlags usage;
	};
}