#pragma once
#include <engine/data_struct/RingBuffer.hpp>
#include <engine/os/Atomic.hpp>
#include <engine/os/Async.hpp>
namespace LockFree
{
	using namespace Collections;
	using namespace OS::Atomic;
	using namespace OS::Async;
	template< typename T , int N >
	class Consumer
	{
	private:
		RingBuffer< T , N > events;
		Signal signal;
	public:
		Consumer( Consumer const & ) = delete;
		Consumer &operator=( Consumer const & ) = delete;
		Consumer( Consumer && ) = default;
		Consumer &operator=( Consumer && ) = default;
		Consumer() = default;
		void pushEvent( T const &file_event )
		{
			auto tmp = file_event;
			pushEvent( std::move( file_event ) );
		}
		void pushEvent( T &&file_event )
		{
			events.push( std::move( file_event ) );
			signal.signal();
		}
		T popEvent( bool wait = true )
		{
			if( events.isEmpty() && wait )
			{
				signal.wait();
				signal.reset();
			}
			return events.pop();
		}
		bool isEmpty()
		{
			return events.isEmpty();
		}
	};
}