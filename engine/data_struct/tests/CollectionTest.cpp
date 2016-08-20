#pragma once
#include <stdafx.h>
#include <test_util/TestUtil.hpp>
#include <data_struct/tests/CollectionTest.hpp>
using namespace std;
using namespace Collections;
int counter = 0;
struct TestStruct
{
	TestStruct()
	{
		counter++;
		//cout << "created id:" << counter++ << endl;
	}
	TestStruct( TestStruct const &)
	{
		counter++;
		//cout << "created id:" << counter++ << endl;
	}
	TestStruct &operator=( TestStruct const & )
	{
		counter++;
		//cout << "created id:" << counter++ << endl;
		return *this;
	}
	TestStruct( TestStruct && )
	{

		//cout << "created id:" << counter++ << endl;
	}
	TestStruct &operator=( TestStruct && )
	{
		//cout << "created id:" << counter++ << endl;
		return *this;
	}
	~TestStruct()
	{
		counter--;
		//cout << "deleted id:" << --counter << endl;
	}
};
Tuple< int , String > getString()
{
	return{ 0 , "hello, world!" };
}
bool CollectionTest()
{
	{
		{
			Array< TestStruct > array;
			array += TestStruct();
			array += TestStruct();
		}
		RETURN_ASSERT( counter == 0 );
	}
	auto t = getString();
	RETURN_ASSERT( t.get< 0 >() == 0 );
	RETURN_ASSERT( t.get< 1 >() == "hello, world!" );
	Array< int > outer;
	{
		int stack_arr[] = { 0 , 1 , 2 , 666 , 777 };
		Array< int > n = Array< int >::createView( stack_arr , sizeof( stack_arr ) );
		RETURN_ASSERT( n[ 0 ] == 0 && n[ 3 ] == 666 );
		n[ 3 ] = 777;
		RETURN_ASSERT( n[ 3 ] == 777 && stack_arr[ 3 ] == 666 );
		auto a = Array< int >::range( 0 , 10 );
		auto tmp = a
			.map< bool >( []( int v )
		{
			return v > 5;
		} )
			.filter( []( bool b )
		{
			return b;
		} );
		RETURN_ASSERT( tmp.getSize() == 4 );
		auto tmp1 = a.map< bool >( []( int v )
		{
			return v % 2 == 1;
		} );
		auto tmp2 = a.mask( tmp1 );
		RETURN_ASSERT( tmp2.getSize() == 5 );
		Array< int > ii = { 1 , 2 , 3 , 4 , 5 };
		ii = { 1 , 2 , 888 };
	}
	outer.print();
	{
		Array< int > a;
		a , 1 , 2 , 3 , 4 , 5;
		RETURN_ASSERT( a == Array< int >( 1 , 2 , 3 , 4 , 5 ) );
	}
	OS::IO::debugLogln( "Array test success" );
	return true;
}
bool LinkedListTest()
{
	{
		LinkedList< String > list;
		list.pushUnique( "banana" );
		list.pushUnique( "apple" );
		list.pushUnique( "orange" );
		RETURN_ASSERT( list.contains( "banana" ) );
		RETURN_ASSERT( list.contains( "apple" ) );
		list.remove( "apple" );
		RETURN_ASSERT( list.contains( "orange" ) );
		RETURN_ASSERT( !list.contains( "apple" ) );
		//OS::IO::debugLogln( tmp );
		//stress test
		/*while( true  )
		{
		LinkedList< String > tmp;
		ito( 10000 )
		{
		tmp.pushUnique( i );
		}
		ito( 10000 )
		{
		tmp.remove( i );
		}
		OS::Async::sleep( 300 );
		RETURN_ASSERT( tmp.getSize() == 0 );
		}*/
	}
	OS::IO::debugLogln( "LinkedListTest success" );
	return true;
}
#include <ctime>
bool BufferTest()
{
	using namespace Math;
	BufferView bv( new byte[ 100 ] , 100 );
	struct TestStruct
	{
		f3 pos = { 1.0f , 0.0f , 0.0f };
		int id = 13;
	} test_struct;
	PlainField plain_fields[] =
	{
		createField( TestStruct , pos , PlainFieldType::FLOAT32x3 ) ,
		createField( TestStruct , id , PlainFieldType::INT32 ) ,
	};
	//OS::IO::debugLogln( test_struct.id );
	//test_struct.pos.print();
	bv.serialize( &test_struct , ArrayView< PlainField >{ plain_fields , 2 } );
	bv.deserialize( &test_struct , ArrayView< PlainField >{ plain_fields , 2 } );
	//OS::IO::debugLogln( test_struct.id );
	//test_struct.pos.print();
	plain_fields[ 0 ].set( &test_struct , &f3( 0.0f , 999.0f , -666.0f ) );
	RETURN_ASSERT( test_struct.pos.y = 999.0f );
	//test_struct.pos.print();
	//OS::IO::debugLogln( plain_fields[ 1 ].get< int >( &test_struct ) );
	return true;
}