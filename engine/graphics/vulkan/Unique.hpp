#pragma once
#include <engine/graphics/vulkan/defines.hpp>
namespace VK
{
	template< typename T >
	struct UniqueBase
	{
	};
	template<>
	struct UniqueBase< VkInstance >
	{
		VkInstance value = VK_NULL_HANDLE;
		VkAllocationCallbacks *alloc_callbacks = nullptr;
		void init( VkInstance value , VkAllocationCallbacks *alloc_callbacks = nullptr )
		{
			this->value = value;
			this->alloc_callbacks = alloc_callbacks;
		}
		void create( VkInstanceCreateInfo const &create_info , VkAllocationCallbacks *alloc_callbacks = nullptr )
		{
			VKASSERTLOG( vkCreateInstance( &create_info , alloc_callbacks , &value ) );
			this->alloc_callbacks = alloc_callbacks;
		}
		void release()
		{
			if( value != VK_NULL_HANDLE )
			{
				vkDestroyInstance( value , nullptr );
			}
			alloc_callbacks = nullptr;
			value = VK_NULL_HANDLE;
		}
	};
	template<>
	struct UniqueBase< VkDevice >
	{
		VkDevice value = VK_NULL_HANDLE;
		VkAllocationCallbacks *alloc_callbacks = nullptr;
		void init( VkDevice value , VkAllocationCallbacks *alloc_callbacks = nullptr )
		{
			this->value = value;
			this->alloc_callbacks = alloc_callbacks;
		}
		void create( VkPhysicalDevice dev , VkDeviceCreateInfo const &create_info , VkAllocationCallbacks *alloc_callbacks = nullptr )
		{
			VKASSERTLOG( vkCreateDevice( dev , &create_info , nullptr , &value ) );
			this->alloc_callbacks = alloc_callbacks;
		}
		void release()
		{
			if( value != VK_NULL_HANDLE )
			{
				vkDestroyDevice( value , alloc_callbacks );
			}
			alloc_callbacks = nullptr;
			value = VK_NULL_HANDLE;
		}
	};
	template<>
	struct UniqueBase< VkSurfaceKHR >
	{
		VkInstance instance = VK_NULL_HANDLE;
		VkSurfaceKHR value = VK_NULL_HANDLE;
		VkAllocationCallbacks *alloc_callbacks = nullptr;
		void init( VkInstance instance , VkSurfaceKHR value , VkAllocationCallbacks *alloc_callbacks = nullptr )
		{
			this->instance = instance;
			this->value = value;
			this->alloc_callbacks = alloc_callbacks;
		}
		void create( VkInstance instance , VkWin32SurfaceCreateInfoKHR const &create_info , VkAllocationCallbacks *alloc_callbacks = nullptr )
		{
			VKASSERTLOG( vkCreateWin32SurfaceKHR( instance , &create_info , alloc_callbacks , &value ) );
			this->instance = instance;
			this->alloc_callbacks = alloc_callbacks;
		}
		void release()
		{
			if( value != VK_NULL_HANDLE )
			{
				vkDestroySurfaceKHR( instance , value , alloc_callbacks );
			}
			alloc_callbacks = nullptr;
			value = VK_NULL_HANDLE;
		}
	};
	template<>
	struct UniqueBase< VkCommandBuffer >
	{
		VkDevice dev = VK_NULL_HANDLE;
		VkCommandPool command_pool = VK_NULL_HANDLE;
		VkCommandBuffer value = VK_NULL_HANDLE;
		void init( VkDevice dev , VkCommandPool command_pool , VkCommandBuffer value )
		{
			this->dev = dev;
			this->value = value;
			this->command_pool = command_pool;
		}
		void create( VkDevice dev , VkCommandPool command_pool , VkCommandBufferAllocateInfo const &create_info )
		{
			VKASSERTLOG( vkAllocateCommandBuffers( dev , &create_info , &value ) );
			this->dev = dev;
			this->command_pool = command_pool;
		}
		void release()
		{
			if( value != VK_NULL_HANDLE )
			{
				vkFreeCommandBuffers( dev , command_pool , 1 , &value );
			}
			value = VK_NULL_HANDLE;
			dev = VK_NULL_HANDLE;
			command_pool = VK_NULL_HANDLE;
		}
	};
	/*template< typename T , typename C >
	struct DevChildCicleFunc
	{
		typedef std::function< VkResult( VkDevice , T , VkAllocationCallbacks* ) > DevChildDestroyFunc;
		typedef std::function< VkResult( VkDevice , C* , VkAllocationCallbacks* , T * ) > DevChildCreateFunc;
	};
	template< typename T >
	struct DevChildCicle
	{
	};
	template<>
	struct DevChildCicle< VkSwapchainKHR >
	{
		typedef VkSwapchainCreateInfoKHR CreateDesc;
		static DevChildCicleFunc< VkSwapchainKHR , CreateDesc >::DevChildDestroyFunc destructor()
		{
			return []( VkDevice dev , VkSwapchainKHR value , VkAllocationCallbacks* allocator ) { return vkDestroySwapchainKHR( dev , value , allocator ); };
		}
		static void* constructor()
		{
			return vkCreateSwapchainKHR;
		}
	};
	template<>
	struct DevChildCicle< VkBuffer >
	{
		static void* destructor()
		{
			return vkDestroyBuffer;
		}
		static void* constructor()
		{
			return vkCreateBuffer;
		}
	};
	template<>
	struct DevChildCicle< VkImage >
	{
		static void* destructor()
		{
			return vkDestroyImage;
		}
		static void* constructor()
		{
			return vkCreateImage;
		}
	};
	template<>
	struct DevChildCicle< VkImageView >
	{
		static void* destructor()
		{
			return vkDestroyImageView;
		}
		static void* constructor()
		{
			return vkCreateImageView;
		}
	};
	template<>
	struct DevChildCicle< VkShaderModule >
	{
		static void* destructor()
		{
			return vkDestroyShaderModule;
		}
		static void* constructor()
		{
			return vkCreateShaderModule;
		}
	};
	template<>
	struct DevChildCicle< VkRenderPass >
	{
		static void* destructor()
		{
			return vkDestroyRenderPass;
		}
		static void* constructor()
		{
			return vkCreateRenderPass;
		}
	};*/
	template< typename T , typename C , VkResult( _stdcall *Constructor )( VkDevice , C const * , VkAllocationCallbacks const* , T * ) , void( _stdcall *Destructor )( VkDevice , T , VkAllocationCallbacks const* ) >
	struct DevChild
	{
		VkDevice dev = VK_NULL_HANDLE;
		T value = VK_NULL_HANDLE;
		VkAllocationCallbacks *alloc_callbacks = nullptr;
		void init( VkDevice dev , T value , VkAllocationCallbacks *alloc_callbacks = nullptr )
		{
			this->dev = dev;
			this->value = value;
			this->alloc_callbacks = alloc_callbacks;
		}
		void create( VkDevice dev , C const &create_info , VkAllocationCallbacks *alloc_callbacks = nullptr )
		{
			VKASSERTLOG( Constructor( dev , &create_info , alloc_callbacks , &value ) );
			this->dev = dev;
			this->alloc_callbacks = alloc_callbacks;
		}
		void release()
		{
			if( value != VK_NULL_HANDLE )
			{
				Destructor( dev , value , alloc_callbacks );
			}
			alloc_callbacks = nullptr;
			value = VK_NULL_HANDLE;
			dev = VK_NULL_HANDLE;
		}
	};
	template<> struct UniqueBase< VkSwapchainKHR > : public DevChild< VkSwapchainKHR , VkSwapchainCreateInfoKHR , vkCreateSwapchainKHR , vkDestroySwapchainKHR >{};
	template<> struct UniqueBase< VkCommandPool > : public DevChild< VkCommandPool , VkCommandPoolCreateInfo , vkCreateCommandPool , vkDestroyCommandPool >{};
	template<> struct UniqueBase< VkBuffer > : public DevChild< VkBuffer , VkBufferCreateInfo , vkCreateBuffer , vkDestroyBuffer >{};
	template<> struct UniqueBase< VkSemaphore > : public DevChild< VkSemaphore , VkSemaphoreCreateInfo , vkCreateSemaphore , vkDestroySemaphore >{};
	template<> struct UniqueBase< VkImage > : public DevChild< VkImage , VkImageCreateInfo , vkCreateImage , vkDestroyImage >{};
	template<> struct UniqueBase< VkImageView > : public DevChild< VkImageView , VkImageViewCreateInfo , vkCreateImageView , vkDestroyImageView >{};
	template<> struct UniqueBase< VkShaderModule > : public DevChild< VkShaderModule , VkShaderModuleCreateInfo , vkCreateShaderModule , vkDestroyShaderModule > {};
	template<> struct UniqueBase< VkRenderPass > : public DevChild< VkRenderPass , VkRenderPassCreateInfo , vkCreateRenderPass , vkDestroyRenderPass > {};
	template<> struct UniqueBase< VkFramebuffer > : public DevChild< VkFramebuffer , VkFramebufferCreateInfo , vkCreateFramebuffer , vkDestroyFramebuffer > {};
	template<> struct UniqueBase< VkDeviceMemory > : public DevChild< VkDeviceMemory , VkMemoryAllocateInfo , vkAllocateMemory , vkFreeMemory > {};
	template<>
	struct UniqueBase< VkPipeline >{};
	template< typename T >
	class Unique
	{
	private:
		UniqueBase< T > base;
	public:
		template< typename ...K >
		Unique( K ...values )
		{
			base.init( values... );
		}
		template< typename ...K >
		void create( K ...values )
		{
			base.release();
			base.create( values... );
		}
		template< typename ...K >
		void init( K ...values )
		{
			base.release();
			base.init( values... );
		}
		Unique() = default;
		Unique( Unique  const & ) = delete;
		Unique &operator=( Unique  const & ) = delete;
		Unique( Unique &&p )
		{
			*this = std::move( p );
		}
		Unique &operator=( Unique &&p )
		{
			base.release();
			base = p.base;
			p.base.value = VK_NULL_HANDLE;
			p.base.release();
			return *this;
		}
		T &operator*()
		{
			return base.value;
		}
		T const *operator&() const
		{
			return base.value;
		}

		T const &operator*() const
		{
			return base.value;
		}
		~Unique()
		{
			base.release();
		}
	};
}