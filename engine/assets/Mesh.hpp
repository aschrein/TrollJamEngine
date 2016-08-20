#pragma once
#include <engine/math/vec.hpp>
#include <engine/mem/Allocators.hpp>
#include <engine/data_struct/String.hpp>
#include <engine/data_struct/StringStream.hpp>
#include <engine/components/Reflection.hpp>
namespace Assets
{
	namespace Mesh
	{
		using namespace Math;
		using namespace Allocators;
		using namespace Collections;
		struct SimpleMesh3D
		{
			Allocator *allocator;
			Array< f3 > positions;
			Array< f3 > normals;
			Array< i3 > faces;
		};
		static SimpleMesh3D parseSimpleMesh3d( char const *file_image , Allocator *allocator = Allocator::singleton )
		{

			StringStream ss = { file_image , allocator , 0 };
			String mesh_name = ss.getString();
			String area_name = ss.getString();
			Array< f3 > normals;
			normals.setAllocator( allocator );
			Array< f3 > positions;
			positions.setAllocator( allocator );
			Array< f2 > tex_coords;
			tex_coords.setAllocator( allocator );
			Array< i3 > faces;
			faces.setAllocator( allocator );
			Array< i3 > texture_faces;
			texture_faces.setAllocator( allocator );
			String texture_filename( allocator );
			while( area_name != "" )
			{
				if( area_name == "POSITION" )
				{
					while( ( area_name = ss.getString( ' ' , '\n' , '\r' ) ) == "v" )
					{
						positions.push( ss.getVec3() );
					}
					//OS::IO::debugLogln( positions.getSize() );
				} else if( area_name == "NORMAL" )
				{
					while( ( area_name = ss.getString( ' ' , '\n' , '\r' ) ) == "n" )
					{
						normals.push( ss.getVec3() );
					}
				} else if( area_name == "TEXCOORD" )
				{
					while( ( area_name = ss.getString( ' ' , '\n' , '\r' ) ) == "tx" )
					{
						tex_coords.push( ss.getVec2() );
					}
				} else if( area_name == "FACE" )
				{
					while( ( area_name = ss.getString( ' ' , '\n' , '\r' ) ) == "f" )
					{
						faces.push( ss.getiVec3() );
					}
				} else if( area_name == "TEXTURE_FACE" )
				{
					while( ( area_name = ss.getString( ' ' , '\n' , '\r' ) ) == "f" )
					{
						texture_faces.push( ss.getiVec3() );
					}
				} else
				{
					break;
				}
			}
			return{ allocator , std::move( positions ) , std::move( normals ) , std::move( faces ) };
		}
	}
}