#pragma once
#include <engine/util/defines.hpp>
#ifdef _WIN32
#include <windows.h>
#else
#include <pthread>
#endif
namespace OS
{
	namespace Async
	{
		using namespace Collections;
		using namespace Pointers;
		class Thread
		{
		private:
#ifdef _WIN32
			HANDLE hndl = 0;
			DWORD id = 0;
#endif
			NONMOVABLE( Thread );
		public:
			std::function< void( void ) > func;
			//create and start thread
			static Unique< Thread > create( std::function< void( void ) > func , Allocator *allocator = Allocator::singleton );
			~Thread();
			static void sleep( int milis );
			//must wait thread to stop executing and then release (all) resources
			void join();
		};
		class Signal
		{
		private:
#ifdef _WIN32
			HANDLE hndl = 0;
#endif
			bool lazy;
			bool flag;
		public:
			Signal( bool lazy = true );
			Signal( Signal const & ) = delete;
			Signal& operator=( Signal const & ) = delete;
			Signal( Signal &&th );
			Signal& operator=( Signal &&th );
			void wait( uint32_t milis = INFINITE );
			void reset();
			void signal();
			~Signal();
		};
		using namespace Atomic;
		template< typename T >
		class Promise
		{
		private:
			T value;
			AtomicFlag ready_flag;
			AtomicFlag init_flag;
			Signal signal;
			NONMOVABLE( Promise );
		public:
			static Shared< Promise > create( Allocator *allocator = Allocator::singleton )
			{
				Promise * out = allocator->alloc< Promise >();
				new( out ) Promise();
				return Shared< Promise >( out , allocator );
			}
			bool isReady()
			{
				return ready_flag.isSet();
			}
			Promise &waitTillReady()
			{
				if( ready_flag.isSet() )
				{
					return *this;
				}
				while( true )
				{
					if( init_flag.capture() )
					{
						if( !signal.isValid() )
						{
							signal = Signal::create();
						}
						init_flag.reset();
						break;
					}
				}
				signal.wait();
				return *this;
			}
			Promise &set( T const &value )
			{
				T tmp = value;
				return set( std::move( tmp ) );
			}
			Promise &set( T &&value )
			{
				this->value = std::move( value );
				ready_flag.set();
				while( true )
				{
					if( init_flag.capture() )
					{
						if( signal.isValid() )
						{
							signal.signal();
						}
						init_flag.reset();
						break;
					}
				}
				return *this;
			}
			T &&get()
			{
				return std::move( value );
			}
		};
		class ClockTimer
		{
		public:
			static uint64_t getClocks();
		private:
			uint64_t start;
			char const *msg;
		public:
			ClockTimer( char const *msg ) :
				msg( msg ) ,
				start( getClocks() )
			{}
			~ClockTimer()
			{
				auto end = getClocks();
				OS::IO::debugLogln( msg , " " , ( end - start ) );
			}
		};
		class Timer
		{
		public:
			static uint64_t getCurrentTimeMilis();
		private:
			double dt = 0.0f , last_time = 0.0f , cur_time = 0.0f;
			long long initial_time = 0L;
		public:
			Timer()
			{
				cur_time = last_time = getCurrentTimeMilis();
				dt = 0.0f;
			}
			double getDeltaTime() const
			{
				return dt;
			}
			double getDeltaTimeMs() const
			{
				return dt * 1.0e3;
			}
			void updateTime()
			{
				cur_time = getCurrentTimeMilis();
				dt = cur_time - last_time;
				last_time = cur_time;
			}
		};
	}
}