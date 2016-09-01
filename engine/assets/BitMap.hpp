#pragma once
#include <engine/assets/FileManager.hpp>
#include <engine/graphics/Graphics.hpp>
#include <engine/os/Files.hpp>
static Graphics::BitMap2D mapTGA( OS::Files::ImmutableFileView const &file )
{
	Graphics::BitMap2D out;
	file.setPosition( 12 );
	out.width = file.getInc< uint16_t >();
	out.height = file.getInc< uint16_t >();
	byte bpp = file.getInc< byte >();
	out.data = ( uint8_t* )file.getRaw() + 18;
	/*if( bpp == 8 )
	{
		out.
	} else if( bpp == 16 )
	{
		out.pixel_type = PixelType::B5G5R5A1_UNORM;
	} else
	{
		out.pixel_type = bpp == 32 ? PixelType::BGRA_8_UNORM : PixelType::BGR_8_UNORM;
	}*/
	return out;
}