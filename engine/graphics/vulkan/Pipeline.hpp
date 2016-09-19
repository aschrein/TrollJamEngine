#pragma once
#include <engine/graphics/vulkan/defines.hpp>
#include <engine/data_struct/Buffer.hpp>
namespace VK
{
	struct DescriptorSetInfo
	{
		LocalArray< VkDescriptorSetLayoutBinding , 10 > bindings;
	};
	struct Pipeline
	{
		VkPipeline handle;
		VkDescriptorPool desc_pool;
		LocalArray< VkDescriptorSet , 10 > desc_sets;
		VkPipelineLayout pipeline_layout;
		LocalArray< VkDescriptorSetLayout , 10 > desc_set_layouts;
	};
	static Pipeline vkCreate( VkDevice device ,
		VkRenderPass renderpass , uint subpass ,
		LocalArray< Pair< VkShaderStageFlagBits , VkShaderModule > , 5 > const &stages ,
		LocalArray< DescriptorSetInfo , 5 > const &desc_info ,
		LocalArray< VkPushConstantRange , 5 > const &push_constants ,
		LocalArray< VkVertexInputBindingDescription , 10 > attrib_binding_info ,
		LocalArray< VkVertexInputAttributeDescription , 10 > attrib_info ,
		VkPrimitiveTopology topology ,
		LocalArray< VkPipelineColorBlendAttachmentState , 5 > color_blend_state_info
	)
	{
		LocalArray< VkDescriptorPoolSize , 10 > type_counts = {};
		for( auto const &di : desc_info )
		{
			for( auto const &bn : di.bindings )
			{
				bool set = false;
				for( auto &ps : type_counts )
				{
					if( ps.type == bn.descriptorType )
					{
						ps.descriptorCount += bn.descriptorCount;
					}
				}
				if( !set )
				{
					type_counts.push( { bn.descriptorType , bn.descriptorCount } );
				}
			}
		}
		Pipeline out = {};
		if( type_counts.size )
		{
			VkDescriptorPoolCreateInfo desc_pool_info;
			Allocator::zero( &desc_pool_info );
			desc_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			desc_pool_info.poolSizeCount = type_counts.size;
			desc_pool_info.pPoolSizes = &type_counts[ 0 ];
			desc_pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			desc_pool_info.maxSets = desc_info.size;
			out.desc_pool = vkNew( device , desc_pool_info );
			for( auto const &di : desc_info )
			{
				VkDescriptorSetLayoutCreateInfo desc_set_layout_info;
				Allocator::zero( &desc_set_layout_info );
				desc_set_layout_info.bindingCount = di.bindings.size;
				desc_set_layout_info.pBindings = &di.bindings[ 0 ];
				desc_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				auto layout = vkNew( device , desc_set_layout_info );
				VkDescriptorSetAllocateInfo desc_set_info = {};
				desc_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				desc_set_info.descriptorPool = out.desc_pool;
				desc_set_info.descriptorSetCount = 1;
				desc_set_info.pSetLayouts = &layout;
				auto desc_set = vkNew( device , out.desc_pool , desc_set_info );
				out.desc_sets.push( desc_set );
				out.desc_set_layouts.push( layout );
			}
		}
		LocalArray< VkPipelineShaderStageCreateInfo , 5 > infos = {};
		for( auto const &stage : stages )
		{
			infos.push(
			{
				VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO ,
				nullptr ,
				0 ,
				stage.key ,
				stage.value ,
				"main" ,
				nullptr
			}
			);
		}
		VkPipelineVertexInputStateCreateInfo vertex_state_create_info;
		Allocator::zero( &vertex_state_create_info );
		vertex_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_state_create_info.pVertexBindingDescriptions = &attrib_binding_info[ 0 ];
		vertex_state_create_info.vertexBindingDescriptionCount = attrib_binding_info.size;
		vertex_state_create_info.pVertexAttributeDescriptions = &attrib_info[ 0 ];
		vertex_state_create_info.vertexAttributeDescriptionCount = attrib_info.size;
		VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info;
		Allocator::zero( &input_assembly_create_info );
		input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_create_info.topology = topology;
		input_assembly_create_info.primitiveRestartEnable = VK_FALSE;
		LocalArray< VkDynamicState , 10 > dynamic_states = {};
		dynamic_states.push( VK_DYNAMIC_STATE_SCISSOR );
		dynamic_states.push( VK_DYNAMIC_STATE_VIEWPORT );
		//dynamic_states.push( VK_DYNAMIC_STATE_LINE_WIDTH );
		//dynamic_states.push( VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK );
		//dynamic_states.push( VK_DYNAMIC_STATE_STENCIL_WRITE_MASK );
		//dynamic_states.push( VK_DYNAMIC_STATE_STENCIL_REFERENCE );
		VkPipelineDynamicStateCreateInfo dynamic_state = {};
		dynamic_state.dynamicStateCount = dynamic_states.size;
		dynamic_state.pDynamicStates = &dynamic_states[ 0 ];
		dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		VkPipelineViewportStateCreateInfo view_port_create_info;
		Allocator::zero( &view_port_create_info );
		view_port_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		view_port_create_info.scissorCount = 1;
		view_port_create_info.viewportCount = 1;
		VkPipelineRasterizationStateCreateInfo rasterizer_state_create_info;
		Allocator::zero( &rasterizer_state_create_info );
		rasterizer_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer_state_create_info.lineWidth = 1.0f;
		VkPipelineMultisampleStateCreateInfo multisample_state_create_info;
		Allocator::zero( &multisample_state_create_info );
		multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_state_create_info.minSampleShading = 1.0f;
		VkPipelineLayoutCreateInfo pipeline_layout_create_info;
		Allocator::zero( &pipeline_layout_create_info );
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.setLayoutCount = out.desc_set_layouts.size;
		pipeline_layout_create_info.pSetLayouts = &out.desc_set_layouts[ 0 ];
		pipeline_layout_create_info.pPushConstantRanges = &push_constants[ 0 ];
		pipeline_layout_create_info.pushConstantRangeCount = push_constants.size;
		out.pipeline_layout = vkNew( device , pipeline_layout_create_info );
		VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = {};
		pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
		pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
		pipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
		pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		pipelineDepthStencilStateCreateInfo.front = pipelineDepthStencilStateCreateInfo.back;
		pipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
		VkGraphicsPipelineCreateInfo pipeline_create_info;
		Allocator::zero( &pipeline_create_info );
		pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_create_info.stageCount = infos.size;
		pipeline_create_info.pStages = &infos[ 0 ];
		pipeline_create_info.pVertexInputState = &vertex_state_create_info;
		pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
		pipeline_create_info.pViewportState = &view_port_create_info;
		pipeline_create_info.pRasterizationState = &rasterizer_state_create_info;
		pipeline_create_info.pMultisampleState = &multisample_state_create_info;
		VkPipelineColorBlendStateCreateInfo blender_state;
		Allocator::zero( &blender_state );
		blender_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blender_state.attachmentCount = color_blend_state_info.size;
		blender_state.pAttachments = &color_blend_state_info[ 0 ];
		pipeline_create_info.pColorBlendState = &blender_state;
		pipeline_create_info.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
		pipeline_create_info.layout = out.pipeline_layout;
		pipeline_create_info.renderPass = renderpass;
		pipeline_create_info.subpass = subpass;
		pipeline_create_info.basePipelineIndex = -1;
		pipeline_create_info.pDynamicState = &dynamic_state;
		out.handle = vkNew( device , pipeline_create_info );
		return out;
	}
}