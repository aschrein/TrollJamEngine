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
		Array< Image > images;
		uint image_counter = 0;
		Array< ImageView > views;
		uint view_counter = 0;
		Array< Buffer > buffers;
		uint buffer_counter = 0;
		Array< Attachment > attachments;
		uint attachment_counter = 0;
		Array< Unique< VkShaderModule > > shader_modules;
	};
}