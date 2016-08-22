#pragma once
#include <stdlib.h>
#include <engine/util/defines.hpp>
namespace Allocators
{
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
			T * out = ( T* )alloc( sizeof( T ) * count , sizeof( T ) );
			return out;
		}
		template< typename T >
		T *allocArray( uint count )
		{
			void *ptr = alloc( count * sizeof( T ) + 4 , sizeof( T ) );
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
		static void copy( T *dst , T const *src , uint count )
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
			_aligned_free( base_ptr );
		}
		int getPosition() const
		{
			return cur_pos;
		}
		LinearAllocator() = default;
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
		struct MemRange
		{
			MemRange *next = nullptr;
			int offset = 0;
			int size = 0;
		};
	private:
		byte *base_ptr;
		int size;
		MemRange *root;
		MemRange *free_root;
		MemRange *free_tail;
		MemRange *captured_root = nullptr;
		MemRange *captured_tail = nullptr;
		uint range_counter = 1;
	public:
		PoolAllocator( uint pages , uint max_ranges ):
			size( pages * 4096 + max_ranges * sizeof( MemRange ) )
		{
			base_ptr = ( byte* )Allocator::singleton->alloc( size , 0x100 );
			root = free_tail = free_root = ( MemRange* )( base_ptr + pages * 4096 );
			free_root->next = nullptr;
			free_root->offset = 0;
			free_root->size = size;
		}
		void *alloc( size_t size , uint alignment = 4 ) override
		{
			MemRange *cur = free_root;
			MemRange *last = nullptr;
			uint mask = ~( ( ~alignment ) + 1 );
			size = size + ( alignment - ( size & mask ) );
			while( cur != nullptr )
			{
				if( cur->size > size )
				{
					MemRange *new_range = root + range_counter++;
					if( !new_range )
					{
						return nullptr;
					}
					new_range->next = cur->next;
					new_range->offset = cur->offset + size;
					new_range->size = cur->size - size;
					cur->size = size;
					cur->next = new_range;
					return cur->offset + base_ptr;
				} else if( cur->size == size )
				{
					last->next = cur->next;
					if( captured_tail == nullptr )
					{
						captured_tail = cur;
					} else
					{
						captured_tail->next = cur;
						captured_tail = cur;
					}
				}
				last = cur;
				cur = cur->next;
			}
			return nullptr;
		}
		void free( void *ptr ) override
		{
			int offset = ( byte* )ptr - base_ptr;
			MemRange *cur = root;
			MemRange *last = nullptr;
			while( cur != nullptr )
			{
				if( cur->offset == offset )
				{
					cur->free = true;
				}
				if( cur->next && cur->next->free )
				{
					auto tmp = cur->next;
					cur->next = tmp->next;
					cur->size += tmp->size;
					freeRangeStruct( tmp );
				}
				if( last && last->free )
				{
					auto tmp = cur;
					last->next = tmp->next;
					last->size += tmp->size;
					freeRangeStruct( tmp );
				}
				last = cur;
				cur = cur->next;
			}
		}
	};
	/*struct ThreadLocalAllocator
	{
		static __declspec( thread ) StaticAllocator *static_allocator;
		template< typename T >
		static T *alloc()
		{
			return alloc< T >( 1 );
		}
		template< typename T >
		static T *alloc( int count )
		{
			return ( T *)static_allocator->alloc( count * sizeof( T ) );
		}
		static void free( void *ptr )
		{
			static_allocator->free( ptr );
		}
	};*/
}