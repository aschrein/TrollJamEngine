#pragma once
#include <engine/data_struct/Optional.hpp>
namespace LockFree
{
	using namespace OS::Async;
	using namespace Options;
	template< typename T , int N >
	class RingBuffer
	{
	private:
		T dequeue[ N ];
		AtomicCounter head , tail;
	public:
		void push( T const &val )
		{
			auto tmp = val;
			push( std::move( tmp ) );
		}
		void push( T &&val )
		{
			auto indx = head++;
			dequeue[ indx % N ] = std::move( val );
		}
		Result< T > pop()
		{
			if( head != tail )
			{
				auto indx = tail++;
				T out = std::move( dequeue[ indx % N ] );
				return Result< T >( std::move( out ) );
			}
			return Result< T >();
		}
	};
}