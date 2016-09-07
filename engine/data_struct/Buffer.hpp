#pragma once
#include <engine/util/defines.hpp>
#include <engine/math/vec.hpp>
#include <engine/components/Reflection.hpp>
namespace Collections
{
	template< typename T >
	struct ArrayView
	{
		T *data;
		int size;
		inline T const &operator[]( int i ) const
		{
			return data[ i ];
		}
		inline T &operator[]( int i )
		{
			return data[ i ];
		}
	};
	template< typename T , int N >
	struct LocalArray
	{
		T data[ N ];
		uint size = 0;
		inline T const &operator[]( int i ) const
		{
			return data[ i ];
		}
		inline T &operator[]( int i )
		{
			return data[ i ];
		}
		void push( T const &value )
		{
			data[ size++ ] = value;
		}
		void push( T &&value )
		{
			data[ size++ ] = std::move( value );
		}
		void resize( uint new_size )
		{
			size = new_size;
		}
		struct ArrayIterator
		{
			LocalArray *array;
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
		struct CArrayIterator
		{
			LocalArray const *array;
			uint pos;
			T const &operator*() const
			{
				return *( array->data + pos );
			}
			T const *operator->() const
			{
				return ( array->data + pos );
			}
			CArrayIterator operator++( int )
			{
				pos++;
				return *this;
			}
			CArrayIterator operator++()
			{
				CArrayIterator out = *this;
				pos++;
				return out;
			}
			bool operator==( CArrayIterator const &a ) const
			{
				return pos == a.pos;
			}
			bool operator!=( CArrayIterator const &a ) const
			{
				return pos != a.pos;
			}
		};
		ArrayIterator begin()
		{
			return{ this , 0 };
		}
		CArrayIterator begin() const
		{
			return{ this , 0 };
		}
		ArrayIterator end()
		{
			return{ this , size };
		}
		CArrayIterator end() const
		{
			return CArrayIterator{ this , size };
		}
	};
	class BufferView
	{
	protected:
		mutable int pos = 0;
		int limit = 0;
		byte *data = nullptr;
	public:
		BufferView() = default;
		BufferView( void *data , int limit ):
			data( ( byte* )data ) ,
			limit( limit )
		{}
		void *getRaw()
		{
			return data;
		}
		void const *getRaw() const
		{
			return data;
		}
		template< typename T >
		BufferView &put( T const &i )
		{
			*( T* )( data + pos ) = i;
			pos += sizeof( T );
			return *this;
		}
		template< typename T >
		T get() const
		{
			pos -= sizeof( T );
			return *( T* )( data + pos );
		}
		template< typename T >
		T getInc() const
		{
			T val = *( T* )( data + pos );
			pos += sizeof( T );
			return val;
		}
		template< typename T , int N >
		BufferView &put( Math::TVector< N , T > const &v )
		{
			ito( N )
			{
				( T* )( data + pos )[ i ] = v[ i ];
			}
			pos += sizeof( T ) * N;
			return *this;
		}
		BufferView &serialize( void *ptr , ArrayView< PlainField > fields )
		{
			ito( fields.size )
			{
				int size = fields[ i ].getSize();
				byte *baseptr = ( byte* )ptr + fields[ i ].offset;
				jto( size )
				{
					put( *baseptr++ );
				}
			}
			return *this;
		}
		BufferView &deserialize( void *ptr , ArrayView< PlainField > fields )
		{
			for( int i = fields.size - 1; i >= 0; i-- )
			{
				int size = fields[ i ].getSize();
				byte *baseptr = ( byte* )ptr + fields[ i ].offset + size - 1;
				for( int j = size - 1; j >= 0; j-- )
				{
					*baseptr-- = get< byte >();
				}
			}
			return *this;
		}
		int getLimit() const
		{
			return limit;
		}
		void reset() const
		{
			pos = 0;
		}
		void setPosition( int p ) const
		{
			pos = p;
		}
	};
}