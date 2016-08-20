#pragma once
#include <stdafx.h>
#include <test_util/TestUtil.hpp>
#include <os/log.hpp>
#include <os/Async.hpp>
#include <data_struct/tests/CollectionTest.hpp>
using namespace std;
using namespace Collections;
using namespace Allocators;
//__declspec( thread ) StaticAllocator *ThreadLocalAllocator::static_allocator;

bool AVLTreeTest()
{
	//ThreadLocalAllocator::static_allocator = new StaticAllocator( 1000 );
	{
		LinearAllocator *static_allocator = new LinearAllocator( 10000 );
		BinaryTree< String > tree ( static_allocator );
		int N = 10000;
		Array< int > arr;
		arr.setAllocator( static_allocator );
		arr.make_space( N );
		{
			auto start_time = OS::Async::Timer::getCurrentTimeMilis();
			ito( N )
			{
				arr.push( i );
				tree.pushUnique( i );
			}
			auto end_time = OS::Async::Timer::getCurrentTimeMilis();
			//OS::IO::debugLogln( "addition takes " , end_time - start_time , "ms" );
		}
		ito( N )
		{
			int val = rand() % N;
			tree.remove( val );
			arr[ val ] = -1;
		}
		/*tree.print();
		arr.print();
		OS::IO::debugLogln( "______________" );
		for( auto const &item : tree )
		{
			OS::IO::debugLog( item , " " );
		}
		OS::IO::debugLogln( ":" , tree.getSize() );*/
		/*int val = -1;
		for( auto const &item : tree )
		{
			RETURN_ASSERT( item > val );
			val = item;
		}*/
		int count_passed = 0;
		int positive_count = 0;
		RETURN_ASSERT( arr.allMatch(
			[ & ]( int const &v )
		{
			bool success = false;
			if( v != -1 )
			{
				positive_count++;
				success = tree.contains( v );
			} else
			{
				success = !tree.contains( v );
			}
			if( !success )
			{
				OS::IO::debugLogln( v , "," , count_passed );
			} else
			{
				count_passed++;
			}
			return success;
		}
		) );
		RETURN_ASSERT( tree.getSize() == positive_count );
		//OS::IO::debugLogln( "memory used:" , static_allocator->getPosition() );
		//stress test
		/*while( true  )
		{
		ito( 10000 )
		{
		tree.pushUnique( i );
		}
		ito( 10000 )
		{
		tree.remove( i );
		}
		OS::Async::sleep( 300 );
		//RETURN_ASSERT( tree.getSize() == 0 );
		}*/
	}
	{
		BinaryTree< int > tree;
		int N = 100;
		ito( N )
		{
			tree.pushUnique( i );// rand() % 100 );
		}
		//tree.print();
		/*OS::IO::debugLogln( "______________" );
		for( auto const &item : tree )
		{
			OS::IO::debugLog( item , " " );
		}
		OS::IO::debugLogln( ":" , tree.getSize() );*/
	}
	{
		BinaryTree< String > tree;
		tree.pushUnique( "apple" );
		tree.pushUnique( "orange" );
		tree.pushUnique( "banana" );
		tree.pushUnique( "pussy" );
		RETURN_ASSERT( tree.contains( "pussy" ) );
		//tree.print();
	}
	OS::IO::debugLogln( "BinaryTree success" );
	return true;
}
bool SetTest()
{
	{
		HashSet< int > hashset;
		hashset.push( 0 ).push( 1 );
		ito( 100 )
		{
			hashset.push( i );
		}
		RETURN_ASSERT( hashset.getItemsCount() == 100 );
		RETURN_ASSERT( hashset.contains( 0 ) && hashset.contains( 1 ) && !hashset.contains( -1 ) );
	}
	{
		HashSet< String > outer;
		{
			HashSet< String > hashset;
			hashset.push( "test" ).push( "test1" );
			RETURN_ASSERT( hashset.getItemsCount() == 2 );
			RETURN_ASSERT( hashset.contains( "test" ) && hashset.contains( "test1" ) && !hashset.contains( "test2" ) );
			outer = std::move( hashset );
			RETURN_ASSERT( !hashset.contains( "test" ) && !hashset.contains( "test1" ) );
			RETURN_ASSERT( outer.contains( "test" ) && outer.contains( "test1" ) );
			hashset.push( "hello" );
			RETURN_ASSERT( hashset.contains( "hello" ) );
			hashset.remove( "hello" );
			RETURN_ASSERT( !hashset.contains( "hello" ) );
			/*while( true )
			{
			HashSet< int > tmp;
			ito( 10000 )
			{
			tmp.push( i );
			}
			ito( 1000 )
			{
			tmp.remove( i );
			}

			}*/
		}
		{
		}
	}
	{
		HashMap< String , String > map;
		map.push( "banana" , "yellow" );
		map.push( "apple" , "green" );
		RETURN_ASSERT( map.get( "banana" ).getValue() == "yellow" );
		RETURN_ASSERT( map.get( "apple" ).isPresent() );
		RETURN_ASSERT( !map.get( "orange" ).isPresent() );
		//map.print();
	}
	OS::IO::debugLogln( "hash set test success" );
	return true;
}