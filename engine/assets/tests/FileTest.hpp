#pragma once
#include <assets/FileManager.hpp>

struct TestStructC
{
	int counter = 0;
	TestStructC()
	{
		counter = 1;
		//cout << "created id:" << counter++ << endl;
	}
	TestStructC( TestStructC const & )
	{

		counter = 1;
		//cout << "created id:" << counter++ << endl;
	}
	TestStructC &operator=( TestStructC const & )
	{
		counter = 1;
		//cout << "created id:" << counter++ << endl;
		return *this;
	}
	TestStructC( TestStructC &&rx )
	{
		counter = 1;
		//cout << "created id:" << counter++ << endl;
	}
	TestStructC &operator=( TestStructC && )
	{
		//cout << "created id:" << counter++ << endl;
		return *this;
	}
	~TestStructC()
	{
		counter = 0;
		//cout << "deleted id:" << --counter << endl;
	}
};
Shared< TestStructC > getStruct()
{
	TestStructC *out = ( TestStructC * )Allocators::Allocator::singleton->alloc( sizeof( TestStructC ) );
	new( out ) TestStructC;
	return out;
}
Shared< TestStructC > getStruct1( Shared< TestStructC > cp )
{
	return cp;
}
bool FileTest()
{
	//auto t = getStruct1( getStruct() );
	//OS::IO::debugLogln( t->counter );
	FileManager *file_manager = FileManager::create();
	file_manager->loadFile( "test.txt" );
	file_promise->waitTillReady();
	auto file_result = file_promise->get();
	if( file_result[ 0 ].isPresent() )
	{
		auto file = file_result[ 0 ].getValue();
		OS::IO::debugLogln( "file opened with size " , file.getLimit() );
		OS::IO::debugLogln( ( char const * )file.getRaw() );
	} else
	{
		OS::IO::debugLogln( "file was not loaded" );
	}
	//OS::Async::Thread::sleep( 1000 );
	//delete file_manager;
	return true;
}