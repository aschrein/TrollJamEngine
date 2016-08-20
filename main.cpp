#include <stdafx.h>
#include <engine/os/Window.hpp>
#include <engine/os/log.hpp>
#include <engine/assets/FileManager.hpp>
#include <engine/assets/Mesh.hpp>
#include <engine/assets/Font.hpp>
#include <engine/graphics/Camera.hpp>
#include <engine/graphics/Renderer.hpp>

#include <Init.hpp>
using namespace Math;
using namespace Graphics;

Material default_material;
uint texture_handle = 0;
FileManager *FileManager::singleton;
DrawMeshDesc monkey_head_mesh;
DrawMeshDesc plane_mesh;
int main( int argc , char ** argv )
{
	using namespace OS;
	Unique< FileConsumer > local_file_consumer( new FileConsumer() );
	FileManager::singleton = FileManager::create();
	
	float camera_angle_phi = 0.0f;
	float camera_angle_thetha = 0.0f;
	float camera_distance = 1.0f;
	f3 camera_sight_point = { 0.0f , 0.0f , 0.0f };
	i2 ms;
	Renderer *renderer;
	Signal init_signal;
	
	RingBuffer< Pair< OS::InputState::State , OS::InputState::EventType > , 100 > input_events;
	OS::Window window = OS::Window( { 100 , 100 , 512 , 512 ,
		[ & ]()
	{
		renderer = window.createRenderer();
		init_signal.signal();
	} ,
		[]( int x , int y , int w , int h )
	{
		OS::IO::debugLogln( "resize x:" , x , ",y:" , y , ",width:" , w , ",height:" , h );
	} ,
		[ & ]()
	{
	} ,
		[ & ]( OS::InputState::EventType type , OS::InputState::State const *state )
	{
		input_events.push( {*state , type} );
	}
	} );
	Unique< Thread > update_thread = Thread::create(

		[ & ]()
	{
		OS::InputState::State old_state;
		Allocator::zero( &old_state , 1 );
		init_signal.wait();
		auto allocator = renderer->getAuxiliaryAllocator();
		FileManager::singleton->loadFile( { "Suzanne.txt" , "calibri_16.fnt" , "calibri_16.tga" , "wood_texture.tga" } , local_file_consumer.get() , allocator );
		int files = 4;
		SimpleMesh3D mesh;
		BMFontInfo font_bit_map_info;
		Shared< FileImage > font_bitmap;
		Shared< FileImage > head_bitmap;
		BitMap2D tga_map;
		BitMap2D head_map;
		void *last_consumed = nullptr;
		while( files )
		{
			FileEvent file_event = local_file_consumer->popEvent( true ).getValue();
			if( file_event.filename == "Suzanne.txt" )
			{
				auto file = std::move( file_event.file_result.getValue() );
				mesh = parseSimpleMesh3d( ( char const * )file->getView().getRaw() , allocator );
				files--;
			} else if( file_event.filename == "calibri_16.tga" )
			{
				font_bitmap = std::move( file_event.file_result.getValue() );
				tga_map = mapTGA( font_bitmap->getView() );
				files--;
			} else if( file_event.filename == "wood_texture.tga" )
			{
				head_bitmap = std::move( file_event.file_result.getValue() );
				head_map = mapTGA( head_bitmap->getView() );
				files--;
			} else if( file_event.filename == "calibri_16.fnt" )
			{
				auto file = std::move( file_event.file_result.getValue() );
				font_bit_map_info = parseBMFont( ( char const * )file->getView().getRaw() );
				files--;
			}
		}
		{
			{
				AttributeBufferDesc *attribute_buffer_desc = allocator->alloc< AttributeBufferDesc >();
				attribute_buffer_desc->data = mesh.positions.getPtr();
				attribute_buffer_desc->size = mesh.positions.getSize() * sizeof( f3 );
				attribute_buffer_desc->usage = BufferUsage::STATIC_DRAW;
				Attribute *attributes = allocator->alloc< Attribute >( 1 );
				attributes->elem_count = 3;
				attributes->normalized = false;
				attributes->offset = 0;
				attributes->stride = sizeof( f3 );
				attributes->src_type = PlainFieldType::FLOAT32;
				attributes->slot = AttributeSlot::POSITION;
				attribute_buffer_desc->attributes = attributes;
				attribute_buffer_desc->attributes_count = 1;
				monkey_head_mesh.vertex_buffer_handle = allocator->alloc< uint >( 2 );
				monkey_head_mesh.vertex_buffer_handle[ 0 ] = renderer->createVertexBuffer( attribute_buffer_desc );
			}
			{
				AttributeBufferDesc *attribute_buffer_desc = allocator->alloc< AttributeBufferDesc >();
				attribute_buffer_desc->data = mesh.normals.getPtr();
				attribute_buffer_desc->size = mesh.normals.getSize() * sizeof( f3 );
				attribute_buffer_desc->usage = BufferUsage::STATIC_DRAW;
				Attribute *attributes = allocator->alloc< Attribute >( 1 );
				attributes->elem_count = 3;
				attributes->normalized = false;
				attributes->offset = 0;
				attributes->stride = sizeof( f3 );
				attributes->src_type = PlainFieldType::FLOAT32;
				attributes->slot = AttributeSlot::NORMAL;
				attribute_buffer_desc->attributes = attributes;
				attribute_buffer_desc->attributes_count = 1;
				monkey_head_mesh.vertex_buffer_handle[ 1 ] = renderer->createVertexBuffer( attribute_buffer_desc );
				monkey_head_mesh.vertex_buffers_count = 2;
			}
			{
				IndexBufferDesc *index_buffer_desc = allocator->alloc< IndexBufferDesc >();
				index_buffer_desc->data = mesh.faces.getPtr();
				index_buffer_desc->size = mesh.faces.getSize() * sizeof( i3 );
				index_buffer_desc->index_type = IndexType::UINT32;
				index_buffer_desc->usage = BufferUsage::STATIC_DRAW;
				monkey_head_mesh.index_buffer_handle = renderer->createIndexBuffer( index_buffer_desc );
			}
			monkey_head_mesh.count = mesh.faces.getSize() * 3;
			monkey_head_mesh.start_index = 0;
			monkey_head_mesh.primitive_type = PrimitiveType::TRIANGLES;
		}
		{
			monkey_head_mesh.primitive_type = PrimitiveType::TRIANGLES;

			TextureDesc *texture_desc = allocator->alloc< TextureDesc >();
			texture_desc->bitmap = head_map;
			texture_desc->mag_filter = MAGFilter::LINEAR;
			texture_desc->min_filter = MINFilter::MIPMAP_LINEAR;
			texture_desc->x_regime = WrapRegime::MIRROR;
			texture_desc->y_regime = WrapRegime::MIRROR;

			Allocator::zero( &default_material , 1 );
			default_material.has_albedo_texture = true;
			default_material.albedo = renderer->createTexture( texture_desc );

			texture_desc = allocator->alloc< TextureDesc >();
			texture_desc->bitmap = tga_map;
			texture_desc->mag_filter = MAGFilter::LINEAR;
			texture_desc->min_filter = MINFilter::MIPMAP_LINEAR;
			texture_desc->x_regime = WrapRegime::MIRROR;
			texture_desc->y_regime = WrapRegime::MIRROR;

			texture_handle = renderer->createTexture( texture_desc );

			static float svertex_positions[] =
			{
				-1.0f , -1.0f , 0.0f , 0.0f , 0.0f , 1.0f ,
				1.0f , -1.0f , 0.0f , 0.0f , 0.0f , 1.0f ,
				1.0f , 1.0f , 0.0f , 0.0f , 0.0f , 1.0f ,
				-1.0f , 1.0f , 0.0f , 0.0f , 0.0f , 1.0f
			};
			static uint sindices[] =
			{
				0 , 1 , 2 , 0 , 2 , 3
			};
			{
				{
					AttributeBufferDesc *attribute_buffer_desc = allocator->alloc< AttributeBufferDesc >();
					float *data = ( float * )allocator->alloc( sizeof( svertex_positions ) );
					Allocator::copy( data , svertex_positions , sizeof( svertex_positions ) / sizeof( float ) );
					attribute_buffer_desc->data = data;
					attribute_buffer_desc->size = sizeof( svertex_positions );
					attribute_buffer_desc->usage = BufferUsage::STATIC_DRAW;
					Attribute *attributes = allocator->alloc< Attribute >( 2 );
					attributes[ 0 ].elem_count = 3;
					attributes[ 0 ].normalized = false;
					attributes[ 0 ].offset = 0;
					attributes[ 0 ].stride = 24;
					attributes[ 0 ].src_type = PlainFieldType::FLOAT32;
					attributes[ 0 ].slot = AttributeSlot::POSITION;
					attributes[ 1 ].elem_count = 3;
					attributes[ 1 ].normalized = false;
					attributes[ 1 ].offset = 12;
					attributes[ 1 ].stride = 24;
					attributes[ 1 ].src_type = PlainFieldType::FLOAT32;
					attributes[ 1 ].slot = AttributeSlot::NORMAL;
					attribute_buffer_desc->attributes = attributes;
					attribute_buffer_desc->attributes_count = 2;
					plane_mesh.vertex_buffer_handle = allocator->alloc< uint >( 1 );
					plane_mesh.vertex_buffer_handle[ 0 ] = renderer->createVertexBuffer( attribute_buffer_desc );
					plane_mesh.vertex_buffers_count = 1;
				}
				{
					IndexBufferDesc *index_buffer_desc = allocator->alloc< IndexBufferDesc >();
					uint *indices = ( uint * )allocator->alloc( sizeof( sindices ) );
					index_buffer_desc->data = indices;
					index_buffer_desc->size = 6 * sizeof( uint );
					Allocator::copy( indices , sindices , 6 );
					index_buffer_desc->index_type = IndexType::UINT32;
					index_buffer_desc->usage = BufferUsage::STATIC_DRAW;
					plane_mesh.index_buffer_handle = renderer->createIndexBuffer( index_buffer_desc );
				}
				plane_mesh.count = 6;
				plane_mesh.start_index = 0;
				plane_mesh.primitive_type = PrimitiveType::TRIANGLES;
			}
		}
		while( true )
		{
			f3 camera_dir = f3{
				MathUtil< float >::cos( camera_angle_thetha ) * MathUtil< float >::cos( camera_angle_phi ) ,
				MathUtil< float >::cos( camera_angle_thetha ) * MathUtil< float >::sin( camera_angle_phi ) ,
				MathUtil< float >::sin( camera_angle_thetha )
			};
			auto res = input_events.pop();
			if( res.isPresent() )
			{
				auto event = res.getValue();
				auto type = event.value;
				auto state = event.key;
				do
				{
					//OS::IO::debugLogln( "event popped" );
					if( type == OS::InputState::EventType::PTR_MOVE )
					{
						ms = state.ptr_pos.xy();
						if( state.ptr_key_state[ 0 ] )
						{
							camera_angle_phi += ( old_state.ptr_pos.x - state.ptr_pos.x ) * 0.01f;
							camera_angle_thetha += ( old_state.ptr_pos.y - state.ptr_pos.y ) * 0.01f;
						}
					}
					camera_distance += ( old_state.ptr_pos.z - state.ptr_pos.z ) * 0.01f;
					Allocator::copy( &old_state , &state , 1 );
				} while( ( res = input_events.pop() ).isPresent() );
			}
			f3 camera_sight_dr;
			if( old_state.keyboard_state[ 'W' ] )
			{
				camera_sight_dr -= camera_dir;
			}
			if( old_state.keyboard_state[ 'S' ] )
			{
				camera_sight_dr += camera_dir;
			}
			if( old_state.keyboard_state[ 'D' ] )
			{
				camera_sight_dr -= camera_dir ^ f3( 0.0f , 0.0f , 1.0f );
			}
			if( old_state.keyboard_state[ 'A' ] )
			{
				camera_sight_dr += camera_dir ^ f3( 0.0f , 0.0f , 1.0f );
			}
			camera_sight_point += camera_sight_dr.norm() * 0.1f;
			auto allocator = renderer->getAuxiliaryAllocator();
			CommandBuffer cmd_buffer( 1000 , allocator );
			//OS::IO::debugLogln( "update thread swaped command buffer" );
			if( allocator == last_consumed )
			{
				OS::IO::debugLogln( "-------------------" );
			}
			last_consumed = allocator;
			//
			//OS::IO::debugLogln( "update thread consumed " , ( size_t )allocator );
			//OS::IO::debugLogln( dt );
			{
				f4x4 lview_proj = Camera::perspectiveLookAt( camera_sight_point + camera_distance * camera_dir ,
				camera_sight_point ,
				{ 0.0f , 0.0f , 1.0f } , 0.01f , 10000.0f , 1.0f , 1.0f );
				/*f4x4 orthographic;
				orthographic( 0 , 0 ) = 1;
				orthographic( 1 , 1 ) = 1;
				orthographic( 2 , 2 ) = 1;
				orthographic( 3 , 3 ) = 1;*/
				f4x4 *view_proj = ( f4x4 * )cmd_buffer.allocate( sizeof( f4x4 ) );
				Allocator::copy( view_proj , &lview_proj , 1 );
				cmd_buffer.push( { CommandType::SET_VIEW_PROJ , 0 , view_proj } );
				cmd_buffer.push( { CommandType::CLEAR_COLOR } );
				cmd_buffer.push( { CommandType::CLEAR_DEPTH } );
			}
			{
				ito( 1 )
				{
					jto( 1 )
					{
						f4x4 model_matrix;
						model_matrix( 0 , 0 ) = 1;
						model_matrix( 1 , 1 ) = 1;
						model_matrix( 2 , 2 ) = 1;
						model_matrix( 3 , 3 ) = 1;
						model_matrix( 0 , 3 ) = i * 2 + camera_sight_point.x;
						model_matrix( 1 , 3 ) = j * 2 + camera_sight_point.y;
						model_matrix( 2 , 3 ) = camera_sight_point.z;
						DrawMeshDesc *desc = ( DrawMeshDesc* )cmd_buffer.allocate( sizeof( DrawMeshDesc ) );
						Allocator::copy( desc , &monkey_head_mesh , 1 );
						desc->material = ( Material* )cmd_buffer.allocate( sizeof( Material ) );
						Allocator::copy( desc->material , &default_material , 1 );
						desc->transform = ( f4x4* )cmd_buffer.allocate( sizeof( f4x4 ) );
						Allocator::copy( desc->transform , &model_matrix , 1 );
						cmd_buffer.push( { CommandType::DRAW_INDEXED_MESH , 0 , desc } );
					}
				}
				
				{
					f4x4 model_matrix;
					model_matrix( 0 , 0 ) = 10;
					model_matrix( 1 , 1 ) = 10;
					model_matrix( 2 , 2 ) = 1;
					model_matrix( 3 , 3 ) = 1;
					model_matrix( 0 , 3 ) = 0;
					model_matrix( 1 , 3 ) = 0;
					model_matrix( 2 , 3 ) = -5;
					DrawMeshDesc *desc = ( DrawMeshDesc* )cmd_buffer.allocate( sizeof( DrawMeshDesc ) );
					Allocator::copy( desc , &plane_mesh , 1 );
					desc->material = ( Material* )cmd_buffer.allocate( sizeof( Material ) );
					Allocator::copy( desc->material , &default_material , 1 );
					desc->transform = ( f4x4* )cmd_buffer.allocate( sizeof( f4x4 ) );
					Allocator::copy( desc->transform , &model_matrix , 1 );
					cmd_buffer.push( { CommandType::DRAW_INDEXED_MESH , 0 , desc } );
				}
				{
					f4x4 model_matrix;
					model_matrix( 0 , 0 ) = 10;
					model_matrix( 1 , 1 ) = 10;
					model_matrix( 2 , 2 ) = 1;
					model_matrix( 3 , 3 ) = 1;
					model_matrix( 0 , 3 ) = 0;
					model_matrix( 1 , 3 ) = 0;
					model_matrix( 2 , 3 ) = 0;
					DrawMeshDesc *desc = ( DrawMeshDesc* )cmd_buffer.allocate( sizeof( DrawMeshDesc ) );
					Allocator::copy( desc , &plane_mesh , 1 );
					desc->material = ( Material* )cmd_buffer.allocate( sizeof( Material ) );
					Allocator::copy( desc->material , &default_material , 1 );
					desc->transform = ( f4x4* )cmd_buffer.allocate( sizeof( f4x4 ) );
					Allocator::copy( desc->transform , &model_matrix , 1 );
					cmd_buffer.push( { CommandType::DRAW_INDEXED_MESH , 0 , desc } );
				}
			}
			renderer->wait();
			//OS::IO::debugLogln( "update thread woked up" );
			renderer->pushCommand( cmd_buffer );
			renderer->render();
		}
	}
	);
	window.run();
	getchar();
	return 0;
}
