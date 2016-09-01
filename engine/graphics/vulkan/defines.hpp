#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <engine/mem/Allocators.hpp>
//#include <engine/graphics/Graphics.hpp>
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
	enum class ComponentSwizzle : uint
	{
		R = VK_COMPONENT_SWIZZLE_R ,
		G = VK_COMPONENT_SWIZZLE_G ,
		B = VK_COMPONENT_SWIZZLE_B ,
		A = VK_COMPONENT_SWIZZLE_A ,
		ONE = VK_COMPONENT_SWIZZLE_ONE
	};
	struct ComponentMapping
	{
		ComponentSwizzle swizzle[ 4 ];
	};
	struct ComponentType
	{

	};
	/*enum class ImageAccessMask : uint
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
	enum class ImageUsage : uint
	{
		COLOR_TARGET , SOURCE , DEPTH_STENCIL_TARGET
	};
	inline VkImageUsageFlags getVK( ImageUsage arg )
	{
		static VkAttachmentStoreOp vk_[] =
		{
			VK_ATTACHMENT_STORE_OP_STORE ,
			VK_ATTACHMENT_STORE_OP_DONT_CARE
		};
		return vk_[ uint( arg ) ];
	}
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
		static uint vk_[] =
		{
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT ,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		};
		return vk_[ uint( type ) ];
	}
	enum class BufferTarget : uint
	{
		TRANSFER_SRC , VERTEX_BUFFER , INDEX_BUFFER , UNIFORM_BUFFER
	};
	inline uint getVK( BufferTarget type )
	{
		static uint vk_[] =
		{
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT ,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT ,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT ,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT ,
		};
		return vk_[ uint( type ) ];
	}
	enum class ImageType
	{
		COLOR , DEPTH_STENCIL
	};
	enum class Channel : uint
	{
		R , G , B , A , ONE
	};
	inline VkComponentSwizzle getVK( Channel c )
	{
		static VkComponentSwizzle _vk[] =
		{
			VK_COMPONENT_SWIZZLE_R , VK_COMPONENT_SWIZZLE_G , VK_COMPONENT_SWIZZLE_B , VK_COMPONENT_SWIZZLE_A , VK_COMPONENT_SWIZZLE_ONE
		};
		return _vk[ uint( c ) ];
	}*/
	
	struct ImageMapingInfo
	{
		VkFormat format;
		VkComponentMapping mapping;
	};
	/*inline ImageMapingInfo getVK( Graphics::PixelType pixel_type )
	{
#define R VK_COMPONENT_SWIZZLE_R
#define G VK_COMPONENT_SWIZZLE_G
#define B VK_COMPONENT_SWIZZLE_B
#define A VK_COMPONENT_SWIZZLE_A
#define ONE VK_COMPONENT_SWIZZLE_ONE
		static ImageMapingInfo _vk[] =
		{
			{ VK_FORMAT_R32G32B32_SFLOAT , { R , G , B , ONE } } ,
			{ VK_FORMAT_R32G32B32A32_SFLOAT ,{ R , G , B , A } } ,
			{ VK_FORMAT_R32G32B32A32_SINT ,{ R , G , B , A } } ,
			{ VK_FORMAT_R16G16B16A16_SINT ,{ R , G , B , A } } ,
			{ VK_FORMAT_R16G16B16A16_UNORM ,{ R , G , B , A } } ,
			{ VK_FORMAT_R5G5B5A1_UNORM_PACK16 ,{ R , G , B , A } } ,
			{ VK_FORMAT_R8G8B8A8_UNORM ,{ R , G , B , A } } ,
			{ VK_FORMAT_R32_SFLOAT ,{ R , R , R , R } } ,
			{ VK_FORMAT_R8_UNORM ,{ R , R , R , R } } ,
			{ VK_FORMAT_B8G8R8_UNORM ,{ R , G , B , ONE } } ,
			{ VK_FORMAT_B8G8R8A8_UNORM ,{ R , G , B , A } } ,
			{ VK_FORMAT_B5G5R5A1_UNORM_PACK16 ,{ R , G , B , A } } ,
		};
#undef R
#undef G
#undef B
#undef A
#undef ONE
		return _vk[ uint( pixel_type ) ];
	}*/
}