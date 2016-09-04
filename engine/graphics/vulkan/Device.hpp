#pragma once
#include <engine/graphics/vulkan/defines.hpp>
#include <engine/graphics/vulkan/Unique.hpp>
#include <engine/graphics/vulkan/Instance.hpp>
namespace VK
{
	class Device
	{
		VK_OBJECT( Device );
	private:
		Unique< VkDevice > handle;
		uint32_t graphics_queue_count;
		Instance::PhysicalDevice pdev;
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
			Device out;
			out.pdev = instance.getPhysicalDevice();
			out.handle.create( instance.getPhysicalDevice().getHandle() , vkdevinfo );
			out.graphics_queue_count = graphics_queue_count;
			return out;
		}
		Instance::PhysicalDevice const &getPhysicalDevice() const
		{
			return pdev;
		}
		Unique< VkSemaphore > createSemaphore() const
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
		Unique< VkShaderModule > createShaderModule( void const *data , uint size ) const
		{
			VkShaderModuleCreateInfo shader_module_create_info;
			Allocator::zero( &shader_module_create_info );
			shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shader_module_create_info.codeSize = size;
			shader_module_create_info.pCode = ( const uint32_t* )data;
			Unique< VkShaderModule > out;
			out.create( *handle , shader_module_create_info );
			return out;
		}
		Unique< VkSampler > createSampler( Graphics::SamplerInfo const &info )
		{
			VkSamplerCreateInfo sampler;
			Allocator::zero( &sampler );
			sampler.magFilter = getVK( info.mag_filter );
			sampler.minFilter = getVK( info.min_filter );
			sampler.mipmapMode = info.use_mipmap ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
			sampler.addressModeU = getVK( info.u_regime );
			sampler.addressModeV = getVK( info.v_regime );
			sampler.addressModeW = getVK( info.w_regime );
			sampler.compareOp = VK_COMPARE_OP_NEVER;
			sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			Unique< VkSampler > out;
			out.create( getHandle() , sampler );
			return out;
		}
		VkQueue getGraphicsQueue( uint index = 0 ) const
		{
			VkQueue queue;
			vkGetDeviceQueue( *handle , pdev.getGraphicsQueueFamily() , index , &queue );
			return queue;
		}
		uint getGraphicsQueueCount() const
		{
			return graphics_queue_count;
		}
		uint getGraphicsQueueFamily() const
		{
			return pdev.getGraphicsQueueFamily();
		}

		VkDevice getHandle() const
		{
			return *handle;
		}
	};
}