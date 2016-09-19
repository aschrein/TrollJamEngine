#pragma once
#include <engine/util/defines.hpp>
#include <engine/math/vec.hpp>
#include <engine/math/mat.hpp>
#include <engine/graphics/Textures.h>
#include <engine/data_struct/String.hpp>
#include <engine/data_struct/Array.hpp>
#include <engine/data_struct/Tuple.hpp>
#include <engine/data_struct/Buffer.hpp>
namespace Graphics
{
	using namespace Math;
	using namespace Collections;
	enum class AttributeSlot
	{
		POSITION , COLOR , NORMAL , TANGENT , BINORMAL , TEXCOORD0 , TEXCOORD1 , TEXCOORD2 , TEXCOORD3 , BONEWEIGTS , BONEINDICES
	};
	struct AttributeInfo
	{
		uint offset;
		AttributeSlot slot;
		ComponentMapping component_mapping;
	};
	enum class TextureSlot
	{
		ALBEDO , NORMAL , METALNESS , ROUGHNESS
	};
	enum class ConstantType
	{
		WORLD_POS , TIME , FLOAT , INT
	};
	enum class NodeType
	{
		FUNC , ATTRIBUTE , TEXTURE , CONSTANT
	};
	struct TextureNode
	{
		TextureSlot slot;
		Pair< uint , NodeType > in;
	};
	struct ConstantNode
	{
		ConstantType type;
		union
		{
			float fval;
			int ival;
			float4 f4val;
			f4x4 f4x4val;
		};
	};
	enum class FuncNodeType
	{
		SIN , COS , SATURATE , MUL , DIV , ADD , SUB , DOT , CROSS
	};
	struct FuncNode
	{
		LocalArray< Pair< uint , NodeType > , 4 > argv;
		FuncNodeType type;
	};
	class MaterialCreateInfo
	{
	private:
		Array< AttributeSlot > attributes;
		Array< ConstantNode > constants;
		Array< TextureNode > out_textures;
	public:
	};
}