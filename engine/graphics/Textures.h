#pragma once
#include <engine/util/defines.hpp>
#include <engine/math/vec.hpp>
#include <engine/graphics/Buffers.hpp>
namespace Graphics
{
	using namespace Math;
	enum class ComponentType : uint
	{
		FLOAT32 , INT , SHORT , BYTE , UNORM8 , UNORM16 , FIVE
	};
	enum class ComponentFormat
	{
		RGB , RGBA , BGR , BGRA , R
	};
	enum class ComponentSwizzle : uint
	{
		R , G , B , A , ONE , ZERO
	};
	//default is RGBA
	struct ComponentMapping
	{
		ComponentFormat format;
		ComponentType type = ComponentType::BYTE;
	};
	enum class ImageCompression
	{
		NONE
	};
	static uint getBpp( ComponentMapping mapping )
	{
		bool used[ 4 ] = { false };
		if( mapping.type == ComponentType::FIVE )
		{
			return 2;
		}
		static uint _sizes[] =
		{
			4 , 4 , 2 , 1 , 1 , 2
		};
		static uint _comp[] =
		{
			3 , 4 , 3 , 4 , 1
		};
		return _comp[ ( uint )mapping.format ] * _sizes[ ( uint )mapping.type ];
	}
	//strict linear layout and power of two size
	struct BitMap2D
	{
		void const *data;
		uint size;
		uint width;
		uint height;
		uint depth = 1;
		uint layers_count = 1;
		uint mipmaps_count = 1;
		ImageCompression compression = ImageCompression::NONE;
		ComponentMapping component_mapping;
		uint getBpp() const
		{
			return Graphics::getBpp( component_mapping );
		}
	};
	enum class Filter : int
	{
		NEAREST , LINEAR
	};
	enum class WrapRegime
	{
		CLAMP , REPEAT , MIRROR
	};
	struct SamplerInfo
	{
		Filter mag_filter;
		Filter min_filter;
		bool use_mipmap;
		uint anisotropy_level;
		WrapRegime u_regime;
		WrapRegime v_regime;
		WrapRegime w_regime;
	};
	struct RenderTargetInfo
	{
		ComponentMapping component_mapping;
		uint2 size;
	};
	struct DepthStencilTargetInfo
	{
		uint2 size;
	};
	struct TextureInfo
	{
		BitMap2D bitmap;
		Usage usage;
	};
	enum class TextureSlot
	{
		ALBEDO , NORMAL , METALNESS , ROUGHNESS
	};
	struct TextureRef
	{
		uint texture_view;
		uint sampler;
		TextureSlot target;
	};
	struct TextureViewInfo
	{
		uint texture_handler;
		ComponentSwizzle swizzle[ 4 ];
	};
	enum class RenderTargetType
	{
		COLOR , DEPTH , DEPTH_STENCIL
	};
	enum class DepthFormat
	{
		DEPTH32_UINT
	};
}