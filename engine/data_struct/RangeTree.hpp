#pragma once
#include <engine/data_struct/Array.hpp>
#include <engine/mem/Allocators.hpp>
#include <engine/math/vec.hpp>
#include <engine/mem/Pointers.hpp>
namespace Collections
{
	using namespace Math;
	using namespace Pointers;
	template< typename Real >
	class RangeTreeItem
	{
	private:
		Allocator *allocator = nullptr;
		Array< Unique< RangeTreeItem > > children;
		Real origin , end;
	public:
		RangeTreeItem( Real origin , Real end , Allocator *allocator ) :
			origin( origin ) ,
			end( end )
		{
			children.setAllocator( allocator );
		}
		bool contains( Real o , Real e ) const
		{
			return o >= origin && e <= end;
		}
		bool collide( Real o , Real e ) const
		{
			return o >= origin && o <= end || e <= end && e >= origin;
		}
	};
	template< typename Real >
	class RangeTree
	{

	};
}