#include <stdafx.h>
#include <engine/graphics/vulkan/RendererVK.hpp>
#include <engine/graphics/vulkan/Instance.hpp>
#include <engine/graphics/vulkan/CommandBuffer.hpp>
#include <engine/graphics/vulkan/Queue.hpp>

#include <engine/util/defines.hpp>
#include <engine/data_struct/Buffer.hpp>
#include <engine/mem/Allocators.hpp>
#include <engine/os/Window.hpp>
namespace Graphics
{
	using namespace VK;
	uint Renderer::createVertexBuffer( AttributeBufferDesc const *attribute_buffer_desc )
	{
		RendererVK* thisgl = ( RendererVK* )this;
		return thisgl->pushCreationQueue( { attribute_buffer_desc , CreationType::ATTRIBUTE_BUFFER } );
	}
	uint Renderer::createIndexBuffer( IndexBufferDesc *index_buffer_desc )
	{
		RendererVK* thisgl = ( RendererVK* )this;
		return thisgl->pushCreationQueue( { index_buffer_desc , CreationType::INDEX_BUFFER } );
	}
	uint Renderer::createTexture( TextureDesc const *bitmap )
	{
		RendererVK* thisgl = ( RendererVK* )this;
		return thisgl->pushCreationQueue( { bitmap , CreationType::TEXTURE } );
	}
	bool Renderer::isReady()
	{
		RendererVK* thisgl = ( RendererVK* )this;
		return thisgl->ready_flag.isSet();
	}
	void Renderer::wait()
	{
		RendererVK* thisgl = ( RendererVK* )this;
		//if( !ready_flag.isSet() )
		{
			thisgl->ready_signal.wait();
		}
		thisgl->ready_signal.reset();
	}
	Allocators::Allocator *Renderer::getAuxiliaryAllocator()
	{
		RendererVK* thisgl = ( RendererVK* )this;
		return thisgl->swap_auxiliary_allocator;
	}
	void Renderer::render()
	{
		RendererVK* thisgl = ( RendererVK* )this;
		thisgl->render_signal.signal();
	}
	void Renderer::pushCommand( CommandBuffer cmd_buffer )
	{
		RendererVK* thisgl = ( RendererVK* )this;
		thisgl->command_queue.push( cmd_buffer );
	}
}
namespace OS
{
	Renderer *Window::createRenderer( Allocators::Allocator *allocator )
	{
		using namespace VK;
		RendererVK *rvk = allocator->alloc< RendererVK >();
		new( rvk ) RendererVK();
		rvk->allocator = allocator;
		rvk->wnd = this;
		rvk->working_flag.set();
		rvk->hdc = hdc;
		HWND window_handler = this->hwnd;
		HINSTANCE window_instance = this->hinstance;
		rvk->thread = Thread::create(
			[ = ]()
		{
			Instance instance = Instance::create();
			Device device = Device::createGraphicsDevice( instance );
			SwapChain swap_chain = SwapChain::create( instance , device , 3 , *this );
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
				FileEvent file_event = local_file_consumer->popEvent( true ).getValue();
				if( file_event.filename == vert_filename )
				{
					vert_file = std::move( file_event.file_result.getValue() );
					files--;
				} else if( file_event.filename == frag_filename )
				{
					frag_file = std::move( file_event.file_result.getValue() );
					files--;
				}
			}
			auto vert_shader_module = device.createShaderModule( vert_file.get() );
			auto frag_shader_module = device.createShaderModule( frag_file.get() );
			VkRenderPass render_pass;
			{
				VkAttachmentDescription attachment_desc;
				Allocator::zero( &attachment_desc );
				attachment_desc.format = swap_chain.getFormat();
				attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
				attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachment_desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				attachment_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				VkAttachmentReference attachment_reference;
				Allocator::zero( &attachment_reference );
				attachment_reference.attachment = 0;
				attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				VkSubpassDescription subpass_desc;
				Allocator::zero( &subpass_desc );
				subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subpass_desc.colorAttachmentCount = 1;
				subpass_desc.pColorAttachments = &attachment_reference;
				VkRenderPassCreateInfo render_pass_info;
				Allocator::zero( &render_pass_info );
				render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
				render_pass_info.attachmentCount = 1;
				render_pass_info.pAttachments = &attachment_desc;
				render_pass_info.subpassCount = 1;
				render_pass_info.pSubpasses = &subpass_desc;

				auto res = vkCreateRenderPass( device.getHandle() , &render_pass_info , nullptr , &render_pass );
			}
			VkPipeline pipeline;
			{
				VkPipelineShaderStageCreateInfo infos[] =
				{
					{
						VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO ,
						nullptr ,
						0 ,
						VK_SHADER_STAGE_VERTEX_BIT ,
						*vert_shader_module ,
						"main" ,
						nullptr
					} ,
					{
						VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO ,
						nullptr ,
						0 ,
						VK_SHADER_STAGE_FRAGMENT_BIT ,
						*frag_shader_module ,
						"main" ,
						nullptr
					}
				};
				VkPipelineVertexInputStateCreateInfo vertex_state_create_info;
				Allocator::zero( &vertex_state_create_info );
				vertex_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
				VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info;
				Allocator::zero( &input_assembly_create_info );
				input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
				input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
				input_assembly_create_info.primitiveRestartEnable = VK_FALSE;
				VkViewport viewport =
				{
					0.0f , 0.0f , 512.0f , 512.0f , 0.0f , 1.0f
				};
				VkRect2D scissor =
				{
					{ 0 , 0 } ,
					{ 512 , 512 }
				};
				VkPipelineViewportStateCreateInfo view_port_create_info;
				Allocator::zero( &view_port_create_info );
				view_port_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
				view_port_create_info.viewportCount = 1;
				view_port_create_info.pViewports = &viewport;
				view_port_create_info.scissorCount = 1;
				view_port_create_info.pScissors = &scissor;
				VkPipelineRasterizationStateCreateInfo rasterizer_state_create_info;
				Allocator::zero( &rasterizer_state_create_info );
				rasterizer_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
				rasterizer_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
				rasterizer_state_create_info.cullMode = VK_CULL_MODE_NONE;
				rasterizer_state_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
				rasterizer_state_create_info.lineWidth = 1.0f;
				VkPipelineMultisampleStateCreateInfo multisample_state_create_info;
				Allocator::zero( &multisample_state_create_info );
				multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
				multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
				multisample_state_create_info.minSampleShading = 1.0f;
				VkPipelineColorBlendAttachmentState blender_state;
				Allocator::zero( &blender_state );
				blender_state.blendEnable = VK_FALSE;
				blender_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
				blender_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				blender_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				blender_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				blender_state.colorBlendOp = VK_BLEND_OP_ADD;
				blender_state.alphaBlendOp = VK_BLEND_OP_ADD;
				blender_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
				VkPipelineColorBlendStateCreateInfo blend_state_create_info;
				Allocator::zero( &blend_state_create_info );
				blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
				blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
				blend_state_create_info.attachmentCount = 1;
				blend_state_create_info.pAttachments = &blender_state;
				VkPipelineLayoutCreateInfo pipeline_layout_create_info;
				Allocator::zero( &pipeline_layout_create_info );
				pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				VkPipelineLayout pipeline_layout;
				auto res = vkCreatePipelineLayout( device.getHandle() , &pipeline_layout_create_info , nullptr , &pipeline_layout );
				if( res != VK_SUCCESS )
				{
					OS::IO::debugLogln( "error while creating pipeline layout" );
					return;
				}
				VkGraphicsPipelineCreateInfo pipeline_create_info;
				Allocator::zero( &pipeline_create_info );
				pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
				pipeline_create_info.stageCount = 2;
				pipeline_create_info.pStages = infos;
				pipeline_create_info.pVertexInputState = &vertex_state_create_info;
				pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
				pipeline_create_info.pViewportState = &view_port_create_info;
				pipeline_create_info.pRasterizationState = &rasterizer_state_create_info;
				pipeline_create_info.pMultisampleState = &multisample_state_create_info;
				pipeline_create_info.pColorBlendState = &blend_state_create_info;
				pipeline_create_info.layout = pipeline_layout;
				pipeline_create_info.renderPass = render_pass;
				pipeline_create_info.basePipelineIndex = -1;
				res = vkCreateGraphicsPipelines( device.getHandle() , VK_NULL_HANDLE , 1 , &pipeline_create_info , nullptr , &pipeline );
				if( res != VK_SUCCESS )
				{
					OS::IO::debugLogln( "error while creating pipelines" );
					return;
				}
			}
			CommandPool cmd_pool = CommandPool::create( device , device.getGraphicsQueueFamily() );
			VK::CommandBuffer cmd_buf = cmd_pool.createCommandBuffer();
			VK::Queue graphics_queue = VK::Queue::createGraphicsQueue( device );
			while( true )
			{
				swap_chain.acquireNextImage( *present_semaphore );
				//OS::IO::debugLogln( "current image index " , swap_chain.getCurrentImageIndex() );
				cmd_buf.begin();
				cmd_buf.ImageBarrier( swap_chain.getCurrentImage() ,
				{ ImageAccessMask::READ , ImageLayout::UNDEFINED } ,
				{ ImageAccessMask::WRITE , ImageLayout::COLOR_ATTACHMENT }
				);
				cmd_buf.clearImage( swap_chain.getCurrentImage() , { 1.0f , 1.0f , 0.0f , 1.0f } );
				cmd_buf.ImageBarrier( swap_chain.getCurrentImage() ,
				{ ImageAccessMask::WRITE , ImageLayout::COLOR_ATTACHMENT } ,
				{ ImageAccessMask::READ , ImageLayout::PRESENT }
				);
				VkFramebufferCreateInfo frame_buffer_create_info;
				Allocator::zero( &frame_buffer_create_info );
				frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				frame_buffer_create_info.renderPass = render_pass;
				frame_buffer_create_info.attachmentCount = 1;
				frame_buffer_create_info.pAttachments = &swap_chain.getCurrentImage().getView().getHandle();
				frame_buffer_create_info.width = 512;
				frame_buffer_create_info.height = 512;
				frame_buffer_create_info.layers = 1;
				VK::Unique< VkFramebuffer > frame_buffer;
				frame_buffer.create( device.getHandle() , frame_buffer_create_info );
				VkRenderPassBeginInfo render_pass_begin_info;
				Allocator::zero( &render_pass_begin_info );
				render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				render_pass_begin_info.renderPass = render_pass;
				render_pass_begin_info.framebuffer = *frame_buffer;
				render_pass_begin_info.renderArea =
				{
					{ 0 , 0 } ,
					{ 400 , 400 }
				};
				render_pass_begin_info.clearValueCount = 1;
				VkClearValue clear_value;
				clear_value.color = { 0.1f , 0.12f , 0.14f , 1.0f };
				render_pass_begin_info.pClearValues = &clear_value;
				vkCmdBeginRenderPass( cmd_buf.getHandle() , &render_pass_begin_info , VK_SUBPASS_CONTENTS_INLINE );
				vkCmdBindPipeline( cmd_buf.getHandle() , VK_PIPELINE_BIND_POINT_GRAPHICS , pipeline );
				vkCmdDraw( cmd_buf.getHandle() , 3 , 1 , 0 , 0 );
				vkCmdEndRenderPass( cmd_buf.getHandle() );
				cmd_buf.end();
				graphics_queue.submitCommandBuffer( cmd_buf , *present_semaphore , *render_semaphore );
				graphics_queue.present( swap_chain , *render_semaphore );
			}
			//rvk->mainloop();
		} , Allocator::singleton
		);
		return rvk;
	}
}
namespace VK
{
	uint RendererVK::pushCreationQueue( CreationDesc desc )
	{
		uint new_handler = 0u;
		/*switch( desc.type )
		{
		case CreationType::INDEX_BUFFER:
		case CreationType::ATTRIBUTE_BUFFER:
			new_handler = buffers.getSize();
			buffers.push( Buffer() );
			break;
		case CreationType::TEXTURE:
			new_handler = textures.getSize();
			textures.push( TextureView() );
			break;
		}
		desc.handler = new_handler;
		creation_queue.push( desc );*/
		return new_handler;
	}
	void RendererVK::mainloop()
	{
		Pointers::Unique< FileConsumer > local_file_consumer( new FileConsumer() );
		String vert_filename = "shaders/vk/simple.vs";
		String frag_filename = "shaders/vk/simple.fs";
		FileManager::singleton->loadFile( { frag_filename , vert_filename } , local_file_consumer.get() );
		Shared< FileImage > frag_file , vert_file;
		int files = 2;
		while( files )
		{
			FileEvent file_event = local_file_consumer->popEvent( true ).getValue();
			if( file_event.filename == vert_filename )
			{
				vert_file = std::move( file_event.file_result.getValue() );
				files--;
			} else if( file_event.filename == frag_filename )
			{
				frag_file = std::move( file_event.file_result.getValue() );
				files--;
			}
		}
		Result< Graphics::CommandBuffer > command_res;
		i2 window_size;
		goto wait_section;

		while( working_flag.isSet() )
		{
			window_size = wnd->getSize();
			{
				Result< CreationDesc > res;
				while( ( res = creation_queue.pop() ).isPresent() )
				{
					auto p = res.getValue();
					switch( p.type )
					{
					case CreationType::ATTRIBUTE_BUFFER:
					{
						AttributeBufferDesc const *attribute_buffer_desc = ( AttributeBufferDesc const * )p.data;
					}
					break;
					case CreationType::INDEX_BUFFER:
					{
						IndexBufferDesc const *index_buffer_desc = ( IndexBufferDesc const * )p.data;
					}
					break;
					case CreationType::TEXTURE:
					{
						TextureDesc const *desc = ( TextureDesc const * )p.data;
					}
					break;
					}
				}
			}
			while( ( command_res = command_queue.pop() ).isPresent() )
			{
				auto command_buffer = command_res.getValue();
				ito( command_buffer.getSize() )
				{
					Command const &cmd = command_buffer[ i ];
					switch( cmd.type )
					{
					case CommandType::CLEAR_COLOR:
						break;
					case CommandType::CLEAR_DEPTH:
						break;
					case CommandType::SET_VIEW_PROJ:
					{
						f4x4 *view_proj = ( f4x4 * )cmd.data;
					}
					break;
					case CommandType::DRAW_INDEXED_MESH:
					{
						DrawMeshDesc *desc = ( DrawMeshDesc* )cmd.data;
						Material *material = desc->material;
					}
					break;
					}
				}
			}
			SwapBuffers( hdc );
wait_section:
			auto tmp = auxiliary_allocator;
			auxiliary_allocator = swap_auxiliary_allocator;
			swap_auxiliary_allocator = tmp;
			swap_auxiliary_allocator->reset();
			ready_flag.set();
			ready_signal.signal();
			render_signal.wait();
			render_signal.reset();
			ready_flag.reset();
		}
	}
}