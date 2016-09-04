#pragma once
#include <engine/graphics/vulkan/defines.hpp>
#include <engine/graphics/vulkan/Device.hpp>
#include <engine/graphics/vulkan/ObjectPool.hpp>
#include <engine/data_struct/Buffer.hpp>

namespace VK
{
	class Pass
	{
		VK_OBJECT( Pass );
	private:
		Unique< VkRenderPass > render_pass;
		Unique< VkPipeline > pipeline;
		Unique< VkDescriptorPool > desc_pool;
		Unique< VkDescriptorSet > desc_set;
		Unique< VkPipelineLayout > pipeline_layout;
		Unique< VkDescriptorSetLayout > desc_set_layout;
		Unique< VkFramebuffer > frame_buffer;
		LocalArray< uint , 10 > attachment_indices;
	public:
		static Pass create( Device const &device , ObjectPool const &pool , Graphics::PassInfo const &info )
		{
			LocalArray< VkAttachmentDescription , 10 > attachments_desc;
			LocalArray< VkAttachmentReference , 10 > attachments_ref;
			LocalArray< VkImageView , 10 > attachments_views;
			Pass out;
			ito( info.render_targets.size )
			{
				auto const &attachment = pool.attachments[ info.render_targets[ i ] ];
				out.attachment_indices.push( info.render_targets[ i ] );
				VkAttachmentDescription attachment_desc;
				Allocator::zero( &attachment_desc );
				attachment_desc.format = attachment.getView().getFormat();
				attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
				attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachment_desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				attachment_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				attachments_desc.push( attachment_desc );
				VkAttachmentReference attachment_reference;
				Allocator::zero( &attachment_reference );
				attachment_reference.attachment = i;
				attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				attachments_ref.push( attachment_reference );
				attachments_views.push( attachment.getView().getHandle() );
			}
			VkSubpassDescription subpass_desc;
			if( info.use_depth_stencil )
			{
				auto const &view = pool.attachments[ info.depth_stencil_target ];
				VkAttachmentDescription attachment_desc;
				Allocator::zero( &attachment_desc );
				attachment_desc.format = view.getView().getFormat();
				attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
				attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachment_desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				attachment_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				attachments_desc.push( attachment_desc );
				attachments_views.push( view.getView().getHandle() );
				VkAttachmentReference attachment_reference;
				Allocator::zero( &attachment_reference );
				attachment_reference.attachment = attachments_desc.size - 1;
				attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				subpass_desc.pDepthStencilAttachment = &attachment_reference;
			}
			
			Allocator::zero( &subpass_desc );
			subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass_desc.colorAttachmentCount = attachments_ref.size;
			subpass_desc.pColorAttachments = &attachments_ref[ 0 ];
			VkRenderPassCreateInfo render_pass_info;
			Allocator::zero( &render_pass_info );
			render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			render_pass_info.attachmentCount = attachments_desc.size;
			render_pass_info.pAttachments = &attachments_desc[ 0 ];
			render_pass_info.subpassCount = 1;
			render_pass_info.pSubpasses = &subpass_desc;
			
			out.render_pass.create( device.getHandle() , render_pass_info );

			VkFramebufferCreateInfo frame_buffer_create_info;
			Allocator::zero( &frame_buffer_create_info );
			frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frame_buffer_create_info.renderPass = *out.render_pass;
			frame_buffer_create_info.attachmentCount = attachments_views.size;
			frame_buffer_create_info.pAttachments = &attachments_views[ 0 ];
			frame_buffer_create_info.width = pool.attachments[ info.render_targets[ 0 ] ].getImage().getSize().x;
			frame_buffer_create_info.height = pool.attachments[ info.render_targets[ 0 ] ].getImage().getSize().y;
			frame_buffer_create_info.layers = 1;
			out.frame_buffer.create( device.getHandle() , frame_buffer_create_info );

			LocalArray< VkDescriptorSetLayoutBinding , 10 > layout_bindings;

			/*VkDescriptorSetLayoutBinding bindongs[] =
			{
				{
					0 ,
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ,
					1 ,
					VK_SHADER_STAGE_ALL ,
					NULL
				} ,
				{
					1 ,
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ,
					1 ,
					VK_SHADER_STAGE_ALL ,
					NULL
				}
			};*/
			VkDescriptorSetLayoutCreateInfo desc_set_layout_info;
			Allocator::zero( &desc_set_layout_info );
			desc_set_layout_info.bindingCount = layout_bindings.size;
			desc_set_layout_info.pBindings = &layout_bindings[ 0 ];
			desc_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

			out.desc_set_layout.create( device.getHandle() , desc_set_layout_info );

			LocalArray< VkDescriptorPoolSize , 10 > type_counts;
			/*VkDescriptorPoolSize type_counts[ 2 ];
			type_counts[ 0 ].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			type_counts[ 0 ].descriptorCount = 1;
			type_counts[ 1 ].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			type_counts[ 1 ].descriptorCount = 1;*/
			VkDescriptorPoolCreateInfo desc_pool_info;
			Allocator::zero( &desc_pool_info );
			desc_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			desc_pool_info.poolSizeCount = type_counts.size;
			desc_pool_info.pPoolSizes = &type_counts[ 0 ];
			desc_pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			desc_pool_info.maxSets = 1;
			out.desc_pool.create( device.getHandle() , desc_pool_info );

			VkDescriptorSetAllocateInfo desc_set_info = {};
			desc_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			desc_set_info.descriptorPool = *out.desc_pool;
			desc_set_info.descriptorSetCount = 1;
			desc_set_info.pSetLayouts = &out.desc_set_layout;
			out.desc_set.create( device.getHandle() , *out.desc_pool , desc_set_info );


			VkPipelineShaderStageCreateInfo infos[] =
			{
				{
					VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO ,
					nullptr ,
					0 ,
					VK_SHADER_STAGE_VERTEX_BIT ,
					*pool.shader_modules[ 0 ] ,
					"main" ,
					nullptr
				} ,
				{
					VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO ,
					nullptr ,
					0 ,
					VK_SHADER_STAGE_FRAGMENT_BIT ,
					*pool.shader_modules[ 1 ] ,
					"main" ,
					nullptr
				}
			};
			VkPipelineVertexInputStateCreateInfo vertex_state_create_info;
			Allocator::zero( &vertex_state_create_info );
			vertex_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			LocalArray< VkVertexInputBindingDescription , 10 > in_bind_info;
			ito( info.vertex_buffer_binding_strides.size )
			{
				VkVertexInputBindingDescription bind_info;
				Allocator::zero( &bind_info );
				bind_info.binding = i;
				bind_info.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				bind_info.stride = info.vertex_buffer_binding_strides[ i ];
				in_bind_info.push( bind_info );
			}
			vertex_state_create_info.pVertexBindingDescriptions = &in_bind_info[ 0 ];
			vertex_state_create_info.vertexBindingDescriptionCount = in_bind_info.size;
			LocalArray< VkVertexInputAttributeDescription , 10 > attrib_desc;
			ito( info.vertex_attribute_layout.size )
			{
				VkFormat attribute_format;
				switch( info.vertex_attribute_layout[ i ].src_type )
				{
				case PlainFieldType::FLOAT32:
					switch( info.vertex_attribute_layout[ i ].elem_count )
					{
					case 1:
						attribute_format = VK_FORMAT_R32_SFLOAT;
						goto cont;
					case 2:
						attribute_format = VK_FORMAT_R32G32_SFLOAT;
						goto cont;
					case 3:
						attribute_format = VK_FORMAT_R32G32B32_SFLOAT;
						goto cont;
					case 4:
						attribute_format = VK_FORMAT_R32G32B32A32_SFLOAT;
						goto cont;
					}
				}
cont:
				attrib_desc.push(
				{
					getVK( info.vertex_attribute_layout[ i ].slot ) ,
					info.vertex_attribute_layout[ i ].buffer_index ,
					attribute_format ,
					info.vertex_attribute_layout[ i ].offset
				} );
			}
		
			vertex_state_create_info.pVertexAttributeDescriptions = &attrib_desc[ 0 ];
			vertex_state_create_info.vertexAttributeDescriptionCount = attrib_desc.size;
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
			pipeline_layout_create_info.pSetLayouts = &out.desc_set_layout;
			out.pipeline_layout.create( device.getHandle() , pipeline_layout_create_info );
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
			pipeline_create_info.layout = *out.pipeline_layout;
			pipeline_create_info.renderPass = *out.render_pass;
			pipeline_create_info.basePipelineIndex = -1;
			out.pipeline.create( device.getHandle() , pipeline_create_info );
			
			return out;
		}
		VkRenderPass const &getPass() const
		{
			return *render_pass;
		}
		uint getAttachmentCount() const
		{
			return attachment_indices.size;
		}
		uint getAttachmentIndex( uint i ) const
		{
			return attachment_indices[ i ];
		}
		VkPipeline const &getPipeline() const
		{
			return *pipeline;
		}
		VkFramebuffer const &getFrameBuffer() const
		{
			return *frame_buffer;
		}
		VkDescriptorSet const &getDescriptorSet() const
		{
			return *desc_set;
		}
		VkPipelineLayout const &getPipelineLayout() const
		{
			return *pipeline_layout;
		}
	};
}