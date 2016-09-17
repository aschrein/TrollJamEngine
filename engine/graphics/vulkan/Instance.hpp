#pragma once
#include <engine/graphics/vulkan/defines.hpp>
#include <engine/util/defines.hpp>
#include <engine/graphics/vulkan/Unique.hpp>
namespace VK
{
	class Instance
	{
		VK_OBJECT( Instance );
	public:
		class PhysicalDevice
		{
			friend class Instance;
		private:
			VkPhysicalDevice handle;
			VkPhysicalDeviceProperties prop;
			VkPhysicalDeviceFeatures features;
			VkPhysicalDeviceMemoryProperties mem_prop;
			VkQueueFamilyProperties queue_properties[ 10 ] = { 0 };
			uint32_t queue_count;
			uint32_t graphics_queue;
			uint32_t compute_queue;
		public:
			uint32_t getGraphicsQueueFamily() const
			{
				return graphics_queue;
			}
			uint32_t getComputeQueueFamily() const
			{
				return compute_queue;
			}
			VkPhysicalDevice const &getHandle() const
			{
				return handle;
			}
			//TODO find closest match
			uint32_t getMemoryIndex( uint32_t type_bits , uint32_t properties ) const
			{
				for( uint32_t i = 0; i < 32; i++ )
				{
					if( ( type_bits & 1 ) == 1 )
					{
						if( ( mem_prop.memoryTypes[ i ].propertyFlags & properties ) == properties )
						{
							return i;
						}
					}
					type_bits >>= 1;
				}
				return 0;
			}
		};
	private:
		Unique< VkInstance > instance;
		PhysicalDevice pdev;
	public:
		static Instance create()
		{
			VkApplicationInfo vkappinfo;
			Allocator::zero( &vkappinfo );
			vkappinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			vkappinfo.pApplicationName = "vk_app";
			vkappinfo.pEngineName = "vk_engine";
			vkappinfo.apiVersion = VK_API_VERSION_1_0;
			VkInstanceCreateInfo vkinstinfo;
			Allocator::zero( &vkinstinfo );
			vkinstinfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			vkinstinfo.pApplicationInfo = &vkappinfo;
			char* extension_names[] =
			{
				VK_KHR_SURFACE_EXTENSION_NAME ,
				VK_KHR_WIN32_SURFACE_EXTENSION_NAME
			};
			vkinstinfo.enabledExtensionCount = 2;
			vkinstinfo.ppEnabledExtensionNames = extension_names;
			Instance out;
			out.instance.create( vkinstinfo , nullptr );
			uint32_t phys_dev_count = 0;
			VkPhysicalDevice all_phys_devices[ 100 ];
			Allocator::zero( all_phys_devices );
			VkDeviceQueueCreateInfo vkdevqueueinfo;
			PhysicalDevice pdev;
			
			vkEnumeratePhysicalDevices( *out.instance , &phys_dev_count , nullptr );
			VKASSERTLOG( vkEnumeratePhysicalDevices( *out.instance , &phys_dev_count , all_phys_devices ) );
			pdev.handle = all_phys_devices[ 0 ];
			vkGetPhysicalDeviceProperties( pdev.handle , &pdev.prop );
			vkGetPhysicalDeviceFeatures( pdev.handle , &pdev.features );
			vkGetPhysicalDeviceMemoryProperties( pdev.handle , &pdev.mem_prop );
			vkGetPhysicalDeviceQueueFamilyProperties( pdev.handle , &pdev.queue_count , nullptr );
			vkGetPhysicalDeviceQueueFamilyProperties( pdev.handle , &pdev.queue_count , pdev.queue_properties );
			ito( pdev.queue_count )
			{
				if( pdev.queue_properties[ i ].queueFlags & VK_QUEUE_GRAPHICS_BIT )
				{
					pdev.graphics_queue = i;
					break;
				}
			}
			ito( pdev.queue_count )
			{
				if( pdev.queue_properties[ i ].queueFlags & VK_QUEUE_COMPUTE_BIT )
				{
					pdev.compute_queue = i;
					break;
				}
			}
			out.pdev = pdev;
			return out;
		}
		PhysicalDevice const &getPhysicalDevice() const
		{
			return pdev;
		}
		VkInstance const &getHandle() const
		{
			return *instance;
		}
	};
}