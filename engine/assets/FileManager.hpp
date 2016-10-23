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
	Shared< FileImage > file_result;
	EventType type;
public:
	FileEvent( String filename , EventType type , Shared< FileImage > file_image ) :
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
typedef LockFree::Consumer< FileEvent , 100 > FileConsumer;
class FileManager
{
protected:
	NONMOVABLE( FileManager );
public:
	static FileManager *singleton;
	static FileManager *create( String asset_folder , Allocator *allocator = Allocator::singleton );
	~FileManager();
	void loadFile(
		Array< String > filenames ,
		FileConsumer *update_acceptor ,
		Allocator *allocator = Allocator::singleton
	);
	void subscribe( Array< String > filenames , FileConsumer *update_acceptor );
	void unsubscribe( Array< String > filenames , FileConsumer *update_acceptor );
};