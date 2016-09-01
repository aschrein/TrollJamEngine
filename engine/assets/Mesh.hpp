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
		struct VertexGather
		{
			uint pos_id;
			uint normal_id;
			uint uv_ids[ 4 ];
		};
		struct Mesh3D
		{
			Array< float3 > positions;
			Array< float3 > normals;
			Array< float2 > tex_coords[ 4 ];
			Array< VertexGather > faces;
		};
		static Mesh3D parseSimpleMesh3d( char const *file_image , Allocator *allocator = Allocator::singleton )
		{

			StringStream ss = { file_image , allocator , 0 };
			String mesh_name = ss.getString();
			String area_name = ss.getString();
			Array< float3 > normals;
			normals.setAllocator( allocator );
			Array< float3 > positions;
			positions.setAllocator( allocator );
			Array< float2 > tex_coords;
			tex_coords.setAllocator( allocator );
			Array< uint3 > vertex_faces;
			vertex_faces.setAllocator( allocator );
			Array< uint3 > texture_faces;
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
						vertex_faces.push( ss.getiVec3() );
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
			Array< VertexGather > faces;
			return{ std::move( positions ) , std::move( normals ) , { std::move( tex_coords ) } , std::move( faces ) };
		}
	}
}