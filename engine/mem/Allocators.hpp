#pragma once
#include <stdlib.h>
#include <engine/util/defines.hpp>
#include <engine/math/Math.hpp>
#include <engine/os/log.hpp>
namespace Allocators
{
	using namespace Math;
	class Allocator
	{
	public:
		static Allocator *singleton;
	public:
		virtual void *alloc( size_t size , uint alignment = 4 )
		{
			auto out = _aligned_malloc( size , alignment );
			return out;
		}
		template< typename T >
		T *alloc( uint count = 1 )
		{
			T * out = ( T* )alloc( sizeof( T ) * count );
			return out;
		}
		template< typename T >
		T *allocArray( uint count )
		{
			void *ptr = alloc( count * sizeof( T ) + 4 );
			*( uint* )ptr = count;
			T *obj = ( T* )( ( byte* )ptr + 4 );
			for( uint i = 0; i < count; i++ )
			{
				new( obj + i ) T();
			}
			return obj;
		}
		template< typename T >
		void freeObj( T *ptr )
		{
			ptr->~T();
			free( ( void * )ptr );
		}
		template< typename T >
		void freeArray( T *ptr )
		{
			uint count = *( uint* )( ( byte* )ptr - 4 );
			for( uint i = 0; i < count; i++ )
			{
				ptr[ i ].~T();
			}
			free( ( void * )( ( byte* )ptr - 4 ) );
		}
		virtual void free( void *ptr )
		{
			::_aligned_free( ptr );
		}
		template< typename T >
		static void copy( T *dst , T const *src , uint count = 1 )
		{
			memcpy( dst , src , count * sizeof( T ) );
		}
		template< typename T >
		static void zero( T *dst , uint count = 1 )
		{
			memset( dst , 0 , count * sizeof( T ) );
		}
	};
	class LinearAllocator : public Allocator
	{
	private:
		byte *base_ptr = nullptr;
		size_t size = 0;
		size_t cur_pos = 0;
	public:
		~LinearAllocator()
		{
			if( base_ptr )
			{
				_aligned_free( base_ptr );
			}
			base_ptr = nullptr;
			size = 0;
			cur_pos = 0;
		}
		int getPosition() const
		{
			return cur_pos;
		}
		LinearAllocator() = default;
		LinearAllocator( LinearAllocator const & ) = delete;
		LinearAllocator &operator=( LinearAllocator const & ) = delete;
		LinearAllocator( LinearAllocator &&val )
		{
			*this = std::move( val );
		}
		LinearAllocator &operator=( LinearAllocator &&val )
		{
			this->~LinearAllocator();
			base_ptr = val.base_ptr;
			size = val.size;
			cur_pos = val.cur_pos;
			val.base_ptr = nullptr;
			val.~LinearAllocator();
			return *this;
		}
		LinearAllocator( int pages ) :
			size( pages * 4096 )
		{
			base_ptr = ( byte* )_aligned_malloc( size , 0x100 );
		}
		void *alloc( size_t size , uint alignment = 4 ) override
		{
			uint mask = ~( ( ~alignment ) + 1 );
			size_t ptr = ( size_t )( ( byte* )base_ptr + cur_pos );
			size_t offset = alignment - ( ptr & mask );
			ptr += offset;
			cur_pos += size + offset;
			return ( void * )ptr;
		}
		void free( void * ) override
		{

		}
		void reset()
		{
			cur_pos = 0;
		}
	};
	class PoolAllocator : public Allocator
	{
	private:
		byte *base_ptr = nullptr;
		int limit = 0;
		int first_free_range = 0;
		int occupied_space = 0;
		int* headToTail( int *head )
		{
			return ( int* )( size_t( head ) + MathUtil< int >::abs( *head ) + 4 );
		}
		int* headToNextHead( int *head )
		{
			return ( int* )( size_t( head ) + MathUtil< int >::abs( *head ) + 8 );
		}
		int* headToPrevTail( int *head )
		{
			return ( int* )( size_t( head ) - 4 );
		}
		int* tailToHead( int *tail )
		{
			return ( int* )( size_t( tail ) - MathUtil< int >::abs( *tail ) - 4 );
		}
	public:
		PoolAllocator( uint pages )
		{
			init( pages );
		}
		~PoolAllocator()
		{
			release();
		}
		void init( uint pages )
		{
			release();
			limit = pages * 0x1000;
			base_ptr = ( byte* )Allocator::singleton->alloc( pages * 0x1000 , 0x1000 );
			int *free_range_root = ( int * )base_ptr;
			*free_range_root = -limit + 8;
			int *free_range_tail = ( int * )( size_t( base_ptr ) + limit - 4 );
			*free_range_tail = -limit + 8;
			first_free_range = 0;
		}
		void release()
		{
			if( base_ptr )
			{
				Allocator::free( base_ptr );
			}
			base_ptr = nullptr;
			limit = 0;
			first_free_range = 0;
		}
		void *alloc( size_t size , uint alignment = 4 ) override
		{
			int *cur = ( int * )( base_ptr + first_free_range );
			bool first_range = true;
			int isize = -int( size + 4 - ( size & 3 ) );
			while( true )
			{
				uint offset = uint( size_t( cur ) - size_t( base_ptr ) );
				if( *cur <= isize && *cur > isize - 8 )
				{
					int *block_tail = headToTail( cur );
					*cur = -*cur;
					*block_tail = -*block_tail;
					if( first_range )
					{
						first_free_range = uint( size_t( block_tail ) + 4 - size_t( base_ptr ) );
					}
					occupied_space += -isize + 8;
					return ( void* )( size_t( cur ) + 4 );
				} else if( *cur < isize - 8 )
				{
					
					int old_size = *cur;
					int rest = old_size - isize + 8;
					*headToTail( cur ) = rest;
					*cur = -isize;
					auto tail = headToTail( cur );
					*tail = -isize;
					auto new_head = tail + 1;
					*new_head = rest;
					
					if( first_range )
					{
						first_free_range = uint( size_t( new_head ) - size_t( base_ptr ) );
					}
					occupied_space += -isize + 8;
					return ( void* )( size_t( cur ) + 4 );
				} else
				{
					first_range = false;
					cur = ( int* )( size_t( cur ) + Math::MathUtil< int >::abs( *cur ) + 8 );
				}
			}
			return nullptr;
		}
		void free( void *ptr ) override
		{
			int *cur_head = ( int * )( size_t( ptr ) - 4 );
			int *cur_tail = headToTail( cur_head );
			*cur_tail = -*cur_tail;
			*cur_head = -*cur_head;
			occupied_space += *cur_tail;
			if( size_t( cur_head ) != size_t( base_ptr ) )
			{
				int *last_tail = headToPrevTail( cur_head );
				if( *last_tail < 0 )
				{
					int *last_head = tailToHead( last_tail );
					int new_size = *last_head + *cur_head - 8;
					*last_head = new_size;
					*cur_tail = new_size;
					cur_head = last_head;
					occupied_space -= 8;
				}
			}
			if( size_t( cur_tail ) + 4 < size_t( base_ptr ) + limit )
			{
				int *next_head = headToNextHead( cur_head );
				if( *next_head < 0 )
				{
					int *next_tail = headToTail( next_head );
					int new_size = *next_head + *cur_head - 8;
					*cur_head = new_size;
					*next_tail = new_size;
					occupied_space -= 8;
				}
			}
			uint offset = uint( size_t( cur_head ) - size_t( base_ptr ) );
			
			if( offset < first_free_range )
			{
				first_free_range = offset;
			}
		}
	};
}