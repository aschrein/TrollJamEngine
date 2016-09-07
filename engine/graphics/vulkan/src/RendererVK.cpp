#include <stdafx.h>
#include <engine/graphics/vulkan/RenderingBackendVK.hpp>
#include <engine/graphics/vulkan/Instance.hpp>
#include <engine/graphics/vulkan/CommandBuffer.hpp>
#include <engine/graphics/vulkan/Queue.hpp>
#include <engine/graphics/vulkan/Buffers.hpp>
#include <engine/graphics/vulkan/Pass.hpp>
#include <engine/graphics/Camera.hpp>
#include <engine/assets/FileManager.hpp>
#include <engine/util/defines.hpp>
#include <engine/data_struct/Buffer.hpp>
#include <engine/mem/Allocators.hpp>
#include <engine/os/Window.hpp>
void drawIndexedDispatch( VKInterface::RenderingBackend *backend , void const *data )
{
	Graphics::DrawMeshInfo const *info = ( Graphics::DrawMeshInfo const * )data;
	LocalArray< VkBuffer , 10 > buffers;
	LocalArray< VkDeviceSize , 10 > offsets;
	ito( info->vertex_buffers.size )
	{
		auto &buffer = backend->object_pool.buffers[ info->vertex_buffers[ i ].handler ];
		buffers.push( buffer.getHandle() );
		offsets.push( info->vertex_buffers[ i ].offset );
	}
	vkCmdBindVertexBuffers( backend->cmd_buf.getHandle() , 0 , buffers.size , &buffers[ 0 ] , &offsets[ 0 ] );
	vkCmdBindIndexBuffer( backend->cmd_buf.getHandle() , backend->object_pool.buffers[ info->index_buffer.handler ].getHandle() , info->index_buffer.offset , VK::getVK( info->index_type ) );
	vkCmdDrawIndexed( backend->cmd_buf.getHandle() , info->index_count , info->instance_count , info->start_index , info->vertex_offset, info->start_instance );
}
namespace Graphics
{
	void CommandBuffer::drawIndexed( DrawMeshInfo const *info )
	{
		VKInterface::CommandBuffer* thisvk = ( VKInterface::CommandBuffer* )this;
		thisvk->draw_calls.push( VKInterface::DrawCallInfo{ info , drawIndexedDispatch , uint( info->normalized_distance_from_camera * 0xffff ) } );
	}
	void *CommandBuffer::allocate( uint size )
	{
		VKInterface::CommandBuffer* thisvk = ( VKInterface::CommandBuffer* )this;
		if( !thisvk->linear_allocator )
		{
			thisvk->linear_allocator = new LinearAllocator( 10 );
		}
		return thisvk->linear_allocator->alloc( size );
	}
	CommandBuffer *CommandPool::createCommandBuffer()
	{
		VKInterface::CommandPool* thisvk = ( VKInterface::CommandPool* )this;
		thisvk->buffers_per_pass.size++;
		thisvk->buffers_per_pass[ thisvk->buffers_per_pass.size - 1 ].allocator = thisvk->allocator;
		thisvk->buffers_per_pass[ thisvk->buffers_per_pass.size - 1 ].draw_calls.setAllocator( thisvk->allocator );
		return ( CommandBuffer * )&thisvk->buffers_per_pass[ thisvk->buffers_per_pass.size - 1 ];
	}
	void createBufferDispatch( VKInterface::RenderingBackend *backend , void *data , uint handler )
	{
		BufferInfo *info = ( BufferInfo* )data;
		VkBufferUsageFlags usage_flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		if( info->target == BufferTarget::VERTEX_BUFFER )
		{
			usage_flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		} else if( info->target == BufferTarget::INDEX_BUFFER )
		{
			usage_flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		}
		VK::Buffer buffer = VK::Buffer::create( backend->device , usage_flags , &backend->dev_mem , info->size );
		if( info->data )
		{
			VK::Buffer *stage_buffer = ( VK::Buffer* )info->data;
			backend->cmd_buf.begin();
			backend->cmd_buf.copy( buffer , *stage_buffer , stage_buffer->getSize() );
			backend->cmd_buf.end();
			backend->graphics_queue.submitCommandBuffer( backend->cmd_buf );
			backend->graphics_queue.wait();
			stage_buffer->~Buffer();
			backend->allocator->free( info->data );
		}
		backend->object_pool.buffers.push( std::move( buffer ) );
	}
	void createTextureDispatch( VKInterface::RenderingBackend *backend , void *data , uint handler )
	{
		auto *info = ( TextureInfo* )data;
		VK::Image texture_image = VK::Image::create(
			backend->device , &backend->dev_texture_mem , info->bitmap.width , info->bitmap.height , info->bitmap.mipmaps_count ,
			info->bitmap.layers_count , VK_IMAGE_LAYOUT_GENERAL , VK::getVK( info->bitmap.component_mapping ) ,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT );

		VkBufferImageCopy copy_range;
		Allocator::zero( &copy_range );
		copy_range.imageExtent = { 4 , 4 , 1 };
		copy_range.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT , 0 , 0 , 1 };
		VK::Buffer const *tmp_buffer = ( VK::Buffer const* )info->bitmap.data;
		backend->cmd_buf.begin();
		backend->cmd_buf.copy( texture_image , *tmp_buffer , &copy_range , 1 );
		backend->cmd_buf.ImageBarrier( texture_image , { VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } , VK_ACCESS_COLOR_ATTACHMENT_READ_BIT , VK_IMAGE_LAYOUT_GENERAL );
		backend->cmd_buf.end();
		backend->graphics_queue.submitCommandBuffer( backend->cmd_buf );
		backend->graphics_queue.wait();
		tmp_buffer->~Buffer();
		backend->allocator->free( const_cast< VK::Buffer* >( tmp_buffer ) );
		backend->object_pool.textures.push( std::move( texture_image ) );
	}
	void createTextureViewDispatch( VKInterface::RenderingBackend *backend , void *data , uint handler )
	{
		auto *info = ( TextureViewInfo* )data;
		auto &texture = backend->object_pool.textures[ info->texture_handler ];
		auto view = texture.createView( VK::getVK( info->swizzle ) , { VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } );
		backend->object_pool.views.push( std::move( view ) );
	}
	void createSamplerDispatch( VKInterface::RenderingBackend *backend , void *data , uint handler )
	{
		auto *info = ( SamplerInfo* )data;
		auto sampler = backend->device.createSampler( *info );
		backend->object_pool.samplers.push( std::move( sampler ) );
	}
	void createPassDispatch( VKInterface::RenderingBackend *backend , void *data , uint handler )
	{
		auto *info = ( PassInfo* )data;
		auto pass = VK::Pass::create( backend->device , backend->object_pool , *info );
		backend->passes[ handler ] = std::move( pass );
	}
	void createRenderTargetDispatch( VKInterface::RenderingBackend *backend , void *data , uint handler )
	{

	}
	void createShaderDispatch( VKInterface::RenderingBackend *backend , void *data , uint handler )
	{
		auto descp = *( ShaderInfo* )data;
		backend->object_pool.shaders.push( std::move( VK::Shader::create( backend->device , descp ) ) );
		descp.~ShaderInfo();
	}
	CommandPool *RenderingBackend::createCommandPool( uint pass_handler )
	{
		auto out = new VKInterface::CommandPool();
		out->pass_handler = pass_handler;
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;
		return out;
	}
	uint RenderingBackend::createShader( ShaderInfo info )
	{
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;
		auto *descp = thisgl->allocator->alloc< ShaderInfo >();
		new( descp ) ShaderInfo();
		*descp = info;
		auto new_handler = thisgl->object_pool.shader_counter++;
		thisgl->creation_queue.push( { descp , createShaderDispatch , new_handler } );
		return new_handler;
	}
	uint RenderingBackend::createBuffer( BufferInfo desc )
	{
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;
		BufferInfo *descp = thisgl->allocator->alloc< BufferInfo >();
		Allocator::copy( descp , &desc , 1 );
		uint new_handler = thisgl->object_pool.buffer_counter++;
		if( desc.data )
		{
			VK::Buffer *tmp_buffer = thisgl->allocator->alloc< VK::Buffer >();
			new( tmp_buffer ) VK::Buffer();
			*tmp_buffer = VK::Buffer::create( thisgl->device , VK_BUFFER_USAGE_TRANSFER_SRC_BIT , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT , desc.size , desc.data );
			descp->data = tmp_buffer;
		}
		thisgl->creation_queue.push( { descp , createBufferDispatch , new_handler } );
		return new_handler;
	}
	uint RenderingBackend::createTexture( TextureInfo info )
	{
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;
		auto *descp = thisgl->allocator->alloc< TextureInfo >();
		Allocator::copy( descp , &info , 1 );
		uint new_handler = thisgl->object_pool.texture_counter++;

		VK::Buffer *tmp_buffer = thisgl->allocator->alloc< VK::Buffer >();
		*tmp_buffer = VK::Buffer::create( thisgl->device , VK_BUFFER_USAGE_TRANSFER_SRC_BIT , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT , info.bitmap.size , info.bitmap.data );
		descp->bitmap.data = tmp_buffer;
		thisgl->creation_queue.push( { descp , createTextureDispatch , new_handler } );
		return new_handler;
	}
	uint RenderingBackend::createTextureView( TextureViewInfo info )
	{
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;
		auto *descp = thisgl->allocator->alloc< TextureViewInfo >();
		Allocator::copy( descp , &info , 1 );
		uint new_handler = thisgl->object_pool.view_counter++;
		thisgl->creation_queue.push( { descp , createTextureViewDispatch , new_handler } );
		return new_handler;
	}
	uint RenderingBackend::createRenderTarget( RenderTargetInfo info )
	{
		return 0;
	}
	uint RenderingBackend::createDepthStencilTarget( DepthStencilTargetInfo info )
	{
		return 0;
	}
	uint RenderingBackend::createPass( PassInfo info )
	{
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;
		auto *descp = thisgl->allocator->alloc< PassInfo >();
		Allocator::copy( descp , &info , 1 );
		uint new_handler = thisgl->passes.size++;
		thisgl->creation_queue.push( { descp , createPassDispatch , new_handler } );
		return new_handler;
	}
	uint RenderingBackend::createSampler( SamplerInfo info )
	{
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;
		auto *descp = thisgl->allocator->alloc< SamplerInfo >();
		Allocator::copy( descp , &info , 1 );
		uint new_handler = thisgl->object_pool.sampler_counter++;
		thisgl->creation_queue.push( { descp , createSamplerDispatch , new_handler } );
		return new_handler;
	}
	bool RenderingBackend::isReady()
	{
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;
		return thisgl->ready_flag.isSet();
	}
	void RenderingBackend::wait()
	{
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;
		//if( !ready_flag.isSet() )
		{
			thisgl->ready_signal.wait();
		}
		thisgl->ready_signal.reset();
	}
	void RenderingBackend::render( CommandPool *cmd_pool )
	{
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;
		thisgl->current_command_pool = ( VKInterface::CommandPool* ) cmd_pool;
		thisgl->render_signal.signal();
	}
}
namespace OS
{
	RenderingBackend *Window::createRenderingBackend( Allocators::Allocator *allocator )
	{
		using namespace VK;
		VKInterface::RenderingBackend *rvk = allocator->alloc< VKInterface::RenderingBackend >();
		new( rvk ) VKInterface::RenderingBackend();
		rvk->allocator = allocator;
		rvk->wnd = this;
		rvk->working_flag.set();
		rvk->hdc = hdc;
		rvk->wnd = this;
		HWND window_handler = this->hwnd;
		HINSTANCE window_instance = this->hinstance;
		rvk->thread = Thread::create(
			[ = ]()
		{
			rvk->mainloop();
		} , Allocator::singleton
		);
		rvk->wait();
		return rvk;
	}
}
namespace VKInterface
{
	void RenderingBackend::mainloop()
	{
		instance = VK::Instance::create();
		device = VK::Device::createGraphicsDevice( instance );
		swap_chain = VK::SwapChain::create( instance , device , object_pool , 2 , *this->wnd );
		
		
		auto render_semaphore = device.createSemaphore();
		auto present_semaphore = device.createSemaphore();

		FileConsumer *local_consumer = Allocator::singleton->alloc< FileConsumer >();
		new( local_consumer ) FileConsumer();
		Pointers::Unique< FileConsumer > local_file_consumer( local_consumer , Allocator::singleton );
		String vert_filename = "shaders/vk/vert.spv";
		String frag_filename = "shaders/vk/frag.spv";
		FileManager::singleton->loadFile( { frag_filename , vert_filename } , local_file_consumer.get() );
		Shared< FileImage > frag_file , vert_file;
		int files = 2;
		while( files )
		{
			FileEvent file_event = local_file_consumer->popEvent();
			if( file_event.filename == vert_filename )
			{
				vert_file = std::move( file_event.file_result );
				files--;
			} else if( file_event.filename == frag_filename )
			{
				frag_file = std::move( file_event.file_result );
				files--;
			}
		}
		//object_pool.shader_modules.push( device.createShaderModule( vert_file->getView().getRaw() , vert_file->getView().getLimit() ) );
		//object_pool.shader_modules.push( device.createShaderModule( frag_file->getView().getRaw() , frag_file->getView().getLimit() ) );
		cmd_pool = VK::CommandPool::create( device , device.getGraphicsQueueFamily() );
		cmd_buf = cmd_pool.createCommandBuffer();
		graphics_queue = VK::Queue::createGraphicsQueue( device );
		dev_mem = VK::Memory::create( device , 0x10000 , device.getPhysicalDevice().getMemoryIndex( 29 , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ) );
		VK::Memory texture_mem = VK::Memory::create( device , 0x10000 , device.getPhysicalDevice().getMemoryIndex( 258 , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ) );

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
		};
		uint texture[] =
		{
			0xffffffff , 0x00000000 , 0xff00ff00 , 0x00000000 ,
			0x00000000 , 0xffffffff , 0x000f0000 , 0x00000000 ,
			0x0000f000 , 0x00000000 , 0x00000000 , 0x00000000 ,
			0xf0000000 , 0x000000f0 , 0x00000000 , 0x00000000
		};
		VK::Image texture_image = VK::Image::create( device , &texture_mem , 4 , 4 , 1 , 1 , VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL , VK_FORMAT_R8G8B8A8_UNORM , VK_IMAGE_USAGE_SAMPLED_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT );
		VK::ImageView texture_view = texture_image.createView(
		{ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A } ,
		{ VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 }
		);
		
		{
			VkBufferImageCopy copy_range;
			Allocator::zero( &copy_range );
			copy_range.imageExtent = { 4 , 4 , 1 };
			copy_range.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT , 0 , 0 , 1 };
			VK::Buffer tmp_buffer = VK::Buffer::create( device , VK_BUFFER_USAGE_TRANSFER_SRC_BIT , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT , sizeof( texture ) , texture );
			cmd_buf.begin();
			cmd_buf.copy( texture_image , tmp_buffer , &copy_range , 1 );
			cmd_buf.ImageBarrier( texture_image , { VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } , VK_ACCESS_COLOR_ATTACHMENT_READ_BIT , VK_IMAGE_LAYOUT_GENERAL );
			cmd_buf.end();
			graphics_queue.submitCommandBuffer( cmd_buf );
			graphics_queue.wait();
		}
		VK::Buffer vertex_buffer = VK::Buffer::create( device , VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT , &dev_mem , sizeof( vertices ) );
		VK::Buffer index_buffer = VK::Buffer::create( device , VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT , &dev_mem , 24 );
		{
			VK::Buffer tmp_buffer_i = VK::Buffer::create( device , VK_BUFFER_USAGE_TRANSFER_SRC_BIT , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT , 24 , indices );
			VK::Buffer tmp_buffer_v = VK::Buffer::create( device , VK_BUFFER_USAGE_TRANSFER_SRC_BIT , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT , sizeof( vertices ) , vertices );
			cmd_buf.begin();
			cmd_buf.copy( vertex_buffer , tmp_buffer_v , tmp_buffer_v.getSize() );
			cmd_buf.copy( index_buffer , tmp_buffer_i , tmp_buffer_i.getSize() );
			cmd_buf.end();
			graphics_queue.submitCommandBuffer( cmd_buf );
			graphics_queue.wait();
		}
		object_pool.buffers.push( std::move( vertex_buffer ) );
		object_pool.buffers.push( std::move( index_buffer ) );
		float color[] =
		{
			0.2f , 0.6f , 0.9f , 1.0f
		};*/

		
		//goto wait_section;
		/*Graphics::PassInfo pass_info;
		Allocator::zero( &pass_info );
		pass_info.render_targets.push( 0 );
		pass_info.vertex_attribute_layout.push(
		{
			Graphics::AttributeSlot::POSITION ,
			0 ,
			3 ,
			PlainFieldType::FLOAT32 ,
			false ,
			0
		}
		);
		pass_info.vertex_attribute_layout.push(
		{
			Graphics::AttributeSlot::TEXCOORD ,
			12 ,
			2 ,
			PlainFieldType::FLOAT32 ,
			false ,
			0
		}
		);
		pass_info.vertex_buffer_binding_strides.push( 20 );
		pass_info.viewport_rect = { 0 , 0 , 512 , 512 };
		passes.push( VK::Pass::create( device , object_pool , pass_info ) );
		pass_info.render_targets[ 0 ] = 1;
		passes.push( VK::Pass::create( device , object_pool , pass_info ) );*/
		/*Graphics::DrawMeshInfo draw_mesh_info;
		Allocator::zero( &draw_mesh_info );
		draw_mesh_info.count = 6;
		draw_mesh_info.index_buffer_handle = 1;
		draw_mesh_info.vertex_buffer_handles.push( 0 );
		draw_mesh_info.start_index = 0;
		draw_mesh_info.index_type = Graphics::IndexType::UINT32;*/
		Timer timer;
		timer.updateTime();
		while( working_flag.isSet() )
		{
			timer.updateTime();
			while( !creation_queue.isEmpty() )
			{
				auto p = creation_queue.pop();
				p.dispatch( this , p.data , p.handler );
				allocator->free( p.data );
			}
			swap_chain.acquireNextImage( *present_semaphore );
			//OS::IO::debugLogln( "current image index " , swap_chain.getCurrentImageIndex() );
			cmd_buf.begin();
			auto &attachment = object_pool.attachments[ swap_chain.getCurrentImageIndex() ];
			cmd_buf.ImageBarrier( attachment.getImage() ,
			{ VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } ,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT , VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			);
			cmd_buf.clearImage( attachment.getImage() , { VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } , { 0.7f , 0.4f , 0.32f , 1.0f } );
			if( current_command_pool )
			{
				auto &pass = passes[ current_command_pool->pass_handler ];
				VkClearValue cv[ 1 ] = { { 1.0f , 0.0f , 0.3f , 1.0f } };
				cmd_buf.beginPass( pass , { 0 , 0 , 512 , 512 } , cv );
				for( auto const &cmd_buf : current_command_pool->buffers_per_pass )
				{
					for( auto const &cmd : cmd_buf.draw_calls )
					{
						cmd.dispatch( this , cmd.data );
					}
				}
				cmd_buf.endPass();

				delete current_command_pool;
				current_command_pool = nullptr;
			}
			/*auto &pass = passes[ swap_chain.getCurrentImageIndex() ];
			
			qf rot =
				qf( float3{ 0.0f , 0.0f , 1.0f } , timer.getCurrentTimeMilis() * 1.0e-3f ) //*
				//qf( float3{ 1.0f , 0.0f , 0.0f }.norm() , timer.getCurrentTimeMilis() * -1.0e-2f )
				;
			f4x4 view_proj = Camera::perspectiveLookAt(
			{ -2.0f , 2.0f , 2.0f } ,
			{ 0.0f , 0.0f , 0.0f } ,
			{ 0.0f , 0.0f , -1.0f } ,
			0.01f , 1000.0f , 1.0f , 1.0f
			);
			float sbuffer[ 32 ];
			Allocator::copy< float >( sbuffer , rot.__data , 4 );
			Allocator::copy< float >( sbuffer + 4 , view_proj._data , 16 );
			VK::Buffer uniform_buffer = VK::Buffer::create( device , VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT , 128 , sbuffer );
			{
				VkWriteDescriptorSet write_desc[ 2 ];
				{
					VkWriteDescriptorSet write_buffer_desc;
					Allocator::zero( &write_buffer_desc );
					write_buffer_desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					write_buffer_desc.dstSet = pass.getDescriptorSet();
					write_buffer_desc.descriptorCount = 1;
					write_buffer_desc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					VkDescriptorBufferInfo buf_info;
					Allocator::zero( &buf_info );
					buf_info.buffer = uniform_buffer.getHandle();
					buf_info.offset = 0;
					buf_info.range = 128;
					write_buffer_desc.pBufferInfo = &buf_info;
					write_buffer_desc.dstBinding = 0;
					write_desc[ 0 ] = write_buffer_desc;
				}
				{
					VkDescriptorImageInfo tex_desc;
					tex_desc.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
					tex_desc.imageView = texture_view.getHandle();
					tex_desc.sampler = *texture_sampler;

					VkWriteDescriptorSet write_buffer_desc;
					Allocator::zero( &write_buffer_desc );
					write_buffer_desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					write_buffer_desc.dstSet = pass.getDescriptorSet();
					write_buffer_desc.descriptorCount = 1;
					write_buffer_desc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					write_buffer_desc.pImageInfo = &tex_desc;
					write_buffer_desc.dstBinding = 1;
					write_desc[ 1 ] = write_buffer_desc;
				}
				vkUpdateDescriptorSets( device.getHandle() , 2 , write_desc , 0 , NULL );
			}
			
			drawIndexedDispatch( this , &draw_mesh_info );*/
			cmd_buf.ImageBarrier( attachment.getImage() ,
			{ VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } ,
				0 , VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			);
			cmd_buf.end();
			graphics_queue.submitCommandBuffer( cmd_buf , *present_semaphore , *render_semaphore );
			graphics_queue.present( swap_chain , *render_semaphore );
//wait_section:
			ready_flag.set();
			ready_signal.signal();
			render_signal.wait();
			render_signal.reset();
			ready_flag.reset();
		}
	}
}