#pragma once
#include <stdlib.h>
#include <engine/util/defines.hpp>
namespace Allocators
{
	class Allocator
	{
	public:
		static Allocator *singleton;
		//static Collections::HashSet< size_t > allocated_pointers;
	public:
		virtual void *alloc( size_t size , int alignment = 4 )
		{
			auto out = _aligned_malloc( size , alignment );
			//allocated_pointers.push( ( size_t )out );
			return out;
		}
		template< typename T >
		T *alloc( uint count = 1 )
		{
			T * out = ( T* )alloc( sizeof( T ) * count );
			return out;
		}
		template< typename T >
		T *allocArray( int count )
		{
			void *ptr = alloc( count * sizeof( T ) + 4 );
			*( int* )ptr = count;
			T *obj = ( T* )( ( byte* )ptr + 4 );
			for( int i = 0; i < count; i++ )
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
			int count = *( int* )( ( byte* )ptr - 4 );
			for( int i = 0; i < count; i++ )
			{
				ptr[ i ].~T();
			}
			free( ( void * )( ( byte* )ptr - 4 ) );
		}
		virtual void free( void *ptr )
		{
			//allocated_pointers.remove( ( size_t )ptr );
			::_aligned_free( ptr );
		}
		template< typename T >
		static void copy( T *dst , T const *src , int count )
		{
			memcpy( dst , src , count * sizeof( T ) );
		}
		template< typename T >
		static void zero( T *dst , int count = 1 )
		{
			memset( dst , 0 , count * sizeof( T ) );
		}
		virtual void reset()
		{
			/*for( auto &ptr : allocated_pointers )
			{
				free( ( void* )ptr );
			}*/
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
		void *alloc( size_t size , int alignment = 4 ) override
		{
			size_t ptr = ( size_t )( ( byte* )base_ptr + cur_pos );
			size_t offset = ( ptr + 4 ) & 0x7;
			ptr += offset;
			//ptr = ( void *)( ( size_t )ptr + offset );
			cur_pos += size + offset;
			return ( void * )ptr;
		}
		void free( void * ) override
		{

		}
		void reset() override
		{
			cur_pos = 0;
		}
	};
	/*class StaticAllocator : public Allocator
	{
		struct MemRange
		{
			MemRange *next = nullptr;
			int offset = 0;
			int size = 0;
			bool free;
		};
	private:
		byte *base_ptr;
		int size;
		int range_count;
		MemRange *root;
		MemRange *getNewRangeStruct()
		{
			ito( range_count )
			{
				if( root[ i ].offset < -1 )
				{
					return root + i;
				}
			}
			range_count++;
			return root + range_count - 1;
		}
		void freeRangeStruct( MemRange *rs )
		{
			rs->offset = -1;
		}
	public:
		StaticAllocator( int pages ):
			size( pages * 4096 + 1000 * sizeof( MemRange ) )
		{
			base_ptr = ( byte* )malloc( size );
			root = ( MemRange* )( base_ptr + 1000 * sizeof( MemRange ) );
			root->next = nullptr;
			root->free = true;
			root->offset = 0;
			root->size = size;
		}
		void *alloc( size_t size , int alignment = 4 ) override
		{
			MemRange *cur = root;
			while( cur != nullptr )
			{
				if( cur->free )
				{
					if( cur->size > size )
					{
						MemRange *new_range = getNewRangeStruct();
						new_range->next = cur->next;
						new_range->free = true;
						new_range->offset = cur->offset + size;
						new_range->size = cur->size - size;
						cur->size = size;
						cur->next = new_range;
						cur->free = false;
						return cur->offset + base_ptr;
					} else if( cur->size == size )
					{
						cur->free = false;
						cur->offset + base_ptr;
					}
				} else
				{
					cur = cur->next;
				}
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
		byte *resize( byte *ptr , int new_size )
		{

		}
	};
	struct ThreadLocalAllocator
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