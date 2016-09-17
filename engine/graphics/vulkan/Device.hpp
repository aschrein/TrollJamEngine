#pragma once
#include <engine/graphics/vulkan/defines.hpp>
#include <engine/graphics/vulkan/Unique.hpp>
#include <engine/graphics/vulkan/Instance.hpp>
#include <engine/graphics/vulkan/ObjectPool.hpp>
#include <engine/graphics/vulkan/Images.hpp>
#include <engine/graphics/vulkan/Pass.hpp>
#include <engine/graphics/vulkan/Pipeline.hpp>
#include <engine/graphics/vulkan/Buffers.hpp>

namespace VK
{
	struct ShaderModuleDesc
	{
		VkShaderModule handle;
		VkShaderStageFlagBits stage_flag;
	};
	class Device
	{
	public:
		VK_OBJECT( Device );
		VkDevice handle;
		uint32_t graphics_queue_count;
		Instance::PhysicalDevice pdev;
		LocalArray< VkQueue , 10 > graphics_queues , compute_queues;
		ObjectPool< VkSemaphore > semaphore_pool;
		ObjectPool< ShaderModuleDesc > shader_module_pool;
		ObjectPool< VkSampler > samplers_pool;
		ObjectPool< VkFramebuffer > frame_buffers_pool;
		ObjectPool< VkRenderPass > pass_pool;
		ObjectPool< Image > image_pool;
		ObjectPool< ImageView > image_view_pool;
		ObjectPool< Pipeline > pipelines_pool;
		ObjectPool< Memory > memory_pool;
		ObjectPool< Buffer > buffer_pool;
		Memory dev_mem;
		Memory dev_texture_mem;
		Memory host_mem;
	public:
		Memory &getMemory( MemoryType mem_type )
		{
			Memory *memory = nullptr;
			switch( mem_type )
			{
			case MemoryType::HOST:
				memory = &host_mem;
				break;
			case MemoryType::DEV_BUFFER:
				memory = &dev_mem;
				break;
			case MemoryType::DEV_TEXTURE:
				memory = &dev_texture_mem;
				break;
			}
			return *memory;
		}
		static Device createGraphicsDevice( Instance const &instance , uint32_t graphics_queue_count = 1 , uint compute_queue_count = 1 )
		{
			float priorities[ 10 ] = { 0.0f };
			VkDeviceQueueCreateInfo vkdevqueueinfo[ 2 ];
			Allocator::zero( vkdevqueueinfo , 2 );
			vkdevqueueinfo[ 0 ].queueFamilyIndex = instance.getPhysicalDevice().getGraphicsQueueFamily();
			vkdevqueueinfo[ 0 ].queueCount = graphics_queue_count;
			vkdevqueueinfo[ 0 ].pQueuePriorities = priorities;
			vkdevqueueinfo[ 1 ].queueFamilyIndex = instance.getPhysicalDevice().getComputeQueueFamily();
			vkdevqueueinfo[ 1 ].queueCount = compute_queue_count;
			vkdevqueueinfo[ 1 ].pQueuePriorities = priorities;
			VkDeviceCreateInfo vkdevinfo;
			Allocator::zero( &vkdevinfo );
			vkdevinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			vkdevinfo.queueCreateInfoCount = 2;
			vkdevinfo.pQueueCreateInfos = vkdevqueueinfo;
			Device out;
			out.pdev = instance.getPhysicalDevice();
			out.handle = Factory< VkDevice >::create( instance.getPhysicalDevice().getHandle() , vkdevinfo );
			ito( graphics_queue_count )
			{
				out.graphics_queues.push( out.getGraphicsQueue( i ) );
			}
			ito( compute_queue_count )
			{
				out.compute_queues.push( out.getComputeQueue( i ) );
			}
			VkMemoryAllocateInfo alloc_info;
			Allocator::zero( &alloc_info );
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.allocationSize = 256 * 100000;
			alloc_info.memoryTypeIndex = instance.getPhysicalDevice().getMemoryIndex( 1665 , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );
			out.host_mem.handle = Factory< VkDeviceMemory >::create( out.handle , alloc_info );
			out.host_mem.size = alloc_info.allocationSize;
			out.host_mem.rm.setLimit( alloc_info.allocationSize );

			alloc_info.allocationSize = 256 * 100000;
			alloc_info.memoryTypeIndex = instance.getPhysicalDevice().getMemoryIndex( 1665 , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
			out.dev_mem.handle = Factory< VkDeviceMemory >::create( out.handle , alloc_info );
			out.dev_mem.size = alloc_info.allocationSize;
			out.dev_mem.rm.setLimit( alloc_info.allocationSize );

			alloc_info.allocationSize = 256 * 100000;
			alloc_info.memoryTypeIndex = instance.getPhysicalDevice().getMemoryIndex( 258 , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
			out.dev_texture_mem.handle = Factory< VkDeviceMemory >::create( out.handle , alloc_info );
			out.dev_texture_mem.size = alloc_info.allocationSize;
			out.dev_texture_mem.rm.setLimit( alloc_info.allocationSize );

			out.graphics_queue_count = graphics_queue_count;
			return out;
		}
		Instance::PhysicalDevice const &getPhysicalDevice() const
		{
			return pdev;
		}
		Pair< uint , VkSemaphore > createSemaphore()
		{
			VkSemaphoreCreateInfo semaphore_create_info;
			Allocator::zero( &semaphore_create_info );
			semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphore_create_info.pNext = NULL;
			semaphore_create_info.flags = 0;
			auto out = Factory< VkSemaphore >::create( getHandle() , semaphore_create_info );
			return{ semaphore_pool.push( out ) , out };
		}
		Pair< uint , ShaderModuleDesc > createShaderModule( void const *data , uint size , VkShaderStageFlagBits stage_flag )
		{
			VkShaderModuleCreateInfo shader_module_create_info;
			Allocator::zero( &shader_module_create_info );
			shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shader_module_create_info.codeSize = size;
			shader_module_create_info.pCode = ( const uint32_t* )data;
			ShaderModuleDesc out = { Factory< VkShaderModule >::create( getHandle() , shader_module_create_info ) , stage_flag };
			return{ shader_module_pool.push( out ) , out };
		}
		Pair< uint , VkSampler > createSampler( Graphics::SamplerCreateInfo const &info )
		{
			VkSamplerCreateInfo sampler;
			Allocator::zero( &sampler );
			sampler.magFilter = getVK( info.mag_filter );
			sampler.minFilter = getVK( info.min_filter );
			sampler.mipmapMode = info.use_mipmap ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
			sampler.addressModeU = getVK( info.u_regime );
			sampler.addressModeV = getVK( info.v_regime );
			sampler.addressModeW = getVK( info.w_regime );
			sampler.compareOp = VK_COMPARE_OP_NEVER;
			sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			auto out = Factory< VkSampler >::create( getHandle() , sampler );
			return{ samplers_pool.push( out ) , out };
		}
		Pair< uint , VkFramebuffer > createFrameBuffer(
			VkRenderPass render_pass ,
			LocalArray< VkImageView , 5 > attachments_views , uint2 size
		)
		{
			VkFramebufferCreateInfo frame_buffer_create_info;
			Allocator::zero( &frame_buffer_create_info );
			frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frame_buffer_create_info.renderPass = render_pass;
			frame_buffer_create_info.attachmentCount = attachments_views.size;
			frame_buffer_create_info.pAttachments = &attachments_views[ 0 ];
			frame_buffer_create_info.width = size.x;
			frame_buffer_create_info.height = size.y;
			frame_buffer_create_info.layers = 1;
			auto out = Factory< VkFramebuffer >::create( getHandle() , frame_buffer_create_info );
			return{ frame_buffers_pool.push( out  ) , out };
		}
		Pair< uint , Image > createImage2D(
			uint width , uint height ,
			uint mip_levels , uint layers , VkImageLayout initial_layout , VkFormat format ,
			VkImageUsageFlags usage_flags
		)
		{
			VkImageCreateInfo image_info;
			Allocator::zero( &image_info );
			image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image_info.pNext = NULL;
			image_info.imageType = VK_IMAGE_TYPE_2D;
			image_info.format = format;
			image_info.extent = { width, height, 1 };
			image_info.mipLevels = mip_levels;
			image_info.arrayLayers = layers;
			image_info.samples = VK_SAMPLE_COUNT_1_BIT;
			image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
			image_info.usage = usage_flags;
			image_info.flags = 0;
			image_info.initialLayout = initial_layout;

			VkMemoryRequirements mem_req;

			VkResult err;
			Image out;
			out.handle = Factory< VkImage >::create( getHandle() , image_info );
			vkGetImageMemoryRequirements( getHandle() , out.handle , &mem_req );
			out.depth = 1;
			out.format = format;
			out.height = height;
			out.width = width;
			out.layout = initial_layout;
			out.layers = layers;
			out.mip_levels = mip_levels;
			out.memory_offset = dev_texture_mem.allocate( mem_req.size );
			out.memory_size = mem_req.size;
			out.usage = usage_flags;
			vkBindImageMemory( getHandle() , out.handle , dev_texture_mem.handle , out.memory_offset );
			return{ image_pool.push( out ) , out };
		}
		Pair< uint , ImageView > createView2D( Image const &image ,
			VkComponentMapping mapping = { VK_COMPONENT_SWIZZLE_R , VK_COMPONENT_SWIZZLE_G , VK_COMPONENT_SWIZZLE_B , VK_COMPONENT_SWIZZLE_A } ,
			VkImageSubresourceRange subresource_range = { VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } )
		{
			VkImageViewCreateInfo image_view_info;
			Allocator::zero( &image_view_info );
			image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_info.format = image.format;
			image_view_info.components = mapping;
			image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			image_view_info.subresourceRange = subresource_range;
			image_view_info.image = image.handle;
			ImageView image_view;
			image_view.handle = Factory< VkImageView >::create( getHandle() , image_view_info );
			image_view.range = image_view_info.subresourceRange;
			image_view.format = image.format;
			image_view.mapping = mapping;
			return{ image_view_pool.push( image_view ) , image_view };
		}
		Pair< uint , VkRenderPass > createGraphicsRenderPass( PassInfo const &info )
		{
			LocalArray< VkSubpassDescription , 10 > subpasses = {};
			for( auto &sinfo : info.subpasses )
			{
				VkSubpassDescription subpass_desc;
				Allocator::zero( &subpass_desc );
				subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subpass_desc.colorAttachmentCount = sinfo.size;
				subpass_desc.pColorAttachments = &sinfo[ 0 ];
				subpasses.push( subpass_desc );
			}
			
			VkRenderPassCreateInfo render_pass_info;
			Allocator::zero( &render_pass_info );
			render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			render_pass_info.attachmentCount = info.attachments.size;
			render_pass_info.pAttachments = &info.attachments[ 0 ];
			render_pass_info.subpassCount = subpasses.size;
			render_pass_info.pSubpasses = &subpasses[ 0 ];

			auto out = Factory< VkRenderPass >::create( getHandle() , render_pass_info );
			return{ pass_pool.push( out ) , out };
		}

		Pair< uint , Pipeline > createPipeline(
			VkRenderPass renderpass , uint subpass ,
			LocalArray< ShaderModuleDesc , 5 > const &stages ,
			LocalArray< DescriptorSetInfo , 5 > const &desc_info ,
			LocalArray< VkPushConstantRange , 5 > const &push_constants ,
			LocalArray< VkVertexInputBindingDescription , 10 > attrib_binding_info ,
			LocalArray< VkVertexInputAttributeDescription , 10 > attrib_info ,
			VkPrimitiveTopology topology ,
			VkPipelineColorBlendStateCreateInfo color_blend_state_info
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
				out.desc_pool = Factory< VkDescriptorPool >::create( getHandle() , desc_pool_info );
				for( auto const &di : desc_info )
				{
					VkDescriptorSetLayoutCreateInfo desc_set_layout_info;
					Allocator::zero( &desc_set_layout_info );
					desc_set_layout_info.bindingCount = di.bindings.size;
					desc_set_layout_info.pBindings = &di.bindings[ 0 ];
					desc_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
					auto layout = Factory< VkDescriptorSetLayout >::create( getHandle() , desc_set_layout_info );
					VkDescriptorSetAllocateInfo desc_set_info = {};
					desc_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
					desc_set_info.descriptorPool = out.desc_pool;
					desc_set_info.descriptorSetCount = 1;
					desc_set_info.pSetLayouts = &layout;
					auto desc_set = Factory< VkDescriptorSet >::create( getHandle() , out.desc_pool , desc_set_info );
					out.desc_sets.push( std::move( desc_set ) );
					out.desc_set_layouts.push( std::move( layout ) );
				}
			}
			LocalArray< VkPipelineShaderStageCreateInfo , 5 > infos = {};
			for( auto const &stage : stages )
			{
				//auto &stage = shader_module_pool.objects[ stage_index ];
				infos.push(
				{
					VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO ,
					nullptr ,
					0 ,
					stage.stage_flag ,
					stage.handle ,
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
			out.pipeline_layout = Factory< VkPipelineLayout >::create( getHandle() , pipeline_layout_create_info );
			VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = {};
			pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
			pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
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
			pipeline_create_info.pColorBlendState = &color_blend_state_info;
			pipeline_create_info.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
			pipeline_create_info.layout = out.pipeline_layout;
			pipeline_create_info.renderPass = renderpass;// pass_pool.objects[ renderpass ];
			pipeline_create_info.subpass = subpass;
			pipeline_create_info.basePipelineIndex = -1;
			pipeline_create_info.pDynamicState = &dynamic_state;
			out.handle = Factory< VkPipeline >::create( getHandle() , pipeline_create_info );
			return{ pipelines_pool.push( out ) , out };
		}
		VkQueue getGraphicsQueue( uint index = 0 ) const
		{
			VkQueue queue;
			vkGetDeviceQueue( handle , pdev.getGraphicsQueueFamily() , index , &queue );
			return queue;
		}
		VkQueue getComputeQueue( uint index = 0 ) const
		{
			VkQueue queue;
			vkGetDeviceQueue( handle , pdev.getComputeQueueFamily() , index , &queue );
			return queue;
		}
		Pair< uint , Buffer > createBuffer( VkBufferUsageFlags usage , MemoryType mem_type , uint size )
		{
			Buffer out;
			VkBufferCreateInfo info;
			Allocator::zero( &info );
			info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			info.size = size;
			info.usage = usage;
			out.handle = Factory< VkBuffer >::create( handle , info );
			VkMemoryRequirements mem_req;
			vkGetBufferMemoryRequirements( handle , out.handle , &mem_req );
			out.size = size;
			out.usage = usage;
			out.mem_type = mem_type;
			
			out.offset = getMemory( mem_type ).allocate( mem_req.size );
			vkBindBufferMemory( handle , out.handle , getMemory( mem_type ).handle , out.offset );
			return{ buffer_pool.push( out ) , out };
		}
		void releaseBuffer( uint handle )
		{
			auto buffer = buffer_pool.free( handle );
			auto &mem = getMemory( buffer.mem_type );
			mem.free( buffer.offset );
			VK::Factory< VkBuffer >::release( getHandle() , buffer.handle );
		}
		void *map( Buffer const &buffer )
		{
			void *data;
			vkMapMemory( handle , getMemory( buffer.mem_type ).handle , buffer.offset , buffer.size , 0 , &data );
			return data;
		}
		void unmap( Buffer const &buffer )
		{
			vkUnmapMemory( handle , getMemory( buffer.mem_type ).handle );
		}
		void submitGraphicsCommandBuffer( uint queue_index , CommandBuffer const &command_buffer , VkSemaphore wait_semaphore = VK_NULL_HANDLE , VkSemaphore signal_semaphore = VK_NULL_HANDLE ) const
		{
			VkSubmitInfo submit_info;
			Allocator::zero( &submit_info );
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.pNext = NULL;
			VkPipelineStageFlags pipeline_stage_flag = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			submit_info.pWaitDstStageMask = &pipeline_stage_flag;

			if( signal_semaphore )
			{
				submit_info.pSignalSemaphores = &signal_semaphore;
				submit_info.signalSemaphoreCount = 1;
			}
			if( wait_semaphore )
			{
				submit_info.pWaitSemaphores = &wait_semaphore;
				submit_info.waitSemaphoreCount = 1;
			}
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = &command_buffer.getHandle();
			VKASSERTLOG( vkQueueSubmit( graphics_queues[ queue_index ] , 1 , &submit_info , VK_NULL_HANDLE ) );
		}
		void present( uint queue_index , VkSwapchainKHR swap_chain , uint image_index , VkSemaphore wait_semaphore = VK_NULL_HANDLE ) const
		{
			VkPresentInfoKHR present_info;
			Allocator::zero( &present_info );
			present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			if( wait_semaphore )
			{
				present_info.waitSemaphoreCount = 1;
				present_info.pWaitSemaphores = &wait_semaphore;
			}
			present_info.pSwapchains = &swap_chain;
			present_info.swapchainCount = 1;
			present_info.pImageIndices = &image_index;
			VKASSERTLOG( vkQueuePresentKHR( graphics_queues[ queue_index ] , &present_info ) );
		}
		void waitIdleGraphics( uint queue_index ) const
		{
			vkQueueWaitIdle( graphics_queues[ queue_index ] );
		}
		VkDevice getHandle() const
		{
			return handle;
		}
	};
}