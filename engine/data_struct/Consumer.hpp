#pragma once
#include <engine/data_struct/Array.hpp>
#include <engine/os/Atomic.hpp>
#include <engine/os/Async.hpp>
namespace LockFree
{
	using namespace Collections;
	using namespace OS::Atomic;
	using namespace OS::Async;
	template< typename T >
	class Consumer
	{
	private:
		Array< T > events;
		Signal signal;
		AtomicFlag event_transaction_flag;
	public:
		Consumer( Allocator *allocator = Allocator::singleton )
		{
			events.setAllocator( allocator );
		}
		void pushEvent( T const &file_event )
		{
			auto tmp = file_event;
			pushEvent( std::move( file_event ) );
		}
		void pushEvent( T &&file_event )
		{
			while( true )
			{
				if( event_transaction_flag.capture() )
				{
					events.push( std::move( file_event ) );
					event_transaction_flag.reset();
					signal.signal();
					return;
				}
			}
		}
		Result< T > popEvent( bool wait = false )
		{
			while( true )
			{
once_again:
				if( event_transaction_flag.capture() )
				{
					if( !events.isEmpty() )
					{
						T out( std::move( events.pop() ) );
						event_transaction_flag.reset();
						return Result< T >( std::move( out ) );
					} else if( wait )
					{
						event_transaction_flag.reset();
						signal.wait();
						wait = false;
						goto once_again;
					}
					event_transaction_flag.reset();
					return Result< T >();
				}
			}
		}
		bool isEmpty()
		{
			while( true )
			{
				if( event_transaction_flag.capture() )
				{
					bool empty = events.isEmpty();
					event_transaction_flag.reset();
					return empty;
				}
			}
		}
	};
}