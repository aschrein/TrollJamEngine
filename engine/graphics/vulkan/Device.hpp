#pragma once
#include <engine/graphics/vulkan/defines.hpp>
#include <engine/graphics/vulkan/Unique.hpp>
#include <engine/graphics/vulkan/Instance.hpp>
namespace VK
{
	class Device
	{
		friend class Instance;
		VK_OBJECT( Device );
	private:
		Unique< VkDevice > handle;
		uint32_t graphics_family_index;
		uint32_t graphics_queue_count;
	public:
		static Device createGraphicsDevice( Instance const &instance , uint32_t graphics_queue_count = 1 )
		{
			float queue_priorities[] = { 0.0f };
			VkDeviceQueueCreateInfo vkdevqueueinfo;
			Allocator::zero( &vkdevqueueinfo );
			vkdevqueueinfo.queueFamilyIndex = instance.getPhysicalDevice().getGraphicsQueueFamily();
			vkdevqueueinfo.queueCount = graphics_queue_count;
			vkdevqueueinfo.pQueuePriorities = queue_priorities;
			VkDeviceCreateInfo vkdevinfo;
			Allocator::zero( &vkdevinfo );
			vkdevinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			vkdevinfo.queueCreateInfoCount = 1;
			vkdevinfo.pQueueCreateInfos = &vkdevqueueinfo;
			Device out ;
			out.handle.create( instance.getPhysicalDevice().getHandle() , vkdevinfo );
			out.graphics_family_index = instance.getPhysicalDevice().getGraphicsQueueFamily();
			out.graphics_queue_count = graphics_queue_count;
			return out;
		}
		Unique< VkSemaphore > createSemaphore()
		{
			VkSemaphoreCreateInfo semaphore_create_info;
			Allocator::zero( &semaphore_create_info );
			semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphore_create_info.pNext = NULL;
			semaphore_create_info.flags = 0;
			Unique< VkSemaphore > out;
			out.create( *handle , semaphore_create_info );
			return out;
		}
		Unique< VkShaderModule > createShaderModule( FileImage const *image )
		{
			VkShaderModuleCreateInfo shader_module_create_info;
			Allocator::zero( &shader_module_create_info );
			shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shader_module_create_info.codeSize = image->getView().getLimit();
			shader_module_create_info.pCode = ( const uint32_t* )image->getView().getRaw();
			Unique< VkShaderModule > out;
			out.create( *handle , shader_module_create_info );
			return out;
		}
		VkQueue getGraphicsQueue( uint index = 0 ) const
		{
			VkQueue queue;
			vkGetDeviceQueue( *handle , graphics_family_index , index , &queue );
			return queue;
		}
		uint getGraphicsQueueCount() const
		{
			return graphics_queue_count;
		}
		uint getGraphicsQueueFamily() const
		{
			return graphics_family_index;
		}

		VkDevice getHandle() const
		{
			return *handle;
		}
	};
}