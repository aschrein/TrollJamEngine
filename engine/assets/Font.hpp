#pragma once
#include <engine/math/vec.hpp>
#include <engine/mem/Allocators.hpp>
#include <engine/data_struct/String.hpp>
#include <engine/data_struct/StringStream.hpp>
using namespace Math;
using namespace Allocators;
using namespace Collections;
struct BMFontInfo
{
	static const int MAX_CHARS = 0x100;
	int4 bounds[ MAX_CHARS ];
};
static BMFontInfo parseBMFont( char const *file_image , Allocator *allocator = Allocator::singleton )
{
	StringStream ss = { file_image , allocator , 0 };
	String token;
	while( ( token = ss.getString( ' ' , '\n' , '\r' ) ) != "chars" )
	{
	}
	ss.getString( '=' );
	ss.skip( 1 );
	int size = ss.getInt();
	OS::IO::debugLogln( size );
	BMFontInfo out;
	ito( size )
	{
		ss.getString( ' ' );
		ss.getString( '=' );
		ss.skip( 1 );
		int id = ss.getInt();
		ss.getString( '=' );
		ss.skip( 1 );
		int x = ss.getInt();
		ss.getString( '=' );
		ss.skip( 1 );
		int y = ss.getInt();
		ss.getString( '=' );
		ss.skip( 1 );
		int width = ss.getInt();
		ss.getString( '=' );
		ss.skip( 1 );
		int height = ss.getInt();
		//OS::IO::debugLogln( id , " " , x , " " , y , " " , width , " " , height );
		out.bounds[ id ] = { x , y , width , height };
		ss.getString( '\n' , '\r' );
	}
	return out;
}