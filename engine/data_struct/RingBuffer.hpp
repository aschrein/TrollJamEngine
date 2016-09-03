#pragma once
#include <engine/data_struct/Optional.hpp>
#include <engine/os/Async.hpp>
//#include <atomic>
namespace LockFree
{
	using namespace OS::Async;
	using namespace Options;
	template< typename T , int N >
	class RingBuffer
	{
	private:
		//std::atomic< int64_t > head , tail;
		AtomicCounter head , tail;
		T dequeue[ N ];
		AtomicFlag present_flag[ N ];
		//AtomicFlag transaction_flag;
	public:
		void push( T const &val )
		{
			auto tmp = val;
			push( std::move( tmp ) );
		}
		uint getTail() const
		{
			return tail.get();
		}
		uint getHead() const
		{
			return head.get();
		}
		void push( T &&val )
		{
			//while( !transaction_flag.capture() );
			auto indx = head++;
			dequeue[ indx % N ] = std::move( val );
			present_flag[ indx % N ].set();
			//transaction_flag.reset();
		}
		bool isEmpty() const
		{
			return head == tail;
		}
		T pop()
		{
			while( !present_flag[ tail.get() % N ].isSet() );
			//while( !transaction_flag.capture() );
			//if( head != tail )
			{
				auto indx = tail++;
				//transaction_flag.reset();
				T out = std::move( dequeue[ indx % N ] );
				present_flag[ indx % N ].reset();
				return std::move( out );
			}
			//transaction_flag.reset();
			//return Result< T >();
		}
	};
}