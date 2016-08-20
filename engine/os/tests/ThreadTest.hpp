#pragma once
#include <os/Async.hpp>
#include <os/log.hpp>
#include <data_struct/Array.hpp>
#include <util/Timer.hpp>
#include <data_struct/RingBuffer.hpp>
using namespace OS::Async;
Pointers::Unique< int > testRet()
{
	return new int( 10 );
}
bool RingBufferTest()
{
	using namespace LockFree;
	/*Pointers::Unique< int > test = testRet();
	auto t2 = std::move( test );
	OS::IO::debugLogln( *t2 );*/
	RingBuffer< int , 100 > ring_buffer;
	AtomicFlag flag;
	Array< Unique< Thread > > threads;
	ito( 10 )
	{
		auto thread_desc = Thread::create(
			[ i , &ring_buffer , &flag ]()
		{
			while( true )
			{
				if( !ring_buffer.isEmpty() )
				{
					if( flag.capture() )
					{
						int value = ring_buffer.pop();
						OS::IO::debugLogln( "thread " , i , " got value: " , value );
						flag.reset();
					}
				}
				Thread::sleep( 1 );
			}
		}
		);
		threads.push( std::move( thread_desc ) );
	}
	auto thread_desc = Thread::create(
		[ &ring_buffer ]()
	{
		int counter = 0;
		while( true )
		{
			ring_buffer.push( counter++ );
			Thread::sleep( 10 );
		}
	}
	);
	for( auto &thread : threads )
	{
		thread->join();
	}
	return true;
}
bool ThreadTest()
{
	
	OS::Async::AtomicFlag flag;
	
	Array< Unique< Thread > > threads;
	Promise< int > *promise = Promise< int >::create();
	AtomicCounter atomic_counter;
	atomic_counter.set( 100 );
	int counter = 0;
	ito( 4 )
	{
		threads.push( Thread::create(
			[ &atomic_counter , &counter , promise , i ,  &flag ]()
		{
			int thread_counter = 10;
			promise->waitTillReady();
			int val = promise->get();
			//OS::IO::debugLogln( "thread " , i , " got value " , val );

			while( atomic_counter.get() )
			{
				int v = atomic_counter.decrease();
				auto start_clocks = ClockTimer::getClocks();
				while( true )
				{
					if( flag.capture() )
					{
						auto end_clocks = ClockTimer::getClocks();
						counter += end_clocks - start_clocks;
						OS::IO::debugLogln( "thread " , i , " increments to " , v );
						flag.reset();
						break;
					}
				}
				Thread::sleep( 1 );
			}
		}
		) );
	}
	Thread::create(
		[ promise ]()
	{
		Thread::sleep( 100 );
		promise->set( 777 );
		OS::IO::debugLogln( "promise was set" );
	} );
	for( auto &thread : threads )
	{
		thread->join();
	}
	OS::IO::debugLogln( "avg ticked   " , counter / 1000.0f );
	delete promise;
	threads.release();
	return true;
}