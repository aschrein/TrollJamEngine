#pragma once
#include <engine/data_struct/Array.hpp>
#include <engine/graphics/vulkan/Images.hpp>
#include <engine/graphics/vulkan/Device.hpp>
#include <engine/graphics/vulkan/Buffers.hpp>
namespace VK
{
	using namespace Collections;
	class ObjectPool
	{
	public:
		Allocator *allocator;
		Array< Image > textures;
		uint texture_counter = 0;
		Array< ImageView > views;
		uint view_counter = 0;
		Array< Buffer > buffers;
		uint buffer_counter = 0;
		Array< Attachment > attachments;
		uint attachment_counter = 0;
		Array< Unique< VkShaderModule > > shader_modules;
		Array< Unique< VkSampler > > samplers;
		uint sampler_counter = 0;
	};
}