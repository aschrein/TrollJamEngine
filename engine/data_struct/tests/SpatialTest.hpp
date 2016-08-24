#pragma once
#include <engine/math/RandomFactory.hpp>
#include <engine/data_struct/KDTree.hpp>
#include <engine/mem/Allocators.hpp>
#include <engine/util/TestUtil.hpp>
#include <engine/os/Async.hpp>
bool QTreeTest()
{
	using namespace Collections;
	using namespace OS::Async;
	{
		Allocators::PoolAllocator linear_allocator( 1000 );
		KDTree< size_t , float , 1 > qtree( { 0.0f } , 100.0f , &linear_allocator );
		ito( 1000 )
		{
			qtree.push( i , { MathUtil< float >::randomUniform() * 100.0f , 0.0f } );
		}
		uint total_size = 0;
		ito( 2 )
		{
			f3 new_pos;
			jto( 1 )
			{
				new_pos[ j ] = ( int( i >> j ) & 1 ) * 2 - 1;
			}
			{
				auto quart = qtree.getCollided( { new_pos.x * 50.0f , 50.0f } );
				total_size += quart.getItemsCount();
			}
		}
		RETURN_ASSERT( total_size == 1000 );
	}
	{
		Allocators::PoolAllocator linear_allocator( 1000 );
		KDTree< size_t , float , 2 > qtree( { 0.0f , 0.0f } , 100.0f , &linear_allocator );
		ito( 1000 )
		{
			qtree.push( i , { RandomFactory< float >::getRandomCircle() * 100.0f , { 0.0f , 0.0f } } );
		}
		uint total_size = 0;
		ito( 4 )
		{
			f3 new_pos;
			jto( 2 )
			{
				new_pos[ j ] = ( int( i >> j ) & 1 ) * 2 - 1;
			}
			{
				auto quart = qtree.getCollided( { new_pos.x * 50.0f , new_pos.y * 50.0f , 50.0f , 50.0f } );
				total_size += quart.getItemsCount();
			}
		}
		RETURN_ASSERT( total_size == 1000 );
	}
	{
		Allocators::PoolAllocator linear_allocator( 1000 );
		KDTree< size_t , float , 3 > qtree( { 0.0f , 0.0f , 0.0f } , 100.0f , &linear_allocator );
		{
			auto origin = Timer::getCurrentTimeMilis();
			ito( 5000 )
			{
				qtree.push( i , { RandomFactory< float >::getRandomInSphere() * 100.0f , f3{ 0.0f , 0.0f , 0.0f } } );
			}
			auto end = Timer::getCurrentTimeMilis();
			OS::IO::debugLogln( "adding 5000 elems " , end - origin );
		}
		uint total_size = 0;
		ito( 8 )
		{
			f3 new_pos;
			jto( 3 )
			{
				new_pos[ j ] = ( int( i >> j ) & 1 ) * 2 - 1;
			}
			{
				auto quart = qtree.getCollided( { new_pos.x * 50.0f , new_pos.y * 50.0f , new_pos.z * 50.0f , 50.0f , 50.0f , 50.0f } , &linear_allocator );
				total_size += quart.getItemsCount();
			}
		}
		RETURN_ASSERT( total_size == 5000 );
	}
	size_t t = sizeof( KDTreeNode< size_t , float , 3 > );
	OS::IO::debugLogln( "QTreeTest success" );
	return true;
}