#pragma once
#include <engine/math\Math.hpp>
namespace Collections
{
	template< typename T >
	struct Hash
	{
		static uint hashFunc( T const &val )
		{
			return val.hash();
		}
	};
	template<>
	struct Hash< int >
	{
		static uint hashFunc( int const &val )
		{
			return Math::MathUtil< int >::abs( val );
		}
	};
	template<>
	struct Hash< uint >
	{
		static uint hashFunc( uint const &val )
		{
			return val;
		}
	};
	template<>
	struct Hash< size_t >
	{
		static uint hashFunc( size_t const &val )
		{
			return Math::MathUtil< size_t >::abs( val );
		}
	};
	template< typename T >
	struct Hash< T * >
	{
		static uint hashFunc( T * const &val )
		{
			return val->hash();
		}
	};
}