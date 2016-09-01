#include <stdafx.h>
#include <engine/assets/BitMap.hpp>
#include <engine/assets/FileManager.hpp>
using namespace Collections;
using namespace Options;
using namespace Pointers;
using namespace OS::Async;
struct PromiseGiven
{
	Array< String > filenames;
	FileConsumer *file_consumer;
};
class FileManagerImpl : public FileManager
{
public:
	Allocator *allocator;
	Unique< OS::Async::Thread > thread;
	Signal signal;
	AtomicFlag working_flag;
	static const int MAX_PROMISES = 0x100;
	HashMap< String , Array< FileConsumer* > > subscribers;
	HashMap< String , Shared< FileImage > > present_files;
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
	void mainLoop();
	FileManagerImpl() = default;
};
FileManager *FileManager::create( Allocator *allocator )
{
	FileManagerImpl *out = allocator->alloc< FileManagerImpl >();
	new( out ) FileManagerImpl();
	out->working_flag.set();
	out->allocator = allocator;
	out->thread = OS::Async::Thread::create(
		[ out ]()
	{
		out->mainLoop();
	}
	);
	return out;
}
FileManager::~FileManager()
{
	FileManagerImpl *thisimpl = ( FileManagerImpl* )this;
	thisimpl->working_flag.reset();
	thisimpl->signal.signal();
}
void FileManager::loadFile(
	Array< String > filenames ,
	FileConsumer *update_acceptor ,
	Allocator *allocator
)
{
	FileManagerImpl *thisimpl = ( FileManagerImpl* )this;
	PromiseGiven promise_given;
	promise_given.filenames = filenames;
	promise_given.file_consumer = update_acceptor;
	thisimpl->promise_pool.push( promise_given );
	thisimpl->signal.signal();
}
void FileManagerImpl::mainLoop()
{
	OS::IO::debugLogln( "file manager started execution" );
	
	String dir_name = "../../assets";
#ifdef _WIN32
	char buf[ 2048 ];
	char filename[ MAX_PATH ];
	HANDLE dir_handle = CreateFile( dir_name.getChars() , GENERIC_READ | FILE_LIST_DIRECTORY ,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE ,
		NULL , OPEN_EXISTING , FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED ,
		NULL );
	String last_modified_file;
	OVERLAPPED pooling_overlap;
	pooling_overlap.hEvent = CreateEvent( NULL , TRUE , FALSE , NULL );
	pooling_overlap.OffsetHigh = 0;
	DWORD result_size;
	auto file_event = allocator->alloc< FileEvent >();
	ReadDirectoryChangesW( dir_handle , &buf , sizeof( buf ) , TRUE , FILE_NOTIFY_CHANGE_LAST_WRITE , &result_size , &pooling_overlap , NULL );
#endif
	while( working_flag.isSet() )
	{
		while( !promise_pool.isEmpty() )
		{
			auto promise_given = promise_pool.pop();
			for( auto const &filename : promise_given.filenames )
			{
				addSubscriber( filename , promise_given.file_consumer );
				if( present_files.contains( filename ) )
				{
					promise_given.file_consumer->pushEvent( { filename , FileEvent::EventType::LOADED , present_files.get( filename ).getValue() } );
				} else
				{
					auto fres = OS::Files::load( dir_name + "/" + filename , allocator );
					if( fres.isPresent() )
					{
						FileImage *file_image_ptr = allocator->alloc< FileImage >();
						new( file_image_ptr ) FileImage( std::move( fres.getValue() ) );
						Shared< FileImage > new_file( file_image_ptr , allocator );
						present_files.push( filename , new_file );
						Array< FileConsumer* > consumers = subscribers.get( filename ).getValue();
						for( auto &subscriber : consumers )
						{
							subscriber->pushEvent( { filename , FileEvent::EventType::LOADED , new_file } );
						}
					} else
					{
						Array< FileConsumer* > consumers = subscribers.get( filename ).getValue();
						for( auto &subscriber : consumers )
						{
							subscriber->pushEvent( { filename , FileEvent::EventType::FAILED } );
						}
					}
				}
				
			}
		}
#ifdef _WIN32
		signal.wait( 100 );
		signal.reset();
		auto res = WaitForSingleObject( pooling_overlap.hEvent , 1 );
		if( res == WAIT_OBJECT_0 )
		{
			FILE_NOTIFY_INFORMATION* notify_info;
			DWORD offset = 0;
			do
			{
				notify_info = ( FILE_NOTIFY_INFORMATION* )( buf + offset );
				if( notify_info->Action == FILE_ACTION_MODIFIED && last_modified_file != String( filename ) )
				{
					size_t converted;
					notify_info->FileName[ notify_info->FileNameLength / 2 ] = '\0';
					wcstombs_s( &converted , filename , MAX_PATH , notify_info->FileName , MAX_PATH );
					last_modified_file = filename;
					String full_filename = last_modified_file.replace( "\\" , "/" );
					auto res = subscribers.get( full_filename );
					if( res.isPresent() )
					{
						Array< FileConsumer* > consumers = subscribers.get( filename ).getValue();
						for( auto &subscriber : consumers )
						{
							subscriber->pushEvent( { filename , FileEvent::EventType::UPDATED } );
						}
					}
				}
				offset += notify_info->NextEntryOffset;
			} while( notify_info->NextEntryOffset );
			ResetEvent( pooling_overlap.hEvent );
			ReadDirectoryChangesW( dir_handle , &buf , sizeof( buf ) , TRUE , FILE_NOTIFY_CHANGE_LAST_WRITE , &result_size , &pooling_overlap , NULL );
		} else
		{
			last_modified_file = "";
		}
#else
		signal.wait();
		signal.reset();
#endif
		//OS::IO::debugLogln( "file manager woked up" );
	}
#ifdef _WIN32
	allocator->free( file_event );
#endif
	OS::IO::debugLogln( "file manager ended execution" );
}