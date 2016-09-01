#include <stdafx.h>
#include <omp.h>
#include <engine/os/Async.hpp>
namespace OS
{
	void exit()
	{
		::exit( 0 );
	}
	namespace Async
	{
		void sleep( int milis )
		{
			Sleep( milis );
		}
		//HashMap< size_t , Thread* > Thread::thread_map;
		uint64_t ClockTimer::getClocks()
		{
			return __rdtsc();
		}
		uint64_t Timer::getCurrentTimeMilis()
		{
			return omp_get_wtime() * 1.0e3;
		}
		using namespace Collections;
		using namespace Pointers;
		static DWORD WINAPI startThread( LPVOID lpParam )
		{
			Thread *thread = ( Thread* )lpParam;
			thread->func();
			return 0;
		}
		Unique< Thread > Thread::create( std::function< void( void ) > func , Allocator *allocator )
		{
			Thread *new_thread = allocator->alloc< Thread >();
			new( new_thread ) Thread();
			new_thread->func = func;
			new_thread->hndl = CreateThread( NULL , 0 , startThread , new_thread , 0 , &new_thread->id );
			return new_thread;
		}
		Thread::~Thread()
		{
			join();
		}
		void Thread::sleep( int milis )
		{
#ifdef _WIN32
			Sleep( milis );
#endif
		}
		void Thread::join()
		{
#ifdef _WIN32
			if( hndl )
			{
				WaitForSingleObject( hndl , INFINITE );
				CloseHandle( hndl );
				hndl = 0;
				id = 0;
			}
#endif
		}
		Signal::Signal( Signal &&th )
		{
			*this = std::move( th );
		}
		Signal& Signal::operator=( Signal &&th )
		{
			lazy = th.lazy;
			hndl = th.hndl;
			flag = th.flag;
			th.hndl = 0;
			return *this;
		}
		Signal::Signal( bool lazy ):
			lazy( lazy )
		{
			if( !lazy )
			{
				hndl = CreateEvent( NULL , TRUE , FALSE , NULL );
				flag = false;
			}
		}
		void Signal::wait( uint32_t milis )
		{
			if( !hndl )
			{
				hndl = CreateEvent( NULL , TRUE , FALSE , NULL );
				flag = false;
			}
			auto result = WaitForSingleObject( hndl , milis );
		}
		void Signal::reset()
		{
			if( hndl )
			{
				auto result = ResetEvent( hndl );
				flag = false;
			}
		}
		void Signal::signal()
		{
			if( !hndl )
			{
				hndl = CreateEvent( NULL , TRUE , FALSE , NULL );
			}
			if( hndl )
			{
				SetEvent( hndl );
				flag = true;
			}
		}
		Signal::~Signal()
		{
			if( hndl )
			{
				CloseHandle( hndl );
			}
			flag = false;
			hndl = 0;
		}
	}
	namespace Atomic
	{
		bool AtomicFlag::capture()
		{
			return !InterlockedCompareExchange64( &flag , 1 , 0 );
		}
		bool AtomicFlag::isSet() const
		{
			return InterlockedCompareExchange64( &flag , 1 , 1 );
		}
		void AtomicFlag::set()
		{
			InterlockedExchange64( &flag , 1 );
		}
		void AtomicFlag::reset()
		{
			InterlockedExchange64( &flag , 0 );
		}
		int64_t AtomicCounter::operator++()
		{
			return InterlockedIncrement64( &counter );
		}
		int64_t AtomicCounter::operator++( int )
		{
			return InterlockedIncrement64( &counter ) - 1;
		}
		int64_t AtomicCounter::operator--()
		{
			return InterlockedDecrement64( &counter );
		}
		int64_t AtomicCounter::operator--( int )
		{
			return InterlockedDecrement64( &counter ) + 1;
		}
		AtomicCounter &AtomicCounter::operator=( int64_t value )
		{
			InterlockedExchange64( &counter , value );
			return *this;
		}
		int64_t AtomicCounter::get() const
		{
			return counter;
		}
		bool AtomicCounter::operator==( AtomicCounter const &c ) const
		{
			return counter == c.counter;
		}
		bool AtomicCounter::operator!=( AtomicCounter const &c ) const
		{
			return !( *this == c );
		}
	}
}