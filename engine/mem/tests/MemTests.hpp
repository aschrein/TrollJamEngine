#pragma once
#include <engine/mem/Allocators.hpp>
#include <engine/data_struct/String.hpp>
#include <engine/os/Async.hpp>
bool AllocatorsTest()
{
	using namespace Allocators;
	using namespace Collections;
	using namespace OS::Async;
	PoolAllocator pool_allocator( 2000 );
	Array< Array< size_t > > marr;
	ito( 1000 )
	{
		Array< size_t > arr;
		arr.setAllocator( ( Allocator* )&pool_allocator );
		{
			//ClockTimer timer( "adding" );
			ito( 1000 )
			{
				arr += i;
			}
		}
		marr += arr;
	}
	/*ito( 1000 )
	{
		OS::IO::debugLogln( arr[ i ] );
	}*/
	OS::IO::debugLogln( "AllocatorsTest success" );
	return true;
}