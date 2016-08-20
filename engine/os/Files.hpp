#include <engine/os/Async.hpp>
#include <engine/mem/Allocators.hpp>
#include <engine/data_struct/Buffer.hpp>
#include <engine/data_struct/String.hpp>
#include <engine/data_struct/Optional.hpp>
#include <memory>
#pragma once
namespace OS
{
	namespace Files
	{
		using namespace Collections;
		class FileImage : public BufferView
		{
		private:
			Allocator *allocator = Allocator::singleton;
		public:

			FileImage( Allocator *allocator , int size ) :
				BufferView( allocator->alloc( size ) , size ) ,
				allocator( allocator )
			{}
			FileImage() = default;
			FileImage( FileImage const & ) = delete;
			FileImage &operator=( FileImage const & ) = delete;
			FileImage( FileImage &&fi )
			{
				*this = std::move( fi );
			}
			FileImage &operator=( FileImage &&fi )
			{
				this->allocator = fi.allocator;
				this->data = fi.data;
				this->limit = fi.limit;
				fi.data = nullptr;
				fi.limit = 0;
				fi.allocator = nullptr;
				return *this;
			}
			~FileImage()
			{
				if( this->data )
				{
					allocator->free( this->data );
					this->pos = 0;
					this->limit = 0;
				}
			}
		};
		Options::Result< FileImage > load( String filename , Allocator *allocator = Allocator::singleton );
	}
}