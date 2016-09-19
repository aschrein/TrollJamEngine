#pragma once
#include <engine/math/vec.hpp>
#include <engine/mem/Allocators.hpp>
#include <engine/data_struct/String.hpp>
#include <engine/data_struct/StringStream.hpp>
#include <engine/components/Reflection.hpp>
#include <engine/graphics/Material.hpp>
struct TestVertex
{
	float3 pos;
	float2 uv;
	bool operator==( TestVertex const &vertex ) const
	{
		return pos == vertex.pos && uv == vertex.uv;
	}
	bool operator<( TestVertex const &vertex ) const
	{
		return pos.mod2() < vertex.pos.mod2() && uv.mod2() < vertex.uv.mod2();
	}
};
namespace Collections
{
	template<>
	struct Hash< TestVertex >
	{
		static uint hashFunc( TestVertex const &val )
		{
			return uint( val.pos.x + val.pos.y * 0xff + val.pos.z * 0xffff ) + uint( val.uv.x + val.uv.y * 0xffff ) * 0xffff;
		}
	};
}
namespace Assets
{
	namespace Mesh
	{
		using namespace Math;
		using namespace Allocators;
		using namespace Collections;
		struct Mesh3D
		{
			uint stride;
			LocalArray< Graphics::AttributeInfo , 10 > attrib_info;
			Array< uint > indices;
			Array< byte > vertex_data;
		};
		static Pair< byte * , uint > serialize( Mesh3D const &mesh , Allocator *allcoator = Allocator::singleton )
		{
			int N = mesh.vertex_data.getSize();
			int M = mesh.indices.getSize() * sizeof( uint );
			uint file_size = sizeof( uint ) * 3 + sizeof( mesh.attrib_info ) + N + M;
			byte *out = allcoator->alloc< byte >( file_size );
			byte *origin = out;
			memcpy( out , &mesh.stride , sizeof( uint ) );
			out += sizeof( uint );
			memcpy( out , &N , sizeof( uint ) );
			out += sizeof( uint );
			memcpy( out , &M , sizeof( uint ) );
			out += sizeof( uint );
			memcpy( out , &mesh.attrib_info , sizeof( mesh.attrib_info ) );
			out += sizeof( mesh.attrib_info );
			memcpy( out , &mesh.vertex_data[ 0 ] , N );
			out += N;
			memcpy( out , &mesh.indices[ 0 ] , M );
			return{ origin , file_size };
		}
		static Mesh3D deserialize( byte const *blob , Allocator *allcoator = Allocator::singleton )
		{
			Mesh3D out;
			memcpy( &out.stride , blob , sizeof( uint ) );
			blob += sizeof( uint );
			int N , M;
			memcpy( &N , blob , sizeof( uint ) );
			blob += sizeof( uint );
			memcpy( &M , blob , sizeof( uint ) );
			blob += sizeof( uint );
			memcpy( &out.attrib_info , blob , sizeof( out.attrib_info ) );
			blob += sizeof( out.attrib_info );
			Array< byte > vertex_data;
			vertex_data.setAllocator( allcoator );
			vertex_data.make_space( N );
			vertex_data.resize( N );
			Array< uint > indices;
			indices.setAllocator( allcoator );
			indices.make_space( M / sizeof( uint ) );
			indices.resize( M / sizeof( uint ) );
			memcpy( &vertex_data[ 0 ] , blob , N );
			blob += N;
			memcpy( &indices[ 0 ] , blob , M );
			out.vertex_data = std::move( vertex_data );
			out.indices = std::move( indices );
			return out;
		}
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
			Array< uint > vertex_faces;
			vertex_faces.setAllocator( allocator );
			Array< uint > texture_faces;
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
						auto face = ss.getiVec3();
						vertex_faces.push( face.x );
						vertex_faces.push( face.y );
						vertex_faces.push( face.z );
					}
				} else if( area_name == "TEXTURE_FACE" )
				{
					while( ( area_name = ss.getString( ' ' , '\n' , '\r' ) ) == "f" )
					{
						auto face = ss.getiVec3();
						texture_faces.push( face.x );
						texture_faces.push( face.y );
						texture_faces.push( face.z );
					}
				} else
				{
					break;
				}
			}
			struct Vertex
			{
				float3 pos;
				uint pad;
				float3 norm;
				uint pad1;
				float2 uv;
				uint2 pad2;
				float3 tang;
				uint pad3;
				float3 binorm;
				uint pad4;
			};
			uint N = positions.getSize();
			HashMap< TestVertex , uint > gather_map;
			gather_map.setAllocator( allocator );
			uint M = vertex_faces.getSize();
			Array< Vertex > vertex_gather;
			vertex_gather.setAllocator( allocator );
			vertex_gather.setIncrement( 100 );
			Array< uint > gathered_indices;
			gathered_indices.setAllocator( allocator );
			gathered_indices.setIncrement( 100 );
			ito( M )
			{
				auto wpos_index = vertex_faces[ i ];
				auto uvpos_index = texture_faces[ i ];
				TestVertex test_vertex{
					positions[ wpos_index ] ,
					tex_coords[ uvpos_index ]
				};
				auto index_res = gather_map.get( test_vertex );
				if( index_res.isPresent() )
				{
					auto index = index_res.getValue();
					gathered_indices.push( index );
				} else
				{
					Vertex vertex;
					vertex.pos = positions[ wpos_index ];
					vertex.norm = positions[ wpos_index ];
					vertex.uv = tex_coords[ uvpos_index ];
					uint new_index = vertex_gather.getSize();
					vertex_gather.push( vertex );
					gathered_indices.push( new_index );
					gather_map.push( test_vertex , new_index );
				}
			}
			ito( gathered_indices.getSize() / 3 )
			{
				int i0 = gathered_indices[ i * 3 ];
				int i1 = gathered_indices[ i * 3 + 1 ];
				int i2 = gathered_indices[ i * 3 + 2 ];
				auto &v0 = vertex_gather[ i0 ];
				auto &v1 = vertex_gather[ i1 ];
				auto &v2 = vertex_gather[ i2 ];
				float3 p1 = v1.pos - v0.pos;
				float3 p2 = v2.pos - v0.pos;
				float3 norm = ( p1 ^ p2 ).norm();
				float2 t1 = v1.uv - v0.uv;
				float2 t2 = v2.uv - v0.uv;
				f2x2 uv_mat(
					t2.y , -t2.x ,
					-t1.x , t1.y
				);
				float2 a = MatUtil::inv( uv_mat ) * float2( 1.0f , 0.0f );
				float3 tang = ( p1 * a.x + p2 * a.y ).norm();
				a = MatUtil::inv( uv_mat ) * float2( 0.0f , 1.0f );
				float3 binorm = ( p1 * a.x + p2 * a.y ).norm();
				v0.tang = tang;
				v1.tang = tang;
				v2.tang = tang;
				v0.binorm = binorm;
				v1.binorm = binorm;
				v2.binorm = binorm;
				if( MathUtil< float >::abs( binorm * norm ) > 1.0e-4f )
				{
					OS::IO::debugLogln( "error while calculating tangent space" );
				}
			}
			Array< byte > vertex_data;
			vertex_data.setAllocator( allocator );
			vertex_data.make_space( vertex_gather.getSize() * sizeof( Vertex ) );
			vertex_data.resize( vertex_gather.getSize() * sizeof( Vertex ) );
			Vertex *vertex_ptr = ( Vertex* )&vertex_data[ 0 ];
			ito( vertex_gather.getSize() )
			{
				vertex_ptr[ i ] = vertex_gather[ i ];
			}
			//Allocator::copy( ( float3* )&vertex_data[ 0 ] , &positions[ 0 ] , positions.getSize() );
			return
			{
				sizeof( Vertex ) ,
				{ 5 ,
				{
					{ offsetOf( Vertex , pos ) , Graphics::AttributeSlot::POSITION , { Graphics::ComponentFormat::RGB , Graphics::ComponentType::FLOAT32 } } ,
					{ offsetOf( Vertex , norm ) , Graphics::AttributeSlot::NORMAL ,{ Graphics::ComponentFormat::RGB , Graphics::ComponentType::FLOAT32 } } ,
					{ offsetOf( Vertex , uv ) , Graphics::AttributeSlot::TEXCOORD0 ,{ Graphics::ComponentFormat::RG , Graphics::ComponentType::FLOAT32 } } ,
					{ offsetOf( Vertex , tang ) , Graphics::AttributeSlot::TANGENT ,{ Graphics::ComponentFormat::RGB , Graphics::ComponentType::FLOAT32 } } ,
					{ offsetOf( Vertex , binorm ) , Graphics::AttributeSlot::BINORMAL ,{ Graphics::ComponentFormat::RGB , Graphics::ComponentType::FLOAT32 } } ,
				}
				} ,
				std::move( gathered_indices ) ,
				std::move( vertex_data )
			};
		}
	}
}