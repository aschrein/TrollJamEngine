#pragma once
#include <windows.h>
#include <engine/os/Async.hpp>
#include <engine/data_struct/Optional.hpp>
#include <engine/data_struct/Tuple.hpp>
#include <engine/data_struct/Buffer.hpp>
#include <engine/data_struct/Array.hpp>
#include <engine/os/log.hpp>
#include <engine/os/Atomic.hpp>
#include <engine/assets/BitMap.hpp>
#include <engine/mem/Pointers.hpp>
#include <engine/data_struct/RingBuffer.hpp>
#include <engine/components/Event.hpp>
#include <engine/os/Files.hpp>
#include <engine/data_struct/Consumer.hpp>
using namespace Collections;
using namespace Options;
using namespace Pointers;
using namespace OS::Async;
using namespace OS::Atomic;
using namespace OS::Files;
class FileEvent
{
public:
	enum class EventType
	{
		UPDATED , LOADED , FAILED
	};
public:
	String filename;
	Result< FileImage > file_result;
	EventType type;
public:
	FileEvent( String filename , EventType type , FileImage &&file_image ) :
		filename( filename ) ,
		file_result( std::move( file_image ) ) ,
		type( type )
	{}
	FileEvent( String filename , EventType type ) :
		filename( filename ) ,
		type( type )
	{}
	FileEvent() = default;
	FileEvent( FileEvent const & ) = default;
	FileEvent& operator=( FileEvent const & ) = default;
	FileEvent( FileEvent && ) = default;
	FileEvent& operator=( FileEvent && ) = default;
};
//single consumer, single producer
typedef LockFree::Consumer< FileEvent > FileConsumer;
class FileManager
{
private:
	Allocator *allocator;
	NONMOVABLE( FileManager );
	Unique< OS::Async::Thread > thread;
	Signal signal;
	AtomicFlag working_flag;
	static const int MAX_PROMISES = 0x100;
	HashMap< String , Array< FileConsumer* > > subscribers;
	struct PromiseGiven
	{
		Array< String > filenames;
		Allocator *allocator;
	};
	void addSubscriber( String filename , FileConsumer *update_acceptor )
	{
		auto res = subscribers.get( filename );
		if( res.isPresent() )
		{
			res.getValue().push( update_acceptor );
		} else
		{
			Array< FileConsumer* > sarr;
			sarr.setAllocator( allocator );
			sarr.push( update_acceptor );
			subscribers.push( filename , std::move( sarr ) );
		}
	}
	LockFree::RingBuffer< PromiseGiven , MAX_PROMISES > promise_pool;
public:
	static FileManager *singleton;
	static BitMap2D mapTGA( FileImage *file );
	static FileManager *create( Allocator *allocator = Allocator::singleton );
	~FileManager();
	void loadFile(
		Array< String > filenames ,
		FileConsumer *update_acceptor ,
		Allocator *allocator = Allocator::singleton
	);
	void subscribe( Array< String > filenames , FileConsumer *update_acceptor );
	void unsubscribe( Array< String > filenames , FileConsumer *update_acceptor );
	void mainLoop();
};