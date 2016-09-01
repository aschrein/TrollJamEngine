#include <stdafx.h>
#include <engine/graphics/vulkan/RenderingBackendVK.hpp>
#include <engine/graphics/vulkan/Instance.hpp>
#include <engine/graphics/vulkan/CommandBuffer.hpp>
#include <engine/graphics/vulkan/Queue.hpp>
#include <engine/graphics/vulkan/Buffers.hpp>
#include <engine/assets/FileManager.hpp>
#include <engine/util/defines.hpp>
#include <engine/data_struct/Buffer.hpp>
#include <engine/mem/Allocators.hpp>
#include <engine/os/Window.hpp>
namespace Graphics
{
	using namespace VK;
	uint RenderingBackend::createVertexBuffer( BufferInfo const *desc )
	{
		RenderingBackendVK* thisgl = ( RenderingBackendVK* )this;
		return thisgl->pushCreationQueue( { desc , CreationType::VERTEX_BUFFER } );
	}
	uint RenderingBackend::createIndexBuffer( BufferInfo const *desc )
	{
		RenderingBackendVK* thisgl = ( RenderingBackendVK* )this;
		return thisgl->pushCreationQueue( { desc , CreationType::INDEX_BUFFER } );
	}
	uint RenderingBackend::createTexture( TextureInfo const *bitmap )
	{
		RenderingBackendVK* thisgl = ( RenderingBackendVK* )this;
		return thisgl->pushCreationQueue( { bitmap , CreationType::TEXTURE } );
	}
	uint RenderingBackend::createTextureView( TextureViewInfo const *info )
	{
		RenderingBackendVK* thisgl = ( RenderingBackendVK* )this;
		return thisgl->pushCreationQueue( { info , CreationType::TEXTURE } );
	}
	uint RenderingBackend::createRenderTarget( RenderTargetInfo const *info )
	{
		RenderingBackendVK* thisgl = ( RenderingBackendVK* )this;
		return thisgl->pushCreationQueue( { info , CreationType::TEXTURE } );
	}
	uint RenderingBackend::createSampler( SamplerInfo const *info )
	{
		RenderingBackendVK* thisgl = ( RenderingBackendVK* )this;
		return thisgl->pushCreationQueue( { info , CreationType::TEXTURE } );
	}
	bool RenderingBackend::isReady()
	{
		RenderingBackendVK* thisgl = ( RenderingBackendVK* )this;
		return thisgl->ready_flag.isSet();
	}
	void RenderingBackend::wait()
	{
		RenderingBackendVK* thisgl = ( RenderingBackendVK* )this;
		//if( !ready_flag.isSet() )
		{
			thisgl->ready_signal.wait();
		}
		thisgl->ready_signal.reset();
	}
	void RenderingBackend::render()
	{
		RenderingBackendVK* thisgl = ( RenderingBackendVK* )this;
		thisgl->render_signal.signal();
	}
	void RenderingBackend::pushCommand( CommandBufferPool *cmd_pool )
	{
		RenderingBackendVK* thisgl = ( RenderingBackendVK* )this;
		//thisgl->command_queue.push( cmd_buffer );
	}
}
namespace OS
{
	RenderingBackend *Window::createRenderingBackend( Allocators::Allocator *allocator )
	{
		using namespace VK;
		RenderingBackendVK *rvk = allocator->alloc< RenderingBackendVK >();
		new( rvk ) RenderingBackendVK();
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
			auto vert_shader_module = device.createShaderModule( vert_file->getView().getRaw() , vert_file->getView().getLimit() );
			auto frag_shader_module = device.createShaderModule( frag_file->getView().getRaw() , frag_file->getView().getLimit() );
			VK::Unique< VkRenderPass > render_pass;
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
				render_pass.create( device.getHandle() , render_pass_info );
			}
			VK::Unique< VkPipeline > pipeline;
			VK::Unique< VkDescriptorPool > desc_pool;
			VK::Unique< VkDescriptorSet > desc_set;
			VK::Unique< VkPipelineLayout > pipeline_layout;
			VK::Unique< VkDescriptorSetLayout > desc_set_layout;
			{
				VkDescriptorSetLayoutBinding bindongs[] =
				{
					0 ,
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ,
					1 ,
					VK_SHADER_STAGE_FRAGMENT_BIT ,
					NULL
				};
				VkDescriptorSetLayoutCreateInfo desc_set_layout_info;
				Allocator::zero( &desc_set_layout_info );
				desc_set_layout_info.bindingCount = 1;
				desc_set_layout_info.pBindings = bindongs;
				desc_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				
				desc_set_layout.create( device.getHandle() , desc_set_layout_info );

				VkDescriptorPoolSize type_counts[ 1 ];
				type_counts[ 0 ].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				type_counts[ 0 ].descriptorCount = 1;
				VkDescriptorPoolCreateInfo desc_pool_info;
				Allocator::zero( &desc_pool_info );
				desc_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				desc_pool_info.poolSizeCount = 1;
				desc_pool_info.pPoolSizes = type_counts;
				desc_pool_info.maxSets = 1;
				desc_pool.create( device.getHandle() , desc_pool_info );

				VkDescriptorSetAllocateInfo desc_set_info = {};
				desc_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				desc_set_info.descriptorPool = *desc_pool;
				desc_set_info.descriptorSetCount = 1;
				desc_set_info.pSetLayouts = &desc_set_layout;
				desc_set.create( device.getHandle() , *desc_pool , desc_set_info );
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
				VkVertexInputBindingDescription in_bind_info;
				{
					Allocator::zero( &in_bind_info );
					in_bind_info.binding = 0;
					in_bind_info.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
					in_bind_info.stride = 20;
				}
				vertex_state_create_info.pVertexBindingDescriptions = &in_bind_info;
				vertex_state_create_info.vertexBindingDescriptionCount = 1;

				VkVertexInputAttributeDescription attrib_desc[] =
				{
					{ 0 ,  0 , VK_FORMAT_R32G32B32_SFLOAT , 0 } ,
					{ 1 ,  0 , VK_FORMAT_R32G32_SFLOAT , 12 }
				};
				vertex_state_create_info.pVertexAttributeDescriptions = attrib_desc;
				vertex_state_create_info.vertexAttributeDescriptionCount = 2;
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
				pipeline_layout_create_info.setLayoutCount = 1;
				pipeline_layout_create_info.pSetLayouts = &desc_set_layout;
				pipeline_layout.create( device.getHandle() , pipeline_layout_create_info );
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
				pipeline_create_info.layout = *pipeline_layout;
				pipeline_create_info.renderPass = *render_pass;
				pipeline_create_info.basePipelineIndex = -1;
				pipeline.create( device.getHandle() , pipeline_create_info );
			}
			CommandPool cmd_pool = CommandPool::create( device , device.getGraphicsQueueFamily() );
			VK::CommandBuffer cmd_buf = cmd_pool.createCommandBuffer();
			VK::Queue graphics_queue = VK::Queue::createGraphicsQueue( device );
			float vertices[] =
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
			float color[] =
			{
				0.2f , 0.6f , 0.9f , 1.0f
			};
			Memory dev_mem = Memory::create( device , 0x1000 , device.getPhysicalDevice().getMemoryIndex( 29 , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ) );
			Buffer vertex_buffer = Buffer::create( device , VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT , &dev_mem , sizeof( vertices ) );
			Buffer index_buffer = Buffer::create( device , VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT , &dev_mem , 24 );
			Buffer uniform_buffer = Buffer::create( device , VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT , 16 , color );
			{
				Buffer tmp_buffer_i = Buffer::create( device , VK_BUFFER_USAGE_TRANSFER_SRC_BIT , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT , 24 , indices );
				Buffer tmp_buffer_v = Buffer::create( device , VK_BUFFER_USAGE_TRANSFER_SRC_BIT , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT , sizeof( vertices ) , vertices );
				cmd_buf.begin();
				cmd_buf.copy( vertex_buffer , tmp_buffer_v , tmp_buffer_v.getSize() );
				cmd_buf.copy( index_buffer , tmp_buffer_i , tmp_buffer_i.getSize() );
				cmd_buf.end();
				graphics_queue.submitCommandBuffer( cmd_buf );
				graphics_queue.wait();
			}
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
			}
			while( true )
			{
				swap_chain.acquireNextImage( *present_semaphore );
				//OS::IO::debugLogln( "current image index " , swap_chain.getCurrentImageIndex() );
				cmd_buf.begin();
				cmd_buf.ImageBarrier( swap_chain.getCurrentImage() ,
				{ VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } ,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT , VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				);
				cmd_buf.clearImage( swap_chain.getCurrentImage() , { VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } , { 1.0f , 0.0f , 0.0f , 1.0f } );
				VkFramebufferCreateInfo frame_buffer_create_info;
				Allocator::zero( &frame_buffer_create_info );
				frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				frame_buffer_create_info.renderPass = *render_pass;
				frame_buffer_create_info.attachmentCount = 1;
				frame_buffer_create_info.pAttachments = &swap_chain.getCurrentImageView().getHandle();
				frame_buffer_create_info.width = 512;
				frame_buffer_create_info.height = 512;
				frame_buffer_create_info.layers = 1;
				VK::Unique< VkFramebuffer > frame_buffer;
				frame_buffer.create( device.getHandle() , frame_buffer_create_info );
				VkRenderPassBeginInfo render_pass_begin_info;
				Allocator::zero( &render_pass_begin_info );
				render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				render_pass_begin_info.renderPass = *render_pass;
				render_pass_begin_info.framebuffer = *frame_buffer;
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
				vkCmdBindPipeline( cmd_buf.getHandle() , VK_PIPELINE_BIND_POINT_GRAPHICS , *pipeline );
				VkDeviceSize offsets[ 1 ] = { 0 };
				vkCmdBindDescriptorSets( cmd_buf.getHandle() , VK_PIPELINE_BIND_POINT_GRAPHICS , *pipeline_layout , 0 , 1 , &desc_set , 0 , NULL );
				vkCmdBindVertexBuffers( cmd_buf.getHandle() , 0 , 1 , &vertex_buffer.getHandle() , offsets );
				vkCmdBindIndexBuffer( cmd_buf.getHandle() , index_buffer.getHandle() , 0 , VK_INDEX_TYPE_UINT32 );
				vkCmdDrawIndexed( cmd_buf.getHandle() , 6 , 1 , 0 , 0 , 0 );
				vkCmdEndRenderPass( cmd_buf.getHandle() );
				cmd_buf.ImageBarrier( swap_chain.getCurrentImage() ,
				{ VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } ,
					0 , VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
				);
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
	uint RenderingBackendVK::pushCreationQueue( CreationDesc desc )
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
	void RenderingBackendVK::mainloop()
	{
		/*Pointers::Unique< FileConsumer > local_file_consumer( new FileConsumer() );
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
		}*/
	}
}