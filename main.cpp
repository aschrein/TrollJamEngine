#include <stdafx.h>
#include <engine/os/Window.hpp>
#include <engine/os/log.hpp>
#include <engine/assets/FileManager.hpp>
#include <engine/assets/Mesh.hpp>
#include <engine/assets/Font.hpp>
#include <engine/graphics/Camera.hpp>
#include <engine/graphics/Graphics.hpp>

#include <Init.hpp>
using namespace Math;
using namespace Graphics;
FileManager *FileManager::singleton;
int main( int argc , char **argv )
{
	using namespace OS;
	Unique< FileConsumer > local_file_consumer( new FileConsumer() );
	FileManager::singleton = FileManager::create( "../../assets" );
	
	float camera_angle_phi = 0.0f;
	float camera_angle_thetha = 0.0f;
	float camera_distance = 1.0f;
	float3 camera_sight_point = { 0.0f , 0.0f , 0.0f };
	int2 ms;
	RenderingBackend *renderer;
	Signal init_signal;
	uint shader_handler;
	uint vertex_buffer_handler;
	uint index_buffer_handler;
	uint render_target_handler;
	uint pass_handler;
	uint uniform_buffer_handler;
	uint uniform_buffer_handler1;
	uint pipeline_handler;
	uint index_count;
	uint albedo_texture_handler;
	LockFree::RingBuffer< Pair< OS::InputState::State , OS::InputState::EventType > , 100 > input_events;
	OS::Window window = OS::Window( { 100 , 100 , 512 , 512 ,
		[ & ]()
	{
		renderer = window.createRenderingBackend();
		auto cmd = renderer->createCommandQueue();
		String mesh_filename = "Group24221.mesh";
		String texture_filename = "nhead.tga";
		FileManager::singleton->loadFile( { mesh_filename , texture_filename } , local_file_consumer.get() );
		Shared< FileImage > mesh_file;
		Shared< FileImage > texture_file;
		int files = 2;
		while( files )
		{
			FileEvent file_event = local_file_consumer->popEvent();
			if( file_event.filename == mesh_filename )
			{
				mesh_file = std::move( file_event.file_result );
				files--;
			} else if( file_event.filename == texture_filename )
			{
				texture_file = std::move( file_event.file_result );
				files--;
			}
		}
		Mesh3D mesh = Assets::Mesh::parseSimpleMesh3d( ( char const * )mesh_file->getView().getRaw() );
		//Assets::Mesh::deserialize( ( byte const * )mesh_file->getView().getRaw() );
		auto texture_bitmap = parseTGA( texture_file->getView() );
		albedo_texture_handler = cmd->createTexture(
		{
			texture_bitmap ,
			{
				Filter::LINEAR , Filter::LINEAR , false , 8 ,
				WrapRegime::CLAMP , WrapRegime::CLAMP , WrapRegime::CLAMP
			} ,
			Usage::STATIC
		}
		);
		index_count = mesh.indices.getSize();
		/*float vertices[] =
		{
			-1.0f , -1.0f , 0.0f , 0.0f , 0.0f ,
			1.0f , -1.0f , 0.0f , 1.0f , 0.0f ,
			1.0f , 1.0f , 0.0f , 1.0f , 1.0f ,
			-1.0f , 1.0f , 0.0f , 0.0f , 1.0f
		};
		uint indices[] =
		{
			0 , 1 , 2 , 0 , 2 , 3
		};*/
		
		/*LocalArray< AttributeInfo , 10 > attrib_infos = {};
		attrib_infos.push( { AttributeSlot::POSITION , { ComponentFormat::RGB , ComponentType::FLOAT32 } } );
		attrib_infos.push( { AttributeSlot::TEXCOORD0 ,{ ComponentFormat::RG , ComponentType::FLOAT32 } } );*/
		vertex_buffer_handler = cmd->createBuffer( { &mesh.vertex_data[ 0 ] , mesh.vertex_data.getSize() , Usage::STATIC , BufferTarget::VERTEX_BUFFER } );
		index_buffer_handler = cmd->createBuffer( { &mesh.indices[ 0 ] , sizeof( uint ) * mesh.indices.getSize() , Usage::STATIC , BufferTarget::INDEX_BUFFER } );
		pipeline_handler = cmd->createPipeline( { mesh.stride , mesh.attrib_info } );
		renderer->submitCommandQueue( cmd );
		renderer->render();
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

		DrawMeshInfo mesh_info;
		Allocator::zero( &mesh_info );
		mesh_info.index_buffer = { index_buffer_handler , 0 };
		mesh_info.vertex_buffer = { vertex_buffer_handler , 0 };
		mesh_info.index_count = index_count;
		mesh_info.pipeline_handle = pipeline_handler;
		Timer timer;
		while( true )
		{
			/*float3 camera_dir = float3{
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
			float3 camera_sight_dr;
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
				camera_sight_dr -= camera_dir ^ float3( 0.0f , 0.0f , 1.0f );
			}
			if( old_state.keyboard_state[ 'A' ] )
			{
				camera_sight_dr += camera_dir ^ float3( 0.0f , 0.0f , 1.0f );
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
						DrawMeshInfo *desc = ( DrawMeshInfo* )cmd_buffer.allocate( sizeof( DrawMeshInfo ) );
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
					DrawMeshInfo *desc = ( DrawMeshInfo* )cmd_buffer.allocate( sizeof( DrawMeshInfo ) );
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
					DrawMeshInfo *desc = ( DrawMeshInfo* )cmd_buffer.allocate( sizeof( DrawMeshInfo ) );
					Allocator::copy( desc , &plane_mesh , 1 );
					desc->material = ( Material* )cmd_buffer.allocate( sizeof( Material ) );
					Allocator::copy( desc->material , &default_material , 1 );
					desc->transform = ( f4x4* )cmd_buffer.allocate( sizeof( f4x4 ) );
					Allocator::copy( desc->transform , &model_matrix , 1 );
					cmd_buffer.push( { CommandType::DRAW_INDEXED_MESH , 0 , desc } );
				}
			}*/
			renderer->waitIdle();
			timer.updateTime();
			auto cmd = renderer->createCommandQueue();
			mesh_info.rotation =
				qf( float3( 0.0f , 0.0f , 1.0f ).norm() , timer.getCurrentTimeMilis() * 1.0e-3f ) * 
				qf( { 1.0f , 0.0f , 0.0f } ,  MathUtil< float >::PI * 0.5f );
			mesh_info.scale = 1.2f;
			cmd->drawIndexed( mesh_info );
			mesh_info.rotation = qf( float3( 1.0f , 0.0f , 0.0f ).norm() , timer.getCurrentTimeMilis() * 1.0e-3f );
			
			//cmd->drawIndexed( mesh_info );
			renderer->submitCommandQueue( cmd );
			renderer->render();
		}
	}
	);
	window.run();
	getchar();
	return 0;
}
