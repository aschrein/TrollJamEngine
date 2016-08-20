#include <stdafx.h>
#include <string.h>
#include <engine/data_struct/StringStream.hpp>
namespace Collections
{
	/*template< typename Allocator >
	std::ostream& operator<<( std::ostream& os , TString< Allocator > const &string )
	{
		os << string.getChars();
		return os;
	}*/
	template<>
	float StringUtil::parse< float >( char const *text )
	{
		return strtof( text , NULL );
	}
	template<>
	double StringUtil::parse< double >( char const *text )
	{
		return strtod( text , NULL );
	}
	template<>
	int StringUtil::parse< int >( char const *text )
	{
		return strtol( text , NULL , 10 );
	}
	template<>
	uint StringUtil::parse< uint >( char const *text )
	{
		return strtol( text , NULL , 10 );
	}
	template<>
	long StringUtil::parse< long >( char const *text )
	{
		return strtol( text , NULL , 10 );
	}
	template<>
	unsigned long StringUtil::parse< unsigned long >( char const *text )
	{
		return strtol( text , NULL , 10 );
	}
	float StringStream::getFloat()
	{
		char *next_break;
		float out = strtod( text + pos , &next_break );
		pos = int( next_break - text );
		return out;
	}
	int StringStream::getInt()
	{
		char *next_break;
		int out = strtol( text + pos , &next_break , 10 );
		pos = int( next_break - text );
		return out;
	}
	uint StringStream::getUint()
	{
		char *next_break;
		uint out = strtol( text + pos , &next_break , 10 );
		pos = int( next_break - text );
		return out;
	}
	f2 StringStream::getVec2()
	{
		return{ getFloat() , getFloat() };
	}
	f3 StringStream::getVec3()
	{
		return{ getFloat() , getFloat() , getFloat() };
	}
	i3 StringStream::getiVec3()
	{
		return{ getInt() , getInt() , getInt() };
	}
	String operator+( char const c , String const &str )
	{
		String out = c;
		out += str;
		return out;
	}
	String operator+( char const *c , String const &str )
	{
		String out = c;
		out += str;
		return out;
	}
	template<>
	void StringUtil::stringify< char >( char *mem , char const &val )
	{
		mem[ 0 ] = val;
		mem[ 1 ] = '\0';
	}
	template<>
	void StringUtil::stringify< char const * >( char *mem , char const * const &val )
	{
		sprintf_s( mem , 100 , "%s" , val );
	}
	template<>
	void StringUtil::stringify< char * >( char *mem , char * const &val )
	{
		sprintf_s( mem , 100 , "%s" , val );
	}
	template<>
	void StringUtil::stringify< int >( char *mem , int const &val )
	{
		sprintf_s( mem , 100 , "%i" , val );
	}
	template<>
	void StringUtil::stringify< float >( char *mem , float const &val )
	{
		sprintf_s( mem , 100 , "%f" , val );
	}
	template<>
	void StringUtil::stringify< bool >( char *mem , bool const &val )
	{
		if( val )
		{
			sprintf_s( mem , 100 , "%s" , "true" );
		} else
		{
			sprintf_s( mem , 100 , "%s" , "false" );
		}
	}
	/*template
	TString TString::stringify< char const * , DefaultAllocator >( char const * const & );
	template
	TString TString::stringify< int , DefaultAllocator >( int const & );
	template
	TString TString::stringify< bool , DefaultAllocator >( bool const & );
	template
	TString TString::stringify< float , DefaultAllocator >( float const & );
	template
	TString TString::stringify< double , DefaultAllocator >( double const & );
	template
	TString TString::stringify< long , DefaultAllocator >( long const & );*/
	int StringUtil::cstringLen( char const *cstring )
	{
		return strlen( cstring ) + 1;
	}
}