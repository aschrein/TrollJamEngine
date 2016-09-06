#pragma once
#include <engine/data_struct/Collection.hpp>
#ifndef ito
#define ito( x ) for( int i = 0; i < x; i++ )
#endif
#ifndef jto
#define jto( x ) for( int j = 0; j < x; j++ )
#endif
#ifndef xfor
#define xfor( x , n ) for( int x = 0; x < n; x++ )
#endif
#include <engine/mem/Allocators.hpp>
namespace Collections
{
	template< typename K >
	void unpackVarargs( K *data , int cur )
	{}
	template< typename K , typename R , typename ...T >
	void unpackVarargs( K *data , int cur , R const &first , T... args )
	{
		data[ cur ] = K( first );
		unpackVarargs( data , cur + 1 , args... );
	}
	using namespace Allocators;
	template< typename T >
	class Array : public Collection< Array , T >
	{
	protected:
		uint INCREMENT = 20;
		Allocator *allocator = Allocator::singleton;
		T *data = nullptr;
		uint size = 0;
		uint real_size = 0;
		bool copy_on_write = false;
	public:
		Allocator *getAllocator()
		{
			return allocator;
		}
		void setIncrement( uint inc )
		{
			INCREMENT = inc;
		}
		typedef T Type;
		Array &setAllocator( Allocator *allocator )
		{
			if( data )
			{
				auto tmp = ( T* )allocator->alloc( size * sizeof( T ) );
				//Allocator::copy( tmp , data , size * sizeof( T ) );
				ito( size )
				{
					new( tmp + i ) T();
					tmp[ i ] = std::move( data[ i ] );
				}
				this->allocator->free( data );
				data = tmp;
				real_size = size;
				copy_on_write = false;
			}
			this->allocator = allocator;
			return *this;
		}
		template< typename ...K >
		Array( K... args )
		{
			make_space( sizeof...( K ) );
			size = sizeof...( K );
			unpackVarargs( data , 0 , args... );
		}
		Array() = default;
		Array( Array const &rval ) :
			Array()
		{
			*this = rval;
		}
		Array( T const *ptr , uint N )
		{
			make_space( N );
			ito( N )
			{
				data[ i ] = ptr[ i ];
			}
		}
		static Array createView( T const *ptr , uint N , Allocator *allocator = Allocator::singleton )
		{
			Array out;
			out.data = const_cast< T * >( ptr );
			out.size = N;
			out.real_size = N;
			out.copy_on_write = true;
			out.allocator = allocator;
			return out;
		}
		void resetCopyOnWrite()
		{
			if( copy_on_write )
			{
				auto tmp = data;
				data = ( T* )allocator->alloc( size * sizeof( T ) );
				allocator->copy< T >( data , tmp , size );
				real_size = size;
				copy_on_write = false;
			}
		}
		Array const &operator=( Array const & rval )
		{
			release();
			size = rval.size;
			real_size = rval.size;
			copy_on_write = rval.copy_on_write;
			allocator = rval.allocator;
			if( copy_on_write )
			{
				data = rval.data;
			} else
			{
				data = ( T* )allocator->alloc( sizeof( T ) * size );
				ito( size )
				{
					new( data + i ) T();
					data[ i ] = rval[ i ];
				}
				//allocator->copy< T >( data , rval.data , size );
			}
			return *this;
		}
		Array( Array&& rval ) :
			Array()
		{
			*this = std::move( rval );
		}
		/*template< typename ...K >
		Array const &operator=( K... args )
		{
			Array tmp( args... );
			*this = tmp;
			return *this;
		}*/
		Array const &operator=( Array&& rval )
		{
			release();
			size = rval.size;
			real_size = rval.real_size;
			data = rval.data;
			copy_on_write = rval.copy_on_write;
			allocator = rval.allocator;
			rval.size = 0;
			rval.real_size = 0;
			rval.data = nullptr;
			rval.copy_on_write = false;
			return *this;
		}
		struct ArrayIterator
		{
			Array const *array;
			uint pos;
			T &operator*()
			{
				return *( array->data + pos );
			}
			T *operator->()
			{
				return ( array->data + pos );
			}
			T const &operator*() const
			{
				return *( array->data + pos );
			}
			T const *operator->() const
			{
				return ( array->data + pos );
			}
			ArrayIterator operator++( int )
			{
				pos++;
				return *this;
			}
			ArrayIterator operator++()
			{
				ArrayIterator out = *this;
				pos++;
				return out;
			}
			bool operator==( ArrayIterator const &a ) const
			{
				return pos == a.pos;
			}
			bool operator!=( ArrayIterator const &a ) const
			{
				return pos != a.pos;
			}
		};
		ArrayIterator begin()
		{
			return{ this , 0 };
		}
		ArrayIterator begin() const
		{
			return{ this , 0 };
		}
		ArrayIterator end()
		{
			return{ this , size };
		}
		ArrayIterator end() const
		{
			return{ this , size };
		}
		void make_space( uint new_size )
		{
			if( new_size < size )
			{
				for( int i = new_size; i < size; i++ )
				{
					data[ i ].~T();
				}
			}
			if( new_size > real_size )
			{
				uint new_real_size = new_size + INCREMENT;
				uint s = sizeof( T );
				T *new_data = ( T* )allocator->alloc( new_real_size * sizeof( T ) );
				//allocator->zero( new_data , ( size ) );
				if( data != nullptr )
				{
					allocator->copy< T >( new_data , data , size );
					if( copy_on_write )
					{
						copy_on_write = false;
					} else
					{
						allocator->free( data );
					}
				}
				data = new_data;
				real_size = new_real_size;
			} else if( new_size < real_size - INCREMENT )
			{
				uint new_real_size = new_size;
				T *new_data = ( T * )allocator->alloc( new_real_size * sizeof( T ) );
				if( data != nullptr )
				{
					allocator->copy< T >( new_data , data , new_size );
					if( copy_on_write )
					{
						copy_on_write = false;
					} else
					{
						allocator->free( data );
					}
				}
				data = new_data;
				real_size = new_real_size;
			}
			if( new_size > size )
			{
				for( int i = size; i < new_size; i++ )
				{
					new( data + i ) T();
				}
			}
			//size = new_size;
		}
		void remove( uint indx )
		{
			if( indx >= size )
			{
				return;
			}
			resetCopyOnWrite();
			data[ indx ] = data[ size - 1 ];
			make_space( size - 1 );
			size--;
		}
		void removeShift( uint indx )
		{
			resetCopyOnWrite();
			for( int i = indx; i < size - 1; i++ )
			{
				data[ i ] = data[ i + 1 ];
			}
			make_space( size - 1 );
			size--;
		}
		void removeFirst( T const &val )
		{
			resetCopyOnWrite();
			ito( getSize() )
			{
				if( data[ i ] == val )
				{
					remove( i );
					return;
				}
			}
		}
		T pop()
		{
			T out = std::move( data[ size - 1 ] );
			make_space( size - 1 );
			size--;
			return std::move( out );
		}
		Array &push( T &&arg )
		{
			resetCopyOnWrite();
			make_space( size + 1 );
			data[ size ] = std::move( arg );
			size++;
			return *this;
		}
		Array &push( T const &arg )
		{
			auto tmp = arg;
			return push( std::move( tmp ) );
		}
		inline T const &operator[]( int i ) const
		{
			return data[ i ];
		}
		inline T &operator[]( int i )
		{
			resetCopyOnWrite();
			return data[ i ];
		}
		Array &operator+=( Array const &rval )
		{
			if( rval.getSize() == 0 )
			{
				return *this;
			}
			for( auto const &val : rval )
			{
				push( val );
			}
			return *this;
		}
		Array const operator+( Array const &rval ) const
		{
			Array out = *this;
			out += rval;
			return out;
		}
		Array &operator+=( T const &rval )
		{
			return push( rval );
		}
		Array const operator+( T const &rval ) const
		{
			Array out = *this;
			out += rval;
			return out;
		}
		void cut( uint start , uint end )
		{
			uint count = end - start;
			for( uint i = start; i < size - count; i++ )
			{
				data[ i ] = data[ i + count ];
			}
			size -= count;
		}
		void insert( uint start , uint length , Array const &arr )
		{
			make_space( size + length );
			for( uint i = size + length - 1; i >= start; i-- )
			{
				data[ i ] = data[ i - length ];
			}
			ito( length )
			{
				data[ i + start ] = arr[ i ];
			}
			size += length;
		}
		void release()
		{
			if( copy_on_write )
			{
				data = nullptr;
			}
			if( data != nullptr )
			{
				ito( size )
				{
					data[ i ].~T();
				}
				allocator->free( data );
				data = nullptr;
			}
			size = 0;
			real_size = 0;
		}
		~Array()
		{
			release();
		}
		bool isEmpty() const
		{
			return getSize() == 0;
		}
		uint getSize() const
		{
			return size;
		}
		T *getPtr()
		{
			resetCopyOnWrite();
			return data;
		}
		T const *getPtr() const
		{
			return data;
		}
		Array &operator,( T const &val )
		{
			resetCopyOnWrite();
			*this += val;
			return *this;
		}
		static Array< T > const range( int a , int b )
		{
			if( a >= b )
			{
				return Array< T >();
			}
			int N = b - a;
			Array< T > out;
			ito( N )
			{
				out.push( static_cast< T >( a + i ) );
			}
			return out;
		}
		Array slice( int s , int e ) const
		{
			if( e <= s || e > getSize() || s < 0 )
			{
				return Array();
			}
			Array out( e - s );
			ito( e - s )
			{
				out[ i ] = ( *this )[ s + i ];
			}
			return out;
		}
		Array const mask( Array< bool > mask ) const
		{
			Array out;
			ito( getSize() )
			{
				if( mask[ i ] )
				{
					out.push( ( *this )[ i ] );
				}
			}
			return out;
		}
		bool operator==( Array const &str ) const
		{
			if( getSize() != str.getSize() )
			{
				return false;
			}
			ito( getSize() )
			{
				if( ( *this )[ i ] != str[ i ] )
				{
					return false;
				}
			}
			return true;
		}
		bool operator!=( Array const &str ) const
		{
			return !( *this == str );
		}
	};
}
