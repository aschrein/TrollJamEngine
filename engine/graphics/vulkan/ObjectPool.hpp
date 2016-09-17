#pragma once
#include <engine/data_struct/Array.hpp>
namespace VK
{
	using namespace Collections;
	template< typename T >
	class ObjectPool
	{
	public:
		Allocator *allocator;
		Array< T > objects;
		Array< uint > free_objects;
		uint counter = 0;
		uint push( T const &val )
		{
			auto tmp = val;
			return push( std::move( tmp ) );
		}
		uint push( T &&val )
		{
			if( !free_objects.isEmpty() )
			{
				uint index = free_objects.pop();
				objects[ index ] = std::move( val );
				return index;
			}
			objects.push( std::move( val ) );
			return objects.getSize() - 1;
		}
		T free( int i )
		{
			auto val = std::move( objects[ i ] );
			free_objects.push( i );
			return val;
		}
	};
}