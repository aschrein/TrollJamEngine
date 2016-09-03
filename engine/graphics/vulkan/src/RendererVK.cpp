#include <stdafx.h>
#include <engine/graphics/vulkan/RenderingBackendVK.hpp>
#include <engine/graphics/vulkan/Instance.hpp>
#include <engine/graphics/vulkan/CommandBuffer.hpp>
#include <engine/graphics/vulkan/Queue.hpp>
#include <engine/graphics/vulkan/Buffers.hpp>
#include <engine/graphics/vulkan/Pass.hpp>

#include <engine/assets/FileManager.hpp>
#include <engine/util/defines.hpp>
#include <engine/data_struct/Buffer.hpp>
#include <engine/mem/Allocators.hpp>
#include <engine/os/Window.hpp>
void drawIndexedDispatch( VKInterface::RenderingBackend *backend , void const *data )
{
	Graphics::DrawMeshInfo const *info = ( Graphics::DrawMeshInfo const * )data;
	VkDeviceSize offsets[ 1 ] = { 0 };
	ito( info->vertex_buffer_handles.size )
	{
		auto &buffer = backend->object_pool.buffers[ info->vertex_buffer_handles[ i ] ];
		vkCmdBindVertexBuffers( backend->cmd_buf.getHandle() , 0 , 1 , &buffer.getHandle() , offsets );
	}
	vkCmdBindIndexBuffer( backend->cmd_buf.getHandle() , backend->object_pool.buffers[ info->index_buffer_handle ].getHandle() , 0 , VK::getVK( info->index_type ) );
	vkCmdDrawIndexed( backend->cmd_buf.getHandle() , info->count , 1 , info->start_index , 0 , 0 );
}
namespace Graphics
{
	
	void CommandBuffer::drawIndexed( DrawMeshInfo const *info )
	{
		VKInterface::CommandBuffer* thisvk = ( VKInterface::CommandBuffer* )this;
		thisvk->draw_calls.push( VKInterface::DrawCallInfo{ info , drawIndexedDispatch , info->distance_from_camera } );
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
	CommandBuffer *CommandPool::createCommandBuffer( uint pass_id )
	{
		VKInterface::CommandPool* thisvk = ( VKInterface::CommandPool* )this;
		thisvk->buffers_per_pass.size++;
		thisvk->buffers_per_pass[ thisvk->buffers_per_pass.size - 1 ].key = pass_id;
		thisvk->buffers_per_pass[ thisvk->buffers_per_pass.size - 1 ].value.allocator = thisvk->allocator;
		thisvk->buffers_per_pass[ thisvk->buffers_per_pass.size - 1 ].value.draw_calls.setAllocator( thisvk->allocator );
		return ( CommandBuffer * )&thisvk->buffers_per_pass[ thisvk->buffers_per_pass.size - 1 ].value;
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
	Pointers::Unique< CommandPool > RenderingBackend::createCommandPool()
	{
		return new VKInterface::CommandPool();
	}
	uint RenderingBackend::createBuffer( BufferInfo desc )
	{
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;
		BufferInfo *descp = thisgl->allocator->alloc< BufferInfo >();
		Allocator::copy< BufferInfo >( descp , &desc , 1 );
		uint new_handler = thisgl->object_pool.buffer_counter++;
		BufferInfo *info = ( BufferInfo* )desc.data;
		if( info->data )
		{
			VK::Buffer *tmp_buffer = thisgl->allocator->alloc< VK::Buffer >();
			*tmp_buffer = VK::Buffer::create( thisgl->device , VK_BUFFER_USAGE_TRANSFER_SRC_BIT , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT , info->size , info->data );
			info->data = tmp_buffer;
		}
		thisgl->creation_queue.push( { descp , createBufferDispatch , new_handler } );
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
	void RenderingBackend::render( Pointers::Unique< CommandPool > &&cmd_pool )
	{
		VKInterface::RenderingBackend* thisgl = ( VKInterface::RenderingBackend* )this;
		thisgl->current_command_pool = std::move( cmd_pool );
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
		object_pool.shader_modules.push( device.createShaderModule( vert_file->getView().getRaw() , vert_file->getView().getLimit() ) );
		object_pool.shader_modules.push( device.createShaderModule( frag_file->getView().getRaw() , frag_file->getView().getLimit() ) );
		cmd_pool = VK::CommandPool::create( device , device.getGraphicsQueueFamily() );
		cmd_buf = cmd_pool.createCommandBuffer();
		graphics_queue = VK::Queue::createGraphicsQueue( device );
		dev_mem = VK::Memory::create( device , 0x10000 , device.getPhysicalDevice().getMemoryIndex( 29 , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ) );

		float vertices[] =
		{
			-0.8f , -1.0f , 0.0f , 0.0f , 0.0f ,
			1.0f , -1.0f , 0.0f , 1.0f , 0.0f ,
			1.0f , 1.0f , 0.0f , 1.0f , 1.0f ,
			-1.0f , 1.0f , 0.0f , 0.0f , 1.0f
		};
		uint indices[] =
		{
			0 , 1 , 2 , 0 , 2 , 3
		};
		
		
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
		/*float color[] =
		{
			0.2f , 0.6f , 0.9f , 1.0f
		};
		/VK::Buffer uniform_buffer = VK::Buffer::create( device , VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT , 16 , color );
		{
			VkWriteDescriptorSet writeDescriptorSet = {};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.dstSet = *desc_set;
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			VkDescriptorBufferInfo buf_info;
			Allocator::zero( &buf_info );
			buf_info.buffer = uniform_buffer.getHandle();
			buf_info.offset = 0;
			buf_info.range = 16;
			writeDescriptorSet.pBufferInfo = &buf_info;
			writeDescriptorSet.dstBinding = 0;
			vkUpdateDescriptorSets( device.getHandle() , 1 , &writeDescriptorSet , 0 , NULL );
		}(*/
		//goto wait_section;
		Graphics::PassInfo pass_info;
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
		LocalArray< VK::Pass , 10 > passes;
		passes.push( VK::Pass::create( device , object_pool , pass_info ) );
		pass_info.render_targets[ 0 ] = 1;
		passes.push( VK::Pass::create( device , object_pool , pass_info ) );
		Graphics::DrawMeshInfo draw_mesh_info;
		Allocator::zero( &draw_mesh_info );
		draw_mesh_info.count = 6;
		draw_mesh_info.index_buffer_handle = 1;
		draw_mesh_info.vertex_buffer_handles.push( 0 );
		draw_mesh_info.start_index = 0;
		draw_mesh_info.index_type = Graphics::IndexType::UINT32;
		while( working_flag.isSet() )
		{
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
			cmd_buf.clearImage( attachment.getImage() , { VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } , { 1.0f , 0.0f , 0.0f , 1.0f } );
			
			auto &pass = passes[ swap_chain.getCurrentImageIndex() ];
			VkRenderPassBeginInfo render_pass_begin_info;
			Allocator::zero( &render_pass_begin_info );
			render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_begin_info.renderPass = pass.getPass();
			render_pass_begin_info.framebuffer = pass.getFrameBuffer();
			render_pass_begin_info.renderArea =
			{
				{ 0 , 0 } ,
				{ 512 , 512 }
			};
			render_pass_begin_info.clearValueCount = 1;
			VkClearValue clear_value;
			clear_value.color = { 0.1f , 0.12f , 0.14f , 1.0f };
			render_pass_begin_info.pClearValues = &clear_value;
			vkCmdBeginRenderPass( cmd_buf.getHandle() , &render_pass_begin_info , VK_SUBPASS_CONTENTS_INLINE );
			vkCmdBindPipeline( cmd_buf.getHandle() , VK_PIPELINE_BIND_POINT_GRAPHICS , pass.getPipeline() );
			VkDeviceSize offsets[ 1 ] = { 0 };
			vkCmdBindDescriptorSets( cmd_buf.getHandle() , VK_PIPELINE_BIND_POINT_GRAPHICS , pass.getPipelineLayout() , 0 , 1 , &pass.getDescriptorSet() , 0 , NULL );
			drawIndexedDispatch( this , &draw_mesh_info );
			vkCmdEndRenderPass( cmd_buf.getHandle() );
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