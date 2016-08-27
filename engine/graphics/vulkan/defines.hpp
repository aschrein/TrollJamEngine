#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <engine/mem/Allocators.hpp>
#define VKASSERTLOG( token ){ auto res = token; if( res != VK_SUCCESS ){ OS::IO::debugLogln( #token , " has failed" ); } }
#define VK_OBJECT( classname ) \
private:\
classname()=default;\
public:\
classname( classname  const & ) = delete;\
classname &operator=( classname  const & ) = delete;\
classname( classname && ) = default;\
classname &operator=( classname && ) = default;
namespace VK
{
	using namespace Allocators;
	using namespace Collections;

	enum class ImageAccessMask : uint
	{
		COLOR_ATTACHMENT , READ , WRITE
	};
	enum class ImageLayout : uint
	{
		COLOR_ATTACHMENT , PRESENT , UNDEFINED
	};
	enum class LoadOp : uint
	{
		CLEAR , LOAD , DONT_CARE
	};
	enum class StoreOp : uint
	{
		STORE , DONT_CARE
	};
	enum class ImageUse
	{
		TARGET , SRC
	};
	inline VkAttachmentStoreOp getVK( StoreOp arg )
	{
		static VkAttachmentStoreOp vk_[] =
		{
			VK_ATTACHMENT_STORE_OP_STORE ,
			VK_ATTACHMENT_STORE_OP_DONT_CARE
		};
		return vk_[ uint( arg ) ];
	}
	inline VkAttachmentLoadOp getVK( LoadOp arg )
	{
		static VkAttachmentLoadOp vk_[] =
		{
			VK_ATTACHMENT_LOAD_OP_CLEAR ,
			VK_ATTACHMENT_LOAD_OP_LOAD ,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE
		};
		return vk_[ uint( arg ) ];
	}
	inline VkImageLayout getVK( ImageLayout layout )
	{
		static VkImageLayout vk_[] =
		{
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR ,
			VK_IMAGE_LAYOUT_UNDEFINED
		};
		return vk_[ uint( layout ) ];
	}
	inline VkAccessFlags getVK( ImageAccessMask mask )
	{
		static VkAccessFlags vk_[] =
		{
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT ,
			VK_ACCESS_MEMORY_READ_BIT ,
			VK_ACCESS_TRANSFER_WRITE_BIT
		};
		return vk_[ uint( mask ) ];
	}
	struct ImageState
	{
		ImageAccessMask access_mask;
		ImageLayout layout;
	};
	enum class MemoryType : uint
	{
		HOST , DEVICE
	};
	inline uint getVK( MemoryType type )
	{
		uint vk_[] =
		{
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT ,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		};
		return vk_[ uint( type ) ];
	}
	enum class BufferTarget : uint
	{
		VERTEX_BUFFER , INDEX_BUFFER
	};
	inline uint getVK( BufferTarget type )
	{
		uint vk_[] =
		{
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT ,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
		};
		return vk_[ uint( type ) ];
	}
}