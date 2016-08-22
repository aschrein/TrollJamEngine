#pragma once
#include <functional>
#include <iostream>
#include <engine/data_struct\Hash.hpp>
#include <engine/data_struct\BinaryTree.hpp>
#include <engine/util/defines.hpp>
namespace Collections
{
	template< typename T >
	class HashSet
	{
	public:
		typedef BinaryTree< T > Chunk;
	protected:
		uint MAX_ITEMS_PER_CHUNK = 10;
		uint INCREMENT = 10;
		Allocator *allocator = Allocator::singleton;
		Chunk *chunks = nullptr;
		uint chunks_count = 0;
		uint getChunkId( T const &val ) const
		{
			uint hash = Hash< T >::hashFunc( val );
			return hash % chunks_count;
		}
		void init( uint initial_chunks )
		{
			release();
			chunks_count = initial_chunks;
			chunks = ( Chunk* )allocator->alloc( chunks_count * sizeof( Chunk ) );
			ito( chunks_count )
			{
				new( chunks + i ) Chunk( allocator );
			}
		}
	public:
		void setMaxItemsPerChunk( uint mipc )
		{
			MAX_ITEMS_PER_CHUNK = mipc;
		}
		void setIncrement( uint inc )
		{
			INCREMENT = inc;
		}
		HashSet( uint initial_chunks , Allocator *allocator ) :
			allocator( allocator )
		{
			init( initial_chunks );
		}
		HashSet( int initial_chunks )
		{
			init( initial_chunks );
		}
		HashSet() = default;
		HashSet( HashSet const &set )
		{
			*this = set;
		}
		HashSet &operator=( HashSet const &set )
		{
			release();
			for( auto const &item : set )
			{
				push( item );
			}
		}
		HashSet( HashSet &&set )
		{
			*this = set;
		}
		HashSet &operator=( HashSet &&set )
		{
			release();
			chunks = set.chunks;
			chunks_count = set.chunks_count;
			allocator = set.allocator;
			set.chunks = nullptr;
			set.release();
			return *this;
		}
		void release()
		{
			if( chunks )
			{
				ito( chunks_count )
				{
					( chunks + i )->~Chunk();
				}
				allocator->free( chunks );
			}
			chunks = nullptr;
			chunks_count = 0;
		}
		bool isEmpty() const
		{
			return chunks == nullptr;
		}
		~HashSet()
		{
			release();
		}
		HashSet &push( T const &arg )
		{
			T tmp = arg;
			return push( std::move( tmp ) );
		}
		HashSet &push( T &&arg )
		{
			if( !chunks )
			{
				init( INCREMENT );
			}
			int chunk_id = getChunkId( arg );
			chunks[ chunk_id ].pushUnique( std::move( arg ) );
			if( chunks[ chunk_id ].getSize() == MAX_ITEMS_PER_CHUNK )
			{
				HashSet tmp( chunks_count + INCREMENT );
				for( auto &&item : *this )
				{
					tmp.push( std::move( item ) );
				}
				*this = std::move( tmp );
			}
			return *this;
		}
		void remove( T const &arg )
		{
			int chunk_id = getChunkId( arg );
			chunks[ chunk_id ].remove( arg );
			int maxc = 1000000;
			ito( chunks_count )
			{
				maxc = Math::MathUtil< int >::max( maxc , chunks[ i ].getSize() );
			}
			if( maxc < 2 && chunks_count > INCREMENT )
			{
				HashSet tmp( chunks_count - INCREMENT );
				for( auto &&item : *this )
				{
					tmp.push( std::move( item ) );
				}
				*this = std::move( tmp );
			}
		}
		int getItemsCount() const
		{
			int count = 0;
			ito( chunks_count )
			{
				count += chunks[ i ].getSize();
			}
			return count;
		}
		bool contains( T const &arg ) const
		{
			if( !chunks )
			{
				return false;
			}
			int chunk_id = getChunkId( arg );
			return chunks[ chunk_id ].contains( arg );
		}
		struct HashSetIterator
		{
			HashSet const *set;
			TBinaryTreeItem< T > *cur_item;
			uint cur_chunk;
			T &operator*()
			{
				return cur_item->value;
			}
			T *operator->()
			{
				return &cur_item->value;
			}
			T const &operator*() const
			{
				return cur_item->value;
			}
			T const *operator->() const
			{
				return &cur_item->value;
			}
			void step()
			{
				if( cur_item != nullptr )
				{
					cur_item = cur_item->next;
					if( cur_item == nullptr && cur_chunk < set->chunks_count - 1 )
					{
						for( int i = cur_chunk + 1; i < set->chunks_count; i++ )
						{
							if( !set->chunks[ i ].isEmpty() )
							{
								cur_chunk = i;
								cur_item = set->chunks[ i ].getRoot()->getMostLeft();
								return;
							}
						}
					}
				}
			}
			HashSetIterator operator++( int )
			{
				step();
				return *this;
			}
			HashSetIterator operator++()
			{
				HashSetIterator out = *this;
				step();
				return out;
			}
			bool operator==( HashSetIterator const &a ) const
			{
				return cur_item == a.cur_item;
			}
			bool operator!=( HashSetIterator const &a ) const
			{
				return cur_item != a.cur_item;
			}
		};
		HashSetIterator begin()
		{
			ito( chunks_count )
			{
				if( !chunks[ i ].isEmpty() )
				{
					return HashSetIterator{ this , chunks[ i ].getRoot()->getMostLeft() , i };
				}
			}
			return HashSetIterator{ this , nullptr , chunks_count };
		}
		HashSetIterator end()
		{
			return HashSetIterator{ this , nullptr , chunks_count };
		}
		HashSetIterator begin() const
		{
			ito( chunks_count )
			{
				if( !chunks[ i ].isEmpty() )
				{
					return HashSetIterator{ this , chunks[ i ].getRoot()->getMostLeft() , i };
				}
			}
			return HashSetIterator{ this , nullptr , chunks_count };
		}
		HashSetIterator end() const
		{
			return HashSetIterator{ this , nullptr , chunks_count };
		}
		void print() const
		{
			for( auto const &item : *this )
			{
				OS::IO::debugLog( item , "->" );
			}
			OS::IO::debugLogln( "null" );
		}
	};
}
