#pragma once
#include <engine/graphics/vulkan/defines.hpp>
namespace VK
{
	class Buffer
	{
	private:
		VkBuffer buffer;
		VkDeviceMemory memory;
		uint size = 0 , real_size = 0;
		MemoryType mem_type;
		uint INCREMENT = 0x40;
	public:
		void make_space( uint new_size )
		{
			if( new_size > real_size )
			{

			}
		}
	};
}