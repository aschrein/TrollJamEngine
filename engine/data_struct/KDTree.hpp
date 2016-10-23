#pragma once
#include <engine/data_struct/Array.hpp>
#include <engine/data_struct/HashSet.hpp>
#include <engine/mem/Allocators.hpp>
#include <engine/math/vec.hpp>
namespace Collections
{
	using namespace Math;
	template< typename T , typename Real , int K >
	struct KDTreeItem
	{
		typedef TVector< K * 2 , Real > Bound;
		T value;
		Bound bound;
		uint hash() const
		{
			return Hash< T >::hashFunc( value );
		}
		bool operator<( KDTreeItem const &item ) const
		{
			return value < item.value;
		}
		bool operator==( KDTreeItem const &item ) const
		{
			return value == item.value;
		}
	};
	template< typename T , typename Real , int K >
	class KDTreeNode
	{
	private:
		Allocator *allocator = nullptr;
		Array< KDTreeItem< T , Real , K > > items;
		KDTreeNode *children[ 1 << K ] = { nullptr };
		bool container = true;
		uint depth;
		Real size;
		typedef TVector< K , Real > Pos;
		typedef TVector< K * 2 , Real > Bound;
		Pos pos;
	public:
		KDTreeNode( Pos pos , Real size , uint depth , Allocator *allocator ) :
			allocator( allocator ) ,
			pos( pos ) ,
			size( size ) ,
			depth( depth )
		{
			items.setAllocator( allocator );
		}
		uint getItemsCount() const
		{
			return items.getSize();
		}
		bool isEmpty() const
		{
			if( isContainer() )
			{
				return getItemsCount() == 0u;
			} else
			{
				for( auto &child : children )
				{
					if( !child->isEmpty() )
					{
						return false;
					}
					return true;
				}
			}
		}
		bool isContainer() const
		{
			return container;
		}
		bool collide( Bound const &bound )
		{
			ito( K )
			{
				if( MathUtil< Real >::abs( bound[ i ] - pos[ i ] ) > bound[ K + i ] + size )
				{
					return false;
				}
			}
			return true;
		}
		static bool collide( Bound const &a , Bound const &b )
		{
			ito( K )
			{
				if( MathUtil< Real >::abs( a[ i ] - b[ i ] ) > a[ K + i ] + b[ K + i ] )
				{
					return false;
				}
			}
			return true;
		}
		void remove( KDTreeItem< T , Real , K > item )
		{
			if( container )
			{
				items.removeFirst( item );
			} else
			{
				bool all_zero_size = true;
				for( auto &child : children )
				{
					if( child->collide( item.bound ) )
					{
						child->remove( item );
						all_zero_size && = !child->isEmpty();
					}
				}
				if( all_zero_size )
				{
					releaseChildren();
				}
			}
		}
		void release()
		{
			items.release();
			releaseChildren();
		}
		void releaseChildren()
		{
			for( auto &child : children )
			{
				child->release();
				allocator->free( child );
				child = nullptr;
			}
			container = true;
		}
		void push( KDTreeItem< T , Real , K > item , uint MAX_ITEMS = 100 , uint MAX_DEPTH = 10 )
		{
			if( container )
			{
				items.push( item );
				if( items.getSize() > MAX_ITEMS && depth < MAX_DEPTH )
				{
					ito( 1 << K )
					{
						children[ i ] = allocator->alloc< KDTreeNode >();
						Pos new_pos;
						jto( K )
						{
							new_pos[ j ] = ( int( i >> j ) & 1 ) * 2 - 1;
						}
						Real new_size = size / Real( 2 );
						new_pos = pos + new_pos * new_size;
						new( children[ i ] ) KDTreeNode( new_pos , new_size , depth + 1 , allocator );
					}
					for( auto const &item : items )
					{
						for( auto &child : children )
						{
							if( child->collide( item.bound ) )
							{
								child->push( item , MAX_ITEMS , MAX_DEPTH );
							}
						}
					}
					items.release();
					container = false;
				}
			} else
			{
				for( auto &child : children )
				{
					if( child->collide( item.bound ) )
					{
						child->push( item , MAX_ITEMS , MAX_DEPTH );
					}
				}
			}
		}
		template< template< typename > typename Container >
		void getCollided( Bound const &bound , Container< KDTreeItem< T , Real , K > > &out )
		{
			if( container )
			{
				for( auto &item : items )
				{
					if( collide( item.bound , bound ) )
					{
						out.push( item );
					}
				}
			} else
			{
				for( auto &child : children )
				{
					if( child->collide( bound ) )
					{
						child->getCollided( bound , out );
					}
				}
			}
		}
	};
	template< typename T , typename Real , int K  >
	class KDTree
	{
	private:
		uint MAX_ITEMS = 20;
		uint MAX_DEPTH = 100;
		KDTreeNode< T , Real , K > *root = nullptr;
		typedef TVector< K * 2 , Real > Bound;
		typedef TVector< K , Real > Pos;
	public:
		KDTree( Pos const &pos , Real size , Allocator *allocator = Allocator::singleton )
		{
			root = allocator->alloc< KDTreeNode< T , Real , K > >();
			new( root ) KDTreeNode< T , Real , K >( pos , size , 1 , allocator );
		}
		void push( T const &value , Bound const &bound )
		{
			root->push( { value , bound } );
		}
		void remove( T const &value , Bound const &bound )
		{
			root->remove( { value , bound } );
		}
		HashSet< KDTreeItem< T , Real , K > > getCollided( Bound const &bound , Allocator *allocator = Allocator::singleton )
		{
			HashSet< KDTreeItem< T , Real , K > > out;
			out.setAllocator( allocator );
			root->getCollided( bound , out );
			return out;
		}
	};
}