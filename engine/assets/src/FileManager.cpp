#include <stdafx.h>
#include <engine/assets/BitMap.hpp>
#include <engine/assets/FileManager.hpp>
using namespace Collections;
using namespace Options;
using namespace Pointers;
using namespace OS::Async;
BitMap2D FileManager::mapTGA( FileImage *file )
{
	BitMap2D out;
	file->setPosition( 12 );
	out.width = file->getInc< uint16_t >();
	out.height = file->getInc< uint16_t >();
	byte bpp = file->getInc< byte >();
	out.data = ( uint8_t* )file->getRaw() + 18;
	if( bpp == 8 )
	{
		out.pixel_type = PixelType::BYTE;
		out.pixel_mapping = PixelMapping::R;
	} else if( bpp == 16 )
	{
		out.pixel_type = PixelType::FIVE;
		out.pixel_mapping = PixelMapping::BGRA;
	} else
	{
		out.pixel_mapping = bpp == 32 ? PixelMapping::BGRA : PixelMapping::BGR;
		out.pixel_type = PixelType::BYTE;
	}
	return out;
}
FileManager *FileManager::create( Allocator *allocator )
{
	FileManager *out = allocator->alloc< FileManager >();
	new( out ) FileManager();
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
	working_flag.reset();
	signal.signal();
}
void FileManager::loadFile(
	Array< String > filenames ,
	FileConsumer *update_acceptor ,
	Allocator *allocator
)
{
	PromiseGiven promise_given;
	promise_given.filenames = filenames;
	promise_given.allocator = allocator;
	for( auto &filename : filenames )
	{
		addSubscriber( filename , update_acceptor );
	}
	promise_pool.push( promise_given );
	signal.signal();
}
void FileManager::mainLoop()
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
		Result< PromiseGiven > promise_result;
		while( ( promise_result = promise_pool.pop() ).isPresent() )
		{
			auto promise_given = promise_result.getValue();
			for( auto const &filename : promise_given.filenames )
			{
				auto fres = OS::Files::load( dir_name + "/" + filename , promise_given.allocator );
				if( fres.isPresent() )
				{
					Array< FileConsumer* > consumers = subscribers.get( filename ).getValue();
					for( auto &subscriber : consumers )
					{
						subscriber->pushEvent( { filename , FileEvent::EventType::LOADED , std::move( fres.getValue() ) } );
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