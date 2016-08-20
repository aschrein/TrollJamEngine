#pragma once
#include <engine/data_struct/Array.hpp>
#include <engine/data_struct/Hash.hpp>
#include <engine/math/Math.hpp>
#include <engine/mem/Allocators.hpp>
namespace Collections
{
	using namespace Allocators;
	struct StringUtil
	{
		template< typename T >
		static void stringify( char *mem , T const & );
		template< typename T >
		static T parse( char const *text );
		static int cstringLen( char const * );
	};
	class String
	{
	private:
		Array< char > chars;
	public:
		String( Allocator *allocator )
		{
			chars.setAllocator( allocator );
		}
		String() = default;
		String( String && ) = default;
		String( String const & ) = default;
		String &operator=( String const & ) = default;
		String &operator=( String && ) = default;
		String( char const *cstring )
		{
			*this = cstring;
		}
		~String()
		{
			/*if( chars.getSize() > 0 )
			{
				OS::IO::debugLogln( "string destructed : " , getChars() );
			}*/
		}
		String( char const *cstring , bool )
		{
			*this = cstring;
			chars.resetCopyOnWrite();
		}
		template< typename T >
		String( T const &val )
		{
			*this = val;
		}
		typedef char* iterator;
		typedef const char* const_iterator;
		iterator begin()
		{
			return &chars[ 0 ];
		}
		const_iterator begin() const
		{
			return &chars[ 0 ];
		}
		iterator end()
		{
			return &chars[ chars.getSize() - 1 ];
		}
		const_iterator end() const
		{
			return &chars[ chars.getSize() - 1 ];
		}
		char const *getChars() const
		{
			return chars.getPtr();
		}
		template< typename T >
		String &operator=( T const &val )
		{
			chars.release();
			*this += val;
			return *this;
		}
		String &operator=( char const *cstring )
		{
			int n = StringUtil::cstringLen( cstring );
			chars = std::move( Array< char >::createView( cstring , n , chars.getAllocator() ) );
			return *this;
		}
		String &operator+=( char const *cstring )
		{
			if( chars.getSize() > 0 )
			{
				chars.pop();
			}
			do
			{
				chars += *cstring;
			} while( *cstring++ != '\0' );
			return *this;
		}
		static String createSlice( char const *text , int start , int end , Allocator *allocator = Allocator::singleton )
		{
			String out( allocator );
			for( int i = start; i < end; i++ )
			{
				out += text[ i ];
			}
			out += '\0';
			return out;
		}
		String replace( String from , String to )
		{
			String out = *this;
			int i = 0;
			while( i < out.getLength() )
			{
				int j = 0;
				while( out[ j + i ] != '\0' && from[ j ] != '\0' && out[ j + i ] == from[ j ] )
				{
					j++;
				}
				if( from[ j ] == '\0' )
				{
					out.chars.cut( i , i + j );
					out.chars.insert( i , to.getLength() , to.chars );
				}
				i++;
			}
			return std::move( out );
		}
		String &operator+=( char *cstring )
		{
			return *this += ( char const * )cstring;
		}
		String operator+( char const *cstring ) const
		{
			auto cpy = *this;
			cpy += cstring;
			return cpy;
		}
		String &operator+=( char c )
		{
			chars += c;
			return *this;
		}
		String operator+( char c ) const
		{
			String tmp = *this;
			tmp += c;
			return tmp;
		}
		template< typename T >
		String &operator+=( T const &c )
		{
			char *tmp = ( char* )chars.getAllocator()->alloc( 100 );
			StringUtil::stringify( tmp , c );
			*this += tmp;
			chars.getAllocator()->free( tmp );
			return *this;
		}
		String &operator<<( String const &c )
		{
			return *this += c;
		}
		template< typename T >
		String &operator<<( T const &c )
		{
			return *this += c;
		}
		String &operator<<( char const *cstring )
		{
			return *this += cstring;
		}
		template< typename T >
		String const operator+( T const &c ) const
		{
			String tmp = *this;
			tmp += c;
			return tmp;
		}
		String &operator+=( String const &str )
		{
			if( chars.getSize() > 0 )
			{
				chars.pop();
			}
			chars += str.chars;
			return *this;
		}
		String const operator+( String const &str ) const
		{
			String out = *this;
			out += str;
			return out;
		}
		int hash() const
		{
			int hash = 0;
			int i = 0;
			for( char c : chars )
			{
				hash += ( 0xff << ( i++ ) ) + c;
			}
			return Math::MathUtil< int >::abs( hash );
		}
		bool operator<( String const &str ) const
		{
			if( getLength() == str.getLength() )
			{
				ito( getLength() )
				{
					if( chars[ i ] == str.chars[ i ] )
					{
						continue;
					} else
					{
						return chars[ i ] < str.chars[ i ];
					}
				}
				return false;
			} else
			{
				return getLength() < str.getLength();
			}
		}
		inline char const &operator[]( int i ) const
		{
			return chars[ i ];
		}
		inline char &operator[]( int i )
		{
			return chars[ i ];
		}
		int getLength() const
		{
			return Math::MathUtil< int >::max( 0 , chars.getSize() - 1 );
		}
		bool operator==( String const &str ) const
		{
			return chars == str.chars;
		}
		bool operator!=( String const &str ) const
		{
			return chars != str.chars;
		}
		String copy() const
		{
			String out = *this;
			out.chars.pop();
			out.chars += '\0';
			return out;
		}
	};
	String operator+( char const c , String const &str );
	String operator+( char const *c , String const &str );
}
