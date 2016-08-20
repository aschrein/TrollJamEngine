#pragma once
#include <engine/math/vec.hpp>
#include <engine/mem/Allocators.hpp>
#include <engine/data_struct/String.hpp>
using namespace Math;
using namespace Allocators;
namespace Collections
{
	template< typename F , typename ...T >
	struct Comparator
	{
		static bool compare( char c , F f , T... escape_chars )
		{
			if( c == f )
			{
				return true;
			}
			return Comparator< T... >::compare( c , escape_chars... );
		}
	};
	template< typename F >
	struct Comparator< F >
	{
		static bool compare( char c , F f )
		{
			return c == f;
		}
	};
	struct StringStream
	{
		char const *text;
		Allocator *allocator;
		int pos;
		template< typename ...T >
		static bool compare( char c , T... argv )
		{
			return Comparator< T... >::compare( c , argv... );
		}
		template< typename ...T >
		int getNextBreak( T... escape_chars )
		{
			int break_pos = pos;
			do
			{
				break_pos++;
				if( compare( text[ break_pos ] , escape_chars... ) )
				{
					break;
				}
			} while( true );
			return break_pos;
		}
		byte getByte()
		{
			return text[ pos++ ];
		}
		void skip( int n )
		{
			pos += n;
		}
		template< typename ...T >
		String getString( T... escape_chars )
		{
			while( compare( text[ pos ] , '\r' , '\n' , ' ' ) )
			{
				pos++;
			}
			int break_pos = getNextBreak( escape_chars... );
			String slice = String::createSlice( text , pos , break_pos , allocator );
			pos = break_pos;
			while( compare( text[ pos ] , '\r' , '\n' , ' ' ) )
			{
				pos++;
			}
			return slice;
		}
		String getString()
		{
			return getString( '\n' , '\r' , '\0' );
		}
		float getFloat();
		int getInt();
		uint getUint();
		f2 getVec2();
		f3 getVec3();
		i3 getiVec3();
	};
}