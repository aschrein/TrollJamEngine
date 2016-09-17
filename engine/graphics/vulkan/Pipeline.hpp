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
}