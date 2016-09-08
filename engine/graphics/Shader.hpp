#pragma once
#include <engine/data_struct/String.hpp>
#include <engine/data_struct/Array.hpp>
#include <engine/data_struct/HashMap.hpp>
#include <engine/graphics/ShaderBuilder.hpp>
#undef VOID
#undef CONST
namespace Graphics
{
	enum class StageType
	{
		VERTEX , FRAGMENT , GEOMETRY , TESSELATION_CONTROL , TESSELATION_EVAL
	};
	using namespace Collections;
	struct ShaderCreateInfo
	{
		String name;
		Array< Pair< StageType , Shaders::Stage > > stages;
		~ShaderCreateInfo() = default;
	};
}