#pragma once
#include <engine/math\Math.hpp>
namespace Collections
{
	template< typename T >
	struct Hash
	{
		static int hashFunc( T const &val )
		{
			return val.hash();
		}
	};
	template<>
	struct Hash< int >
	{
		static int hashFunc( int const &val )
		{
			return Math::MathUtil< int >::abs( val );
		}
	};
	template<>
	struct Hash< size_t >
	{
		static int hashFunc( size_t const &val )
		{
			return Math::MathUtil< size_t >::abs( val );
		}
	};
	template< typename T >
	struct Hash< T * >
	{
		static int hashFunc( T * const &val )
		{
			return val->hash();
		}
	};
}