#include <stdafx.h>
#include <engine/graphics/ShaderBuilder.hpp>
namespace Collections
{
	template<>
	static void StringUtil::stringify< Graphics::Shaders::Expr >( char *mem , Graphics::Shaders::Expr const &val )
	{
		auto repr = val.getRepr();
		Allocator::copy( mem , repr.getChars() , repr.getLength() + 1 );
	}
	template<>
	static void StringUtil::stringify< Graphics::Shaders::Token >( char *mem , Graphics::Shaders::Token const &val )
	{
		auto repr = val.getRepr();
		Allocator::copy( mem , repr.getChars() , repr.getLength() + 1 );
	}
}