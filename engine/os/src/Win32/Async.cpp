#include <engine/stdafx.h>
#include <omp.h>
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
			th.hndl = 0;
			return *this;
		}
		void Signal::wait( uint32_t milis )
		{
			if( !hndl )
			{
				hndl = CreateEvent( NULL , TRUE , FALSE , NULL );
			}
			auto result = WaitForSingleObject( hndl , milis );
		}
		void Signal::reset()
		{
			if( hndl )
			{
				auto result = ResetEvent( hndl );
			}
		}
		void Signal::signal()
		{
			if( !hndl && !lazy )
			{
				hndl = CreateEvent( NULL , TRUE , TRUE , NULL );
			} else if( hndl )
			{
				SetEvent( hndl );
			}
		}
		Signal::~Signal()
		{
			if( hndl )
			{
				CloseHandle( hndl );
				hndl = 0;
			}
		}
	}
	namespace Atomic
	{
		bool AtomicFlag::capture()
		{
			return !InterlockedCompareExchangeAcquire( &flag , 1 , 0 );
		}
		bool AtomicFlag::isSet()
		{
			return InterlockedCompareExchangeAcquire( &flag , 1 , 1 );
		}
		void AtomicFlag::set()
		{
			InterlockedExchange( &flag , 1 );
		}
		void AtomicFlag::reset()
		{
			InterlockedExchange( &flag , 0 );
		}
		unsigned long AtomicCounter::operator++()
		{
			return InterlockedIncrement( &counter ) - 1;
		}
		unsigned long AtomicCounter::operator++( int )
		{
			return InterlockedIncrement( &counter );
		}
		unsigned long AtomicCounter::operator--()
		{
			return InterlockedDecrement( &counter ) + 1;
		}
		unsigned long AtomicCounter::operator--( int )
		{
			return InterlockedDecrement( &counter );
		}
		AtomicCounter &AtomicCounter::operator=( uint64_t value )
		{
			InterlockedExchange( &counter , value );
			return *this;
		}
		uint64_t AtomicCounter::get() const
		{
			return InterlockedCompareExchangeAcquire( &counter , counter , counter );
		}
		bool AtomicCounter::operator==( AtomicCounter const &c ) const
		{
			return c.counter == counter;
		}
		bool AtomicCounter::operator!=( AtomicCounter const &c ) const
		{
			return !( *this == c );
		}
	}
}