#pragma once
#include <data_struct/String.hpp>
enum class PlainFieldType : uint
{
	INT32 = 0 , INT16 , INT8 , FLOAT32 , UINT32 , UINT16 , UINT8
};
#define offsetOf( Class , Field ) ( ( int )&( ( ( Class* )nullptr )->Field ) )
#define createField( Class , Field , Type ) ( PlainField{ #Field , offsetOf( Class , Field ) , Type } )
static int getFieldSize( PlainFieldType type )
{
	static int sizes[]
	{
		4 , 2 , 1 , 4 , 4 , 2 , 1
	};
	return sizes[ ( uint )type ];
}
struct PlainField
{
	Collections::String name;
	uint offset;
	PlainFieldType type;
	uint elem_count;
	uint getSize()
	{
		return elem_count * getFieldSize( type );
	}
	template< typename T >
	void set( void *base , T const *value )
	{
		byte *baseptr = ( byte* )base + offset;
		byte *valptr = ( byte* )value;
		jto( getSize() )
		{
			*baseptr++ = *valptr++;
		}
	}
	template< typename T >
	T get( void const *base )
	{
		byte const *baseptr = ( byte const* )base + offset;
		T value;
		byte *valptr = ( byte* )&value;
		jto( getSize() )
		{
			*valptr++ = *baseptr++;
		}
		return value;
	}
};