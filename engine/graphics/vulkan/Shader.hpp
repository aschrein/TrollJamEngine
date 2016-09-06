#pragma once
#include <engine/graphics/vulkan/defines.hpp>
namespace VK
{
	class Stage
	{
	private:
		Unique< VkShaderModule > hander;
		VkShaderStageFlagBits stage_flag;
	public:

	};
	struct ShaderConstant
	{
		uint location;
		uint binding;
		VkDescriptorType type;
	};
	class Shader
	{
	private:
		LocalArray< Stage , 5 > stages;
		LocalArray< ShaderConstant , 20 > constants;

		uint output_count;
	public:
		static Shader create( Device const &device , Graphics::Shader const &info )
		{

		}
	};
}