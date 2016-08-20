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
	class BufferView
	{
	protected:
		int pos = 0;
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
		template< typename T >
		BufferView &put( T const &i )
		{
			*( T* )( data + pos ) = i;
			pos += sizeof( T );
			return *this;
		}
		template< typename T >
		T get()
		{
			pos -= sizeof( T );
			return *( T* )( data + pos );
		}
		template< typename T >
		T getInc()
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
		void reset()
		{
			pos = 0;
		}
		void setPosition( int p )
		{
			pos = p;
		}
	};
}