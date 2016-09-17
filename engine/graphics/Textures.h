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
		RGB , RGBA , BGR , BGRA , R , RG
	};
	enum class ComponentSwizzle : uint
	{
		R , G , B , A , ONE , ZERO
	};
	//default is RGBA
	struct ComponentMapping
	{
		ComponentFormat format;
		ComponentType type;
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
			3 , 4 , 3 , 4 , 1 , 2
		};
		return _comp[ ( uint )mapping.format ] * _sizes[ ( uint )mapping.type ];
	}
	//strict linear layout and power of two size
	struct BitMap
	{
		void const *data;
		uint size;
		uint width;
		uint height;
		uint mipmaps_count;
		ImageCompression compression;
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
	struct SamplerCreateInfo
	{
		Filter mag_filter;
		Filter min_filter;
		bool use_mipmap;
		uint max_anisotropy_level;
		WrapRegime u_regime;
		WrapRegime v_regime;
		WrapRegime w_regime;
	};
	struct TextureCreateInfo
	{
		BitMap bitmap;
		SamplerCreateInfo sampler_info;
		Usage usage;
	};
	
}