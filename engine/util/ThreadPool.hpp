#pragma once
#include <engine/data_struct/RingBuffer.hpp>
#include <engine/os/Async.hpp>
#include <engine/util/defines.hpp>
#include <engine/data_struct/Buffer.hpp>
#include <engine/mem/Pointers.hpp>
namespace WorkSystem
{
	using namespace OS::Async;
	using namespace Collections;
	using namespace Pointers;
	using namespace LockFree;
	using namespace OS::Atomic;
	typedef std::function< void( uint , uint ) > GrainedWork;
	class ThreadPool
	{
		NONMOVABLE( ThreadPool );
	private:
		LocalArray< Unique< Thread > , 16 > threads;
		//Signal start_work_signal;
		Signal end_work_signal;
		struct WorkDesc
		{
			uint work_offset;
			uint work_size;
			GrainedWork work;
		};
		LocalArray< RingBuffer< WorkDesc , 100 > , 16 > work_pool_per_threads;
		LocalArray< Signal , 16 > signal_per_threads;
		AtomicFlag working_flag;
		AtomicFlag dispatching_flag;
		AtomicFlag transaction_flag;
		AtomicCounter work_in_progress;
	public:
		static Unique< ThreadPool > create( uint thread_count , Allocator *allocator = Allocator::singleton )
		{
			ThreadPool *out = allocator->alloc< ThreadPool >();
			new( out ) ThreadPool();
			out->end_work_signal = Signal( false );
			//out->start_work_signal = Signal( false );
			out->working_flag.set();
			out->work_pool_per_threads.size = thread_count;
			ito( thread_count )
			{
				out->signal_per_threads.push( Signal( false ) );
				out->threads.push( Thread::create(
					[ out , i ]()
				{
					while( true )
					{
						//out->start_work_signal.wait();
						out->signal_per_threads[ i ].wait();
						out->signal_per_threads[ i ].reset();
						//OS::IO::debugLogln( "worker thread " , i , " woked up" );
						if( !out->working_flag.isSet() )
						{
							return;
						}
						Result< WorkDesc > res;
						while( !out->work_pool_per_threads[ i ].isEmpty() )
						{
							
							auto work_desc = out->work_pool_per_threads[ i ].pop();
							work_desc.work( work_desc.work_offset , work_desc.work_size );
							if( --out->work_in_progress == 0 )
							{
								out->end_work_signal.signal();
								//OS::IO::debugLogln( "end of work signaled" );
								break;
							}
							
						}
						
						/*if( out->dispatching_flag.isSet() )
						{
							out->dispatching_flag.reset();
							out->start_work_signal.signal();
							out->start_work_signal.reset();
						}*/
						
						/*if( out->transaction_flag.capture() )
						{
							
							
							out->transaction_flag.reset();
						}*/
					}
				}
				) );
			}
			return Unique< ThreadPool >( out , allocator );
		}
		void dispatch( GrainedWork work , uint work_size , uint work_per_thread )
		{
			uint offset = 0;
			//while( !transaction_flag.capture() );
			end_work_signal.reset();
			dispatching_flag.set();
			ito( work_size / work_per_thread )
			{
				work_in_progress++;
				work_pool_per_threads[ i % threads.size ].push( { offset , work_per_thread , work } );
				offset += work_per_thread;
			}
			if( work_size % work_per_thread != 0 )
			{
				work_in_progress++;
				work_pool_per_threads[ 0 ].push( { offset , work_size % work_per_thread , work } );
			}
			//transaction_flag.reset();
			ito( threads.size )
			{
				signal_per_threads[ i ].signal();
			}
			//start_work_signal.signal();
			//OS::IO::debugLogln( "start work signaled" );
			
		}
		void wait()
		{
			end_work_signal.wait();
			end_work_signal.reset();
		}
		~ThreadPool()
		{
			working_flag.reset();
			ito( threads.size )
			{
				signal_per_threads[ i ].signal();
			}
		}
	};
}