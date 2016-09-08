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
uint texture_handle = 0;
FileManager *FileManager::singleton;
DrawMeshInfo monkey_head_mesh;
DrawMeshInfo plane_mesh;
int main( int argc , char ** argv )
{
	using namespace OS;
	Unique< FileConsumer > local_file_consumer( new FileConsumer() );
	FileManager::singleton = FileManager::create();
	
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
	LockFree::RingBuffer< Pair< OS::InputState::State , OS::InputState::EventType > , 100 > input_events;
	OS::Window window = OS::Window( { 100 , 100 , 512 , 512 ,
		[ & ]()
	{
		renderer = window.createRenderingBackend();
		using namespace Shaders;
		constexpr size_t t = sizeof( Token );
		constexpr size_t s = sizeof( Expr );
		Token in_pos( "in_pos" );
		Stage vertex_stage;
		vertex_stage.addIn( 0 , Type::FLOAT3 , in_pos );
		vertex_stage.getBody().addExpr( Expr::setVertexPos( Expr::createFloat4( in_pos , { 1.0 } ) ) );
		Stage fragment_stage;
		Token out_color( "out_color" );
		fragment_stage.addOut( 0 , Type::FLOAT4 , out_color );
		fragment_stage.getBody().addExpr( out_color << Expr::createFloat4( { 1.0 } , { 0.4 } , { 0.0 } , { 1.0 } ) );
		shader_handler = renderer->createShader( { "simple" , {
			Pair< StageType , Shaders::Stage >{ StageType::VERTEX , vertex_stage } ,
			Pair< StageType , Shaders::Stage >{ StageType::FRAGMENT , fragment_stage } ,
		} } );
		float vertices[] =
		{
			-1.0f , -1.0f , 0.0f ,
			1.0f , -1.0f , 0.0f ,
			1.0f , 1.0f , 0.0f ,
			-1.0f , 1.0f , 0.0f
		};
		uint indices[] =
		{
			0 , 1 , 2 , 0 , 2 , 3
		};
		vertex_buffer_handler = renderer->createBuffer( { vertices , 48 , Usage::STATIC , BufferTarget::VERTEX_BUFFER } );
		index_buffer_handler = renderer->createBuffer( { indices , 24 , Usage::STATIC , BufferTarget::INDEX_BUFFER } );
		RenderTargetCreateInfo rtinfo;
		rtinfo.component_mapping = { ComponentFormat::BGRA , ComponentType::UNORM8 };
		rtinfo.size = { 512 , 512 };
		render_target_handler = renderer->createRenderTarget( rtinfo );
		PassCreateInfo pass_info;
		Allocator::zero( &pass_info );
		pass_info.render_targets.push( render_target_handler );
		pass_info.shader_handler = shader_handler;
		AttributeInfo attr_info;
		Allocator::zero( &attr_info );
		attr_info.buffer_index = 0;
		attr_info.elem_count = 3;
		attr_info.src_type = PlainFieldType::FLOAT32;
		pass_info.vertex_attribute_layout.push( attr_info );
		pass_info.vertex_buffer_binding_strides.push( 12 );
		pass_handler = renderer->createPass( pass_info );
		renderer->render( nullptr );
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
		mesh_info.vertex_buffers.push( { 0 , 0 } );
		mesh_info.instance_count = 1;
		mesh_info.index_count = 3;
		
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
			renderer->wait();
			auto cmd_pool = renderer->createCommandPool( pass_handler );
			auto cmd = cmd_pool->createCommandBuffer();
			cmd->drawIndexed( &mesh_info );
			//OS::IO::debugLogln( "update thread woked up" );
			//renderer->pushCommand( cmd_buffer );
			renderer->render( std::move( cmd_pool ) );
		}
	}
	);
	window.run();
	getchar();
	return 0;
}
