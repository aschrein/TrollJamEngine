#pragma once
#include <engine/assets/FileManager.hpp>
enum class MAGFilter : int
{
	NEAREST , LINEAR
};
enum class MINFilter : int
{
	NONE , MIPMAP_LINEAR , MIPMAP_CLOSEST
};
enum class WrapRegime
{
	CLAMP , REPEAT , MIRROR
};
enum class PixelType : int
{
	FLOAT , BYTE , INT , FIVE
};
enum class PixelMapping : int
{
	RGB , RGBA , BGRA , BGR , R
};
struct BitMap2D
{
	void const *data;
	int width;
	int height;
	PixelMapping pixel_mapping;
	PixelType pixel_type;
	uint getBpp() const
	{
		uint component_size;
		switch( pixel_type )
		{
		case PixelType::BYTE:
			component_size = 1;
			break;
		case PixelType::FLOAT:
		case PixelType::INT:
			component_size = 4;
			break;
		case PixelType::FIVE:
			return 2;
			break;
		}
		switch( pixel_mapping )
		{
		case PixelMapping::BGR:
		case PixelMapping::RGB:
			return 3 * component_size;
		case PixelMapping::R:
			return component_size;
		case PixelMapping::BGRA:
		case PixelMapping::RGBA:
			return 4 * component_size;
		}
	}
};
struct TextureDesc
{
	BitMap2D bitmap;
	MAGFilter mag_filter;
	MINFilter min_filter;
	WrapRegime x_regime;
	WrapRegime y_regime;
};
static BitMap2D mapTGA( ImmutableFileView const &file )
{
	BitMap2D out;
	file.setPosition( 12 );
	out.width = file.getInc< uint16_t >();
	out.height = file.getInc< uint16_t >();
	byte bpp = file.getInc< byte >();
	out.data = ( uint8_t* )file.getRaw() + 18;
	if( bpp == 8 )
	{
		out.pixel_type = PixelType::BYTE;
		out.pixel_mapping = PixelMapping::R;
	} else if( bpp == 16 )
	{
		out.pixel_type = PixelType::FIVE;
		out.pixel_mapping = PixelMapping::BGRA;
	} else
	{
		out.pixel_mapping = bpp == 32 ? PixelMapping::BGRA : PixelMapping::BGR;
		out.pixel_type = PixelType::BYTE;
	}
	return out;
}