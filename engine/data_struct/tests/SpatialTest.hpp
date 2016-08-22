#pragma once
#include <engine/math/RandomFactory.hpp>
#include <engine/data_struct/QTree.hpp>
#include <engine/mem/Allocators.hpp>
bool QTreeTest()
{
	using namespace Collections;
	Allocators::LinearAllocator linear_allocator( 1000 );
	QTree< size_t , float > qtree( { 0.0f , 0.0f } , 100.0f , &linear_allocator );
	ito( 1000 )
	{
		qtree.push( i , f4( RandomFactory< float >::getRandomCircle() * 100.0f , f2{ 1.0f , 1.0f } ) );
	}
	Array< size_t > quart;
	qtree.getCollided( f4{ -50.f , -50.0f , 50.0f , 50.0f } , quart );
	OS::IO::debugLogln( "ff" );
	return true;
}