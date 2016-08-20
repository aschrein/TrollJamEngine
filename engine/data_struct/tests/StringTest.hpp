#pragma once
#include <data_struct/String.hpp>
#include <iostream>
#include <test_util/TestUtil.hpp>
#include <os/Async.hpp>
#include <mem/Allocators.hpp>
using namespace std;
using namespace Collections;
bool StringTest()
{
	{
		LinearAllocator *static_allocator = new LinearAllocator( 1000 );
		ito( 10 )
		{
			static_allocator->reset();
			String string1;( ( Allocator * )static_allocator );
			{
				//OS::Async::ClockTimer timer( "initialization" );
				string1 = "test string1";
			}
			{
				//OS::Async::ClockTimer timer( "cpy" );
				auto str = string1;
			}
			{
				//OS::Async::ClockTimer timer( "addition1" );
				string1 << "hello";
			}
			{
				//OS::Async::ClockTimer timer( "addition2" );
				string1 << "hellohello";
			}
			{
				//OS::Async::ClockTimer timer( "cast int" );
				string1 << 100500;
			}
			//OS::IO::debugLogln( string1 );
		}
	}
	{
		String string1( "test string1" );
		String string2( "test string2" );
		String string3 = " ";
		string3 = string1 + string3 + string2;
		RETURN_ASSERT( string3 == "test string1 test string2" );
	}
	{
		Allocators::LinearAllocator *allocator = new Allocators::LinearAllocator( 1000 );
		String path( ( Allocators::Allocator* )allocator );
		//path += 'f';
		auto res = path + "1";
		res += "1234567890";
		RETURN_ASSERT( res == String( "11234567890" ) );
	////	OS::IO::debugLogln( path );
		//OS::IO::debugLogln( allocator->getPosition() );
	}
	OS::IO::debugLogln( "string test success" );
	return true;
}