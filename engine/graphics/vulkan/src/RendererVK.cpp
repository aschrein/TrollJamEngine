#include <stdafx.h>
#include <engine/graphics/vulkan/RenderingBackendVK.hpp>
#include <engine/graphics/vulkan/SwapChain.hpp>
#include <engine/graphics/Camera.hpp>
#include <engine/assets/FileManager.hpp>
#include <engine/util/defines.hpp>
#include <engine/data_struct/Buffer.hpp>
#include <engine/mem/Allocators.hpp>
#include <engine/os/Window.hpp>
void drawIndexedDispatch( VKInterface::RenderingBackend *backend , VKInterface::GraphicsState &state ,
	VK::CommandBuffer const &graphics_cmd , VK::CommandBuffer const &transfer_cmd ,
	uint transfer_queue_index , void *data )
{
	Graphics::DrawMeshInfo const *info = ( Graphics::DrawMeshInfo const * )data;
	auto &pipeline = backend->device.pipelines_pool.objects[ info->pipeline_handle ].pipeline;
	if( state.current_pipeline != info->pipeline_handle )
	{
		state.current_pipeline = info->pipeline_handle;
		graphics_cmd.bindGraphicsPipeline( pipeline.handle );
		vkCmdPushConstants( graphics_cmd.getHandle() , pipeline.pipeline_layout , VK_SHADER_STAGE_VERTEX_BIT , 16 , 64 , state.view_proj._data );
	}
	auto &buffer = backend->device.buffer_pool.objects[ info->vertex_buffer.buffer_handler ];
	VkDeviceSize offset = info->vertex_buffer.offset;
	vkCmdBindVertexBuffers( graphics_cmd.getHandle() , 0 , 1 , &buffer.handle , &offset );
	vkCmdBindIndexBuffer( graphics_cmd.getHandle() , backend->device.buffer_pool.objects[ info->index_buffer.buffer_handler ].handle , info->index_buffer.offset , VK::getVK( info->index_type ) );
	vkCmdPushConstants( graphics_cmd.getHandle() , pipeline.pipeline_layout , VK_SHADER_STAGE_VERTEX_BIT , 0 , 16 , info->rotation.__data );
	vkCmdDrawIndexed( graphics_cmd.getHandle() , info->index_count , 1 , info->start_index , 0 , 0 );
}
namespace Graphics
{
	void CommandQueue::drawIndexed( DrawMeshInfo const &info )
	{
		VKInterface::CommandQueue* thisgl = ( VKInterface::CommandQueue* )this;
		DrawMeshInfo *swap_desc = ( DrawMeshInfo* )thisgl->temp_allocator.alloc( sizeof( DrawMeshInfo ) );
		Allocator::copy( swap_desc , &info );
		thisgl->commands.push( VKInterface::Command{ drawIndexedDispatch , swap_desc } );
	}
	/*void *CommandBuffer::allocate( uint size )
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
	}*/
	void createBufferDispatch( VKInterface::RenderingBackend *backend , VKInterface::GraphicsState &state ,
		VK::CommandBuffer const &graphics_cmd , VK::CommandBuffer const &transfer_cmd ,
		uint transfer_queue_index , void *data )
	{
		auto *info = ( BufferCreateInfo* )data;
		VkBufferUsageFlags usage_flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		if( info->target == BufferTarget::VERTEX_BUFFER )
		{
			usage_flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		} else if( info->target == BufferTarget::INDEX_BUFFER )
		{
			usage_flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		}
		VK::Buffer buffer = backend->device.createBuffer( usage_flags , VK::MemoryType::DEV_BUFFER , info->size ).value;
		if( info->data )
		{
			auto tmp_buf_desc = backend->device.createBuffer( VK_BUFFER_USAGE_TRANSFER_SRC_BIT , VK::MemoryType::HOST , info->size );
			VK::Buffer tmp_buf = tmp_buf_desc.value;
			auto map = backend->device.map( tmp_buf );
			memcpy( map , info->data , info->size );
			backend->device.unmap( tmp_buf );
			transfer_cmd.begin();
			transfer_cmd.copy( buffer , tmp_buf , { 0 , 0 , info->size } );
			transfer_cmd.end();
			backend->device.submitGraphicsCommandBuffer( transfer_queue_index , transfer_cmd );
			backend->device.waitIdleGraphics( transfer_queue_index );
			backend->device.releaseBuffer( tmp_buf_desc.key );
		}
	}

	void createPipelineDispatch( VKInterface::RenderingBackend *backend , VKInterface::GraphicsState &state ,
		VK::CommandBuffer const &graphics_cmd , VK::CommandBuffer const &transfer_cmd ,
		uint transfer_queue_index , void *data )
	{
		auto *info = ( PipelineCreateInfo* )data;
		
		VkPipelineColorBlendAttachmentState blend_state_create_info;
		{
			Allocator::zero( &blend_state_create_info );
			blend_state_create_info.blendEnable = VK_FALSE;
			blend_state_create_info.colorWriteMask = 0xf;
			
		}
		LocalArray< VkVertexInputBindingDescription , 10 > attrib_binding_infos = {};
		LocalArray< VkVertexInputAttributeDescription , 10 > attrib_infos = {};
		//uint stride = 0;
		for( auto attrib_info : info->attributes )
		{
			attrib_infos.push(
			{
				( uint )attrib_info.slot , 0 ,
				VK::getVK( attrib_info.component_mapping ) , attrib_info.offset
			}
			);
			//stride = getBpp( attrib_info.component_mapping );
		}
		attrib_binding_infos.push( { 0 , info->stride , VK_VERTEX_INPUT_RATE_VERTEX } );
		auto pipeline = backend->device.createPipeline(
		{
			0 , 0 ,
			{ 2 , 0 , 1 } ,
			{ 0 ,{ 1 ,{ { 0 , VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER , 1 , VK_SHADER_STAGE_ALL } } } } ,
			{ 1 ,{ VK_SHADER_STAGE_VERTEX_BIT , 0 , 80 } } ,
				attrib_binding_infos ,
				attrib_infos  ,
				VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST , { 1 , blend_state_create_info }
		}
		).value;
	}
	/*void createTextureDispatch( VKInterface::RenderingBackend *backend , void *data , uint handler )
	{
		auto *info = ( TextureCreateInfo* )data;
		VK::Image texture_image = VK::Image::create(
			backend->device , &backend->dev_texture_mem , info->bitmap.width , info->bitmap.height , info->bitmap.mipmaps_count ,
			info->bitmap.layers_count , VK_IMAGE_LAYOUT_GENERAL , VK::getVK( info->bitmap.component_mapping ) ,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT , VK_IMAGE_ASPECT_COLOR_BIT );

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
		auto *info = ( TextureViewCreateInfo* )data;
		auto &texture = backend->object_pool.textures[ info->texture_handler ];
		auto view = texture.createView( VK::getVK( info->swizzle ) , { VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } );
		backend->object_pool.views.push( std::move( view ) );
	}
	void createSamplerDispatch( VKInterface::RenderingBackend *backend , void *data , uint handler )
	{
		auto *info = ( SamplerCreateInfo* )data;
		auto sampler = backend->device.createSampler( *info );
		backend->object_pool.samplers.push( std::move( sampler ) );
	}
	void createPassDispatch( VKInterface::RenderingBackend *backend , void *data , uint handler )
	{
		auto *info = ( PassCreateInfo* )data;
		auto pass = VK::Pass::create( backend->device , backend->object_pool , *info );
		backend->passes[ handler ] = std::move( pass );
	}
	void createRenderTargetDispatch( VKInterface::RenderingBackend *backend , void *data , uint handler )
	{
		auto *info = ( RenderTargetCreateInfo* )data;
		backend->object_pool.attachments.push( VK::Attachment::createRenderTarget( backend->device , &backend->dev_texture_mem , *info , false , VK_IMAGE_USAGE_TRANSFER_SRC_BIT ) );
	}
	void createShaderDispatch( VKInterface::RenderingBackend *backend , void *data , uint handler )
	{
		auto descp = *( ShaderCreateInfo* )data;
		backend->object_pool.shaders.push( std::move( VK::Shader::create( backend->device , descp ) ) );
		descp.~ShaderCreateInfo();
	}
	CommandPool *RenderingBackend::createCommandPool( uint pass_handler )
	{
		auto out = new VKInterface::CommandPool();
		out->pass_handler = pass_handler;
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;
		return out;
	}
	uint RenderingBackend::createShader( ShaderCreateInfo info )
	{
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;
		auto *descp = thisgl->allocator->alloc< ShaderCreateInfo >();
		new( descp ) ShaderCreateInfo();
		*descp = info;
		auto new_handler = thisgl->object_pool.shader_counter++;
		thisgl->creation_queue.push( { descp , createShaderDispatch , new_handler } );
		return new_handler;
	}
	uint RenderingBackend::createTexture( TextureCreateInfo info )
	{
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;
		auto *descp = thisgl->allocator->alloc< TextureCreateInfo >();
		Allocator::copy( descp , &info , 1 );
		uint new_handler = thisgl->object_pool.texture_counter++;

		VK::Buffer *tmp_buffer = thisgl->allocator->alloc< VK::Buffer >();
		*tmp_buffer = VK::Buffer::create( thisgl->device , VK_BUFFER_USAGE_TRANSFER_SRC_BIT , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT , info.bitmap.size , info.bitmap.data );
		descp->bitmap.data = tmp_buffer;
		thisgl->creation_queue.push( { descp , createTextureDispatch , new_handler } );
		return new_handler;
	}
	uint RenderingBackend::createTextureView( TextureViewCreateInfo info )
	{
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;
		auto *descp = thisgl->allocator->alloc< TextureViewCreateInfo >();
		Allocator::copy( descp , &info , 1 );
		uint new_handler = thisgl->object_pool.view_counter++;
		thisgl->creation_queue.push( { descp , createTextureViewDispatch , new_handler } );
		return new_handler;
	}
	uint RenderingBackend::createRenderTarget( RenderTargetCreateInfo info )
	{
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;
		auto *descp = thisgl->allocator->alloc< RenderTargetCreateInfo >();
		Allocator::copy( descp , &info , 1 );
		uint new_handler = thisgl->object_pool.attachment_counter++;
		thisgl->creation_queue.push( { descp , createRenderTargetDispatch , new_handler } );
		return new_handler;
	}
	uint RenderingBackend::createDepthStencilTarget( DepthStencilTargetCreateInfo info )
	{
		return 0;
	}
	uint RenderingBackend::createPass( PassCreateInfo info )
	{
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;
		auto *descp = thisgl->allocator->alloc< PassCreateInfo >();
		Allocator::copy( descp , &info , 1 );
		uint new_handler = thisgl->passes.size++;
		thisgl->creation_queue.push( { descp , createPassDispatch , new_handler } );
		return new_handler;
	}
	uint RenderingBackend::createSampler( SamplerCreateInfo info )
	{
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;
		auto *descp = thisgl->allocator->alloc< SamplerCreateInfo >();
		Allocator::copy( descp , &info , 1 );
		uint new_handler = thisgl->object_pool.sampler_counter++;
		thisgl->creation_queue.push( { descp , createSamplerDispatch , new_handler } );
		return new_handler;
	}
	bool RenderingBackend::isReady()
	{
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;
		return thisgl->ready_flag.isSet();
	}*/
	uint CommandQueue::createBuffer( BufferCreateInfo const &desc )
	{
		VKInterface::CommandQueue* thisgl = ( VKInterface::CommandQueue* )this;
		void *swap = thisgl->temp_allocator.alloc( desc.size );
		memcpy( swap , desc.data , desc.size );
		BufferCreateInfo *swap_desc = ( BufferCreateInfo* )thisgl->temp_allocator.alloc( sizeof( BufferCreateInfo ) );
		Allocator::copy( swap_desc , &desc );
		swap_desc->data = swap;
		thisgl->commands.push( VKInterface::Command{ createBufferDispatch , swap_desc } );
		return thisgl->device->buffer_pool.counter++;
	}
	uint CommandQueue::createPipeline( PipelineCreateInfo const &desc )
	{
		VKInterface::CommandQueue* thisgl = ( VKInterface::CommandQueue* )this;
		PipelineCreateInfo *swap_desc = ( PipelineCreateInfo* )thisgl->temp_allocator.alloc( sizeof( PipelineCreateInfo ) );
		Allocator::copy( swap_desc , &desc );
		thisgl->commands.push( VKInterface::Command{ createPipelineDispatch , swap_desc } );
		return thisgl->device->pipelines_pool.counter++;
	}
	CommandQueue *RenderingBackend::createCommandQueue()
	{
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;
		auto cmd_queue = thisgl->allocator->alloc< VKInterface::CommandQueue >();
		Allocator::zero( cmd_queue );
		cmd_queue->device = &thisgl->device;

		cmd_queue->temp_allocator = std::move( LinearAllocator( 4000 ) );
		return cmd_queue;
	}
	void RenderingBackend::waitIdle()
	{
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;
		//if( !ready_flag.isSet() )
		{
			thisgl->ready_signal.wait();
		}
		thisgl->ready_signal.reset();
	}
	void RenderingBackend::submitCommandQueue( CommandQueue *queue )
	{
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;

		thisgl->swap_command_queues.push( ( VKInterface::CommandQueue* )queue );
	}
	void RenderingBackend::render()
	{
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;

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
		rvk->waitIdle();
		return rvk;
	}
}
namespace VKInterface
{
	void RenderingBackend::mainloop()
	{
		auto instance = VK::Instance::create();
		device = VK::Device::createGraphicsDevice( instance , 2 , 1 );
		auto swap_chain = VK::SwapChain::create( instance.getHandle() , instance.getPhysicalDevice().getHandle() , device.getHandle() , 2 , *this->wnd );
		
		
		auto render_semaphore = device.createSemaphore().value;
		auto present_semaphore = device.createSemaphore().value;

		FileConsumer *local_consumer = allocator->alloc< FileConsumer >();
		new( local_consumer ) FileConsumer();
		Pointers::Unique< FileConsumer > local_file_consumer( local_consumer , allocator );
		String vert_filename = "shaders/vk/simple.vert.spv";
		String frag_filename = "shaders/vk/simple.frag.spv";
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
		
		auto cmd_pool = VK::CommandPool::create( device.getHandle() , device.getPhysicalDevice().getGraphicsQueueFamily() );
		auto graphics_cmd = cmd_pool.createPrimaryCommandBuffer();
		auto transfer_cmd = cmd_pool.createPrimaryCommandBuffer();
		struct RenderTarget
		{
			VK::Image image;
			VK::ImageView view;
			VK::Image depth_image;
			VK::ImageView depth_view;
		};
		RenderTarget rt;
		rt.image = device.createImage2D(
			512 , 512 , 1 , 1 , VK_IMAGE_LAYOUT_PREINITIALIZED , VK_FORMAT_B8G8R8A8_UNORM ,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
		).value;
		rt.view = device.createView2D( rt.image ).value;
		rt.depth_image = device.createImage2D(
			512 , 512 , 1 , 1 , VK_IMAGE_LAYOUT_UNDEFINED , VK_FORMAT_D32_SFLOAT_S8_UINT ,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
		).value;
		rt.depth_view = device.createView2D( rt.depth_image ,
		{} ,
		{ VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT , 0 , 1 , 0 , 1 } ).value;
		VK::PassInfo pass_info = {};
		pass_info.subpasses.push( { { 1 , { VK::defaultAttachmentRef( 0 ) } } , true , VK::defaultDepthStencilAttachmentRef( 1 ) } );
		pass_info.attachments.push( VK::defaultAttachmentDesc( rt.image.format ) );
		pass_info.attachments.push( VK::defaultDepthStencilAttachmentDesc( rt.depth_image.format ) );
		opaque_pass.pass = device.createGraphicsRenderPass( pass_info ).value;
		opaque_pass.frame_buffer = device.createFrameBuffer( opaque_pass.pass , { 2 , rt.view.handle , rt.depth_view.handle } , { 512 , 512 } ).value;
		auto vertex_module = device.createShaderModule( vert_file->getView().getRaw() , vert_file->getView().getLimit() , VK_SHADER_STAGE_VERTEX_BIT );
		auto fragment_module = device.createShaderModule( frag_file->getView().getRaw() , frag_file->getView().getLimit() , VK_SHADER_STAGE_FRAGMENT_BIT );
		
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
		auto vb = device.createBuffer( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT , VK::MemoryType::DEV_BUFFER , sizeof( vertices ) ).value;
		auto ib = device.createBuffer( VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT , VK::MemoryType::DEV_BUFFER , sizeof( vertices ) ).value;
		auto tmpb = device.createBuffer( VK_BUFFER_USAGE_TRANSFER_SRC_BIT , VK::MemoryType::HOST , 256 ).value;
		{
			auto map = device.map( tmpb );
			memcpy( map , vertices , sizeof( vertices ) );
			memcpy( ( byte* )map + sizeof( vertices ) , indices , sizeof( indices ) );
			device.unmap( tmpb );
			graphics_cmd.begin();
			graphics_cmd.copy( vb , tmpb , { 0 , 0 , sizeof( vertices ) } );
			graphics_cmd.copy( ib , tmpb , { sizeof( vertices ) , 0 , sizeof( indices ) } );
			graphics_cmd.end();
			device.submitGraphicsCommandBuffer( 0 , graphics_cmd );
			device.waitIdleGraphics( 0 );
		}*/
		/*auto blitimage = */
		/*Graphics::RenderTargetInfo rtinfo;
		Graphics::ComponentMapping mapping;
		mapping.format = Graphics::ComponentFormat::BGRA;
		mapping.type = Graphics::ComponentType::UNORM8;
		rtinfo.component_mapping = mapping;
		rtinfo.size = { 512 , 512 };
		object_pool.attachments.push( VK::Attachment::createRenderTarget( device , &texture_mem , rtinfo , false , VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT ) );
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
			while( !local_file_consumer->isEmpty() )
			{
				auto file_event = local_file_consumer->popEvent();
				if( file_event.filename == vert_filename )
				{
					device.rebuildShaderModule( vertex_module.key , vert_file->getView().getRaw() , vert_file->getView().getLimit() );
				} else if( file_event.filename == frag_filename )
				{
					device.rebuildShaderModule( fragment_module.key , frag_file->getView().getRaw() , frag_file->getView().getLimit() );
				}
			}
			for( auto &cmd : swap_command_queues )
			{
				current_command_queues.push( cmd );
			}
			swap_command_queues.size = 0;
			timer.updateTime();
			swap_chain.acquireNextImage( present_semaphore );
			//OS::IO::debugLogln( "current image index " , swap_chain.getCurrentImageIndex() );
			//cmd_buf.begin();
			auto &attachment = swap_chain.getCurrentAttachment();
			graphics_cmd.begin();
			graphics_cmd.ImageBarrier( attachment ,
			{ VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } ,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT ,
				VK_ACCESS_COLOR_ATTACHMENT_READ_BIT , VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			);
			graphics_cmd.clearImage( attachment , { VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } , { 0.7f , 0.4f , 0.32f , 1.0f } );
			
			graphics_cmd.ImageBarrier( rt.image ,
			{ VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } ,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT ,
				VK_ACCESS_COLOR_ATTACHMENT_READ_BIT , VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			);
			graphics_cmd.ImageBarrier( rt.depth_image ,
			{ VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT , 0 , 1 , 0 , 1 } ,
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT ,
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT , VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			);
			graphics_cmd.clearImage( rt.image , { VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } , { 0.6f , 0.15f , 0.23f , 1.0f } );
			graphics_cmd.clearDepthStencilImage( rt.depth_image , { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT , 0 , 1 , 0 , 1 } , 1.0f , 0 );
			VkClearValue clear_values[ 2 ];
			clear_values[ 0 ].color = { 0.0f , 1.0f , 0.0f , 1.0f };
			clear_values[ 1 ].depthStencil = { 1.0f , 0 };
			graphics_cmd.beginPass( opaque_pass.pass , opaque_pass.frame_buffer , { 0 , 0 , 512 , 512 } , clear_values , 2 );
			
			//vkCmdSetLineWidth( graphics_cmd.getHandle() , 1.0f );
			VkRect2D scissors[] =
			{
				{ 0 , 0 , 512 , 512 }
			};
			vkCmdSetScissor( graphics_cmd.getHandle() , 0 , 1 , scissors );
			VkViewport viewports[] =
			{
				{ 0 , 0 , 512 , 512 , 0.0f , 1.0f }
			};
			vkCmdSetViewport( graphics_cmd.getHandle() , 0 , 1 , viewports );

			//VkDeviceSize offset = 0;
			//vkCmdBindVertexBuffers( graphics_cmd.getHandle() , 0 , 1 , &vb.handle , &offset );
			//vkCmdBindIndexBuffer( graphics_cmd.getHandle() , ib.handle , 0 , VK_INDEX_TYPE_UINT32 );
			//vkCmdDrawIndexed( graphics_cmd.getHandle() , 6 , 1 , 0 , 0 , 0 );
			//vkCmdDraw( graphics_cmd.getHandle() , 4 , 1 , 0 , 0 );
			//vkCmdSetStencilCompareMask( graphics_cmd.getHandle() , VK_STENCIL_FACE_FRONT_BIT , VK_STENCIL_OP_KEEP );
			//vkCmdSetStencilWriteMask( graphics_cmd.getHandle() , VK_STENCIL_FACE_FRONT_BIT , VK_STENCIL_OP_KEEP );
			//vkCmdSetStencilReference( graphics_cmd.getHandle() , VK_STENCIL_FACE_FRONT_BIT , VK_STENCIL_OP_KEEP );

			auto view_proj = Camera::perspectiveLookAt( { 2.0f , 2.0f , 2.0f } , { 0.0f , 0.0f , 0.0f } , { 0.0f , 0.0f , -1.0f } , 0.1f , 1000.0f , 1.0f , 1.0f );
			
			/*LocalArray< VkWriteDescriptorSet , 10 > write_desc;
			ito( info->uniform_buffers.size )
			{
				auto &buffer = backend->object_pool.buffers[ info->uniform_buffers[ i ].buffer_handler ];
				VkWriteDescriptorSet write_buffer_desc;
				Allocator::zero( &write_buffer_desc );
				write_buffer_desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write_buffer_desc.dstSet = current_pass->getDescriptorSet( 0 );
				write_buffer_desc.descriptorCount = 1;
				write_buffer_desc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				VkDescriptorBufferInfo buf_info;
				Allocator::zero( &buf_info );
				buf_info.buffer = buffer.getHandle();
				buf_info.offset = 0;// info->uniform_buffers[ i ].offset;
				ub_offsets.push( info->uniform_buffers[ i ].offset );
				buf_info.range = current_pass->getConstantsLayout()[ i ].range;
				write_buffer_desc.pBufferInfo = &buf_info;
				write_buffer_desc.dstBinding = current_pass->getConstantsLayout()[ i ].location;
				write_desc.push( write_buffer_desc );
			}
			if( write_desc.size )
			{
				vkUpdateDescriptorSets( backend->device.getHandle() , write_desc.size , &write_desc[ 0 ] , 0 , NULL );
				auto dset = current_pass->getDescriptorSet( 0 );
				vkCmdBindDescriptorSets( backend->cmd_buf.getHandle() , VK_PIPELINE_BIND_POINT_GRAPHICS , current_pass->getPipelineLayout() , 0 , 1 , &dset , ub_offsets.size , &ub_offsets[ 0 ] );
			}*/
			GraphicsState state{ view_proj , -1 };
			for( auto &cmd_buf : current_command_queues )
			{
				for( auto &cmd : cmd_buf->commands )
				{
					cmd.dispatch( this , state , graphics_cmd , transfer_cmd , 1 , cmd.data );
				}
				cmd_buf->~CommandQueue();
				allocator->free( cmd_buf );
			}
			graphics_cmd.endPass();
			current_command_queues.size = 0;
			VkImageBlit blit_info;
			Allocator::zero( &blit_info );
			blit_info.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT , 0 , 0 , 1 };
			blit_info.dstOffsets[ 1 ] = { 512 , 512 , 0 };
			blit_info.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT , 0 , 0 , 1 };
			blit_info.srcOffsets[ 1 ] = { 512 , 512 , 0 };

			graphics_cmd.ImageBarrier( rt.image ,
			{ VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } ,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT ,
				VK_ACCESS_TRANSFER_READ_BIT , VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
			);
			graphics_cmd.ImageBarrier( attachment ,
			{ VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } ,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT ,
				VK_ACCESS_TRANSFER_WRITE_BIT , VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
			);

			graphics_cmd.blit( attachment , rt.image , &blit_info , 1 , VK_FILTER_NEAREST );
			/*if( current_command_pool )
			{
				auto &blit_src = object_pool.attachments[ 2 ];
				cmd_buf.ImageBarrier( blit_src.getImage() ,
				{ VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } ,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT , VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				);
				cmd_buf.clearImage( blit_src.getImage() , { VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } , { 0.0f , 0.4f , 0.32f , 1.0f } );
				auto &pass = passes[ current_command_pool->pass_handler ];
				VkClearValue cv[ 1 ] = { { 0.0f , 0.6f , 0.3f , 1.0f } };
				cmd_buf.beginPass( pass , { 0 , 0 , 512 , 512 } , cv );
				for( auto const &cmd_buf : current_command_pool->buffers_per_pass )
				{
					for( auto const &cmd : cmd_buf.draw_calls )
					{
						cmd.dispatch( this , &pass , cmd.data );
					}
				}
				cmd_buf.endPass();
				
				VkImageBlit blit_info;
				Allocator::zero( &blit_info );
				blit_info.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT , 0 , 0 , 1 };
				blit_info.dstOffsets[ 1 ] = { 256 , 256 , 0 };
				blit_info.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT , 0 , 0 , 1 };
				blit_info.srcOffsets[ 1 ] = { 512 , 512 , 0 };
				
				
				cmd_buf.ImageBarrier( attachment.getImage() ,
				{ VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } ,
					VK_ACCESS_TRANSFER_WRITE_BIT , VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
				);

				cmd_buf.ImageBarrier( blit_src.getImage() ,
				{ VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } ,
					VK_ACCESS_TRANSFER_READ_BIT , VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
				);

				cmd_buf.blit( attachment.getImage() , blit_src.getImage() , &blit_info , 1 , VK_FILTER_NEAREST );
				delete current_command_pool;
				current_command_pool = nullptr;
			}*/
			
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
			graphics_cmd.ImageBarrier( attachment ,
			{ VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } ,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT ,
				VK_ACCESS_COLOR_ATTACHMENT_READ_BIT , VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			);
			graphics_cmd.end();
			device.submitGraphicsCommandBuffer( 0 , graphics_cmd , present_semaphore , render_semaphore );
			device.present( 0 , swap_chain.getHandle() , swap_chain.getCurrentImageIndex() , render_semaphore );
			device.waitIdleGraphics( 0 );
//wait_section:
			ready_flag.set();
			ready_signal.signal();
			render_signal.wait();
			render_signal.reset();
			ready_flag.reset();
		}
	}
}