#pragma once
#include <engine/data_struct/Optional.hpp>
#include <engine/assets/BitMap.hpp>
#include <engine/graphics/ogl/oglinclude.hpp>
#include <engine/util/defines.hpp>
namespace GL
{
	using namespace Options;
	
	
	struct TextureView
	{
		uint tex = 0;
		uint target = 0;
		void bind( int slot , int uniform_id )
		{
			glActiveTexture( GL_TEXTURE0 + slot );
			glBindTexture( target , tex );
			glUniform1i( uniform_id , slot );
		}
		void release()
		{
			if( tex )
			{
				glDeleteTextures( 1 , &tex );
				tex = 0;
			}
		}
	};
	struct Texture2D
	{
		static TextureView create( TextureDesc const *desc )
		{
			uint tex;
			uint internal_format = -1 , format = -1 , type;
			int pack = 4;
			switch( desc->bitmap.pixel_type )
			{
			case PixelType::BYTE:
				type = GL_UNSIGNED_BYTE;
				break;
			case PixelType::INT:
				type = GL_INT;
				break;
			case PixelType::FLOAT:
				type = GL_FLOAT;
				break;
			case PixelType::FIVE:
				type = GL_UNSIGNED_SHORT_1_5_5_5_REV;
				break;
			}
			switch( desc->bitmap.pixel_mapping )
			{
			case PixelMapping::BGRA:
			case PixelMapping::RGBA:
				switch( desc->bitmap.pixel_type )
				{
				case PixelType::BYTE:
					internal_format = GL_RGBA8;
					break;
				case PixelType::INT:
					internal_format = GL_RGBA32I;
					break;
				case PixelType::FLOAT:
					internal_format = GL_RGBA32F;
					break;
				case PixelType::FIVE:
					internal_format = GL_RGB5;
					break;
				}
				break;
			case PixelMapping::BGR:
			case PixelMapping::RGB:
				switch( desc->bitmap.pixel_type )
				{
				case PixelType::BYTE:
					internal_format = GL_RGB8;
					break;
				case PixelType::INT:
					internal_format = GL_RGB32I;
					break;
				case PixelType::FLOAT:
					internal_format = GL_RGB32F;
					break;
				}
				break;
			case PixelMapping::R:
				switch( desc->bitmap.pixel_type )
				{
				case PixelType::BYTE:
					internal_format = GL_LUMINANCE8;
					break;
				case PixelType::INT:
					internal_format = GL_LUMINANCE32I_EXT;
					break;
				case PixelType::FLOAT:
					internal_format = GL_LUMINANCE32F_ARB;
					break;
				}
				break;
			}
			switch( desc->bitmap.pixel_mapping )
			{
			case PixelMapping::BGRA:
				format = GL_BGRA;
				break;
			case PixelMapping::RGBA:
				format = GL_RGBA;
				break;
			case PixelMapping::BGR:
				if( desc->bitmap.width * 3 % 4 != 0 )
				{
					pack = 1;
				}
				format = GL_BGR;
				break;
			case PixelMapping::RGB:
				if( desc->bitmap.width * 3 % 4 != 0 )
				{
					pack = 1;
				}
				format = GL_RGB;
				break;
			case PixelMapping::R:
				if( desc->bitmap.width % 4 != 0 )
				{
					pack = 1;
				}
				format = GL_LUMINANCE;
				break;
			}
			glPixelStorei( GL_UNPACK_ALIGNMENT , pack );
			glGenTextures( 1 , &tex );
			glBindTexture( GL_TEXTURE_2D , tex );
			glTexImage2D( GL_TEXTURE_2D , 0 , internal_format , desc->bitmap.width , desc->bitmap.height , 0 , format , type , desc->bitmap.data );
			if( desc->min_filter == MINFilter::NONE )
			{
				glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_BASE_LEVEL , 0 );
				glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MAX_LEVEL , 0 );
				if( desc->min_filter == MINFilter::MIPMAP_LINEAR )
				{
					glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR );
				} else
				{
					glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_NEAREST );
				}
			} else
			{
				glGenerateMipmap( GL_TEXTURE_2D );
				if( desc->mag_filter == MAGFilter::LINEAR )
				{
					if( desc->min_filter == MINFilter::MIPMAP_LINEAR )
					{
						glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR_MIPMAP_LINEAR );
					} else
					{
						glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR_MIPMAP_NEAREST );
					}
					
				} else
				{
					if( desc->min_filter == MINFilter::MIPMAP_LINEAR )
					{
						glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_NEAREST_MIPMAP_LINEAR );
					} else
					{
						glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_NEAREST_MIPMAP_NEAREST );
					}
				}
			}
			if( desc->mag_filter == MAGFilter::LINEAR )
			{
				glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR );
			} else
			{
				glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_NEAREST );
			}
			switch( desc->x_regime )
			{
			case WrapRegime::CLAMP:
				glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_CLAMP_TO_EDGE );
				break;
			case WrapRegime::MIRROR:
				glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_MIRRORED_REPEAT );
				break;
			case WrapRegime::REPEAT:
				glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_REPEAT );
				break;
			}
			switch( desc->y_regime )
			{
			case WrapRegime::CLAMP:
				glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T , GL_CLAMP_TO_EDGE );
				break;
			case WrapRegime::MIRROR:
				glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T , GL_MIRRORED_REPEAT );
				break;
			case WrapRegime::REPEAT:
				glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T , GL_REPEAT );
				break;
			}
			//checkError();
			TextureView out;
			out.target = GL_TEXTURE_2D;
			out.tex = tex;
			return out;
		}
	};
}