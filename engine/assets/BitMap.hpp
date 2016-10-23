#pragma once
#include <engine/assets/FileManager.hpp>
#include <engine/graphics/Graphics.hpp>
#include <engine/os/Files.hpp>
static Graphics::BitMap parseTGA( OS::Files::ImmutableFileView const &file )
{
	using namespace Graphics;
	Graphics::BitMap out;
	file.setPosition( 12 );
	out.width = file.getInc< uint16_t >();
	out.height = file.getInc< uint16_t >();
	out.mipmaps_count = 1;
	byte bpp = file.getInc< byte >();
	out.data = ( uint8_t* )file.getRaw() + 18;
	out.size = file.getLimit() - 18;
	if( bpp == 8 )
	{
		out.component_mapping = { ComponentFormat::R , ComponentType::UNORM8 };
	} else if( bpp == 16 )
	{
		out.component_mapping = { ComponentFormat::BGR , ComponentType::FIVE };
	} else
	{
		out.component_mapping = bpp == 32 ? ComponentMapping{ ComponentFormat::BGRA , ComponentType::UNORM8 }
		: ComponentMapping{ ComponentFormat::BGR , ComponentType::UNORM8 };
	}
	return out;
}