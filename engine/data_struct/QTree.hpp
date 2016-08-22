#pragma once
#include <engine/data_struct/Array.hpp>
#include <engine/mem/Allocators.hpp>
#include <engine/math/vec.hpp>
namespace Collections
{
	using namespace Math;
	template< typename T , typename Real >
	struct QTreeItem
	{
		typedef TVector< 4 , Real > vec4;
		T value;
		vec4 bound;
		bool operator==( QTreeItem const &item ) const
		{
			return value == item.value;
		}
	};
	template< typename T , typename Real >
	class QTreeNode
	{
	private:
		Allocator *allocator = nullptr;
		Array< QTreeItem< T , Real > > items;
		QTreeNode *children[ 4 ] = { nullptr };
		bool container = true;
		uint depth;
		Real size;
		typedef TVector< 2 , Real > vec2;
		typedef TVector< 4 , Real > vec4;
		vec2 pos;
	public:
		QTreeNode( TVector< 2 , Real > pos , Real size , uint depth , Allocator *allocator ) :
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
		bool collide( TVector< 4 , Real > const &bound )
		{
			return MathUtil< Real >::abs( bound.x - pos.x ) <= bound.z + size && MathUtil< Real >::abs( bound.y - pos.y ) <= bound.w + size;
		}
		static bool collide( vec4 const &a , vec4 const &b )
		{
			return MathUtil< Real >::abs( a.x - b.x ) <= a.z + b.z && MathUtil< Real >::abs( a.y - b.y ) <= a.w + b.w;
		}
		void remove( QTreeItem< T , Real > item )
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
						all_zero_size &&= !child->isEmpty();
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
		void push( QTreeItem< T , Real > item , uint MAX_ITEMS = 100 , uint MAX_DEPTH = 10 )
		{
			if( container )
			{
				items.push( item );
				if( items.getSize() > MAX_ITEMS && depth < MAX_DEPTH )
				{
					ito( 4 )
					{
						children[ i ] = allocator->alloc< QTreeNode >();
						int x = ( i & 1 ) * 2 - 1;
						int y = ( i & 2 ) - 1;
						Real new_size = size / Real( 2 );
						vec2 new_pos = pos + vec2( x * new_size , y * new_size );
						new( children[ i ] ) QTreeNode( new_pos , new_size , depth + 1 , allocator );
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
		void getCollided( vec4 const &bound , Container< T > &out )
		{
			if( container )
			{
				for( auto &item : items )
				{
					if( collide( item.bound , bound ) )
					{
						out.push( item.value );
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
	template< typename T , typename Real >
	class QTree
	{
	private:
		uint MAX_ITEMS = 100;
		uint MAX_DEPTH = 100;
		QTreeNode< T , Real > *root = nullptr;
		typedef TVector< 4 , Real > vec4;
		typedef TVector< 2 , Real > vec2;
	public:
		QTree( vec2 const &pos , Real size , Allocator *allocator = Allocator::singleton )
		{
			root = allocator->alloc< QTreeNode< T , Real > >();
			new( root ) QTreeNode< T , Real >( pos , size , 1 , allocator );
		}
		void push( T const &value , vec4 const &bound )
		{
			root->push( { value , bound } );
		}
		void remove( T const &value , vec4 const &bound )
		{
			root->remove( { value , bound } );
		}
		template< template< typename > typename Container >
		void getCollided( vec4 const &bound , Container< T > &out )
		{
			root->getCollided( bound , out );
		}
	};
}