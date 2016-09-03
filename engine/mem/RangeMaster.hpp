#pragma once
#include <engine/mem/Allocators.hpp>
#include <engine/data_struct/HashMap.hpp>
#include <engine/data_struct/BinaryTree.hpp>
#include <engine/os/log.hpp>
namespace Allocators
{
	using namespace Collections;
	class RangeManager
	{
	private:
		HashMap< uint , uint > free_ranges;
		HashMap< uint , uint > allocated_ranges;
		BinaryTree< uint > free_ranges_tree;
		uint upper_border = 0;
	public:
		RangeManager( uint upper_border , Allocator *allocator = Allocator::singleton ) :
			upper_border( upper_border )
		{
			free_ranges.setAllocator( allocator );
			allocated_ranges.setAllocator( allocator );
			free_ranges_tree.setAllocator( allocator );
			free_ranges_tree.pushUnique( 0 );
			free_ranges.push( 0 , upper_border );
		}
		RangeManager() = default;
		void setLimit( uint limit )
		{
			release();
			upper_border = limit;
			free_ranges_tree.pushUnique( 0 );
			free_ranges.push( 0 , upper_border );
		}
		void release()
		{
			free_ranges.release();
			allocated_ranges.release();
			free_ranges_tree.release();
		}
		void setAllocator( Allocator *allocator )
		{
			release();
			free_ranges.setAllocator( allocator );
			allocated_ranges.setAllocator( allocator );
			free_ranges_tree.setAllocator( allocator );
		}
		uint allocate( uint size )
		{
			for( auto &item : free_ranges )
			{
				auto range = item;
				if( range.value == size )
				{
					if( range.key + size > upper_border )
					{
						return UINT32_MAX;
					}
					free_ranges.remove( range.key );
					free_ranges_tree.remove( range.key );
					allocated_ranges.push( range.key , range.value );
					return range.key;
				} else if( range.value > size )
				{
					if( range.key + size > upper_border )
					{
						return UINT32_MAX;
					}
					free_ranges.remove( range.key );
					free_ranges_tree.remove( range.key );
					uint new_size = range.value - size;
					free_ranges.push( range.key + size , new_size );
					free_ranges_tree.pushUnique( range.key + size );
					allocated_ranges.push( range.key , size );
					return range.key;
				}
			}
			return UINT32_MAX;
		}
		void free( uint offset )
		{
			auto res = allocated_ranges.get( offset );
			if( res.isPresent() )
			{
				allocated_ranges.remove( offset );
				auto neighborhood = free_ranges_tree.getNeighborhood( offset );
				uint size = res.getValue();
				if( neighborhood.left.isPresent() )
				{
					uint _offset = neighborhood.left.getValue();
					offset = _offset;
					size += free_ranges.get( _offset ).getValue();
					free_ranges_tree.remove( _offset );
					free_ranges.remove( _offset );
				}
				if( neighborhood.right.isPresent() )
				{
					uint offset = neighborhood.right.getValue();
					size += free_ranges.get( offset ).getValue();
					free_ranges_tree.remove( offset );
					free_ranges.remove( offset );
				}
				free_ranges_tree.pushUnique( offset );
				free_ranges.push( offset , size );
			}
		}
		void print() const
		{
			OS::IO::debugLogln( "free:" );
			for( auto &range : free_ranges )
			{
				OS::IO::debugLogln( range.key , ":" , range.value );
			}
			OS::IO::debugLogln( "allocated:" );
			for( auto &range : allocated_ranges )
			{
				OS::IO::debugLogln( range.key , ":" , range.value );
			}
		}
	};
}