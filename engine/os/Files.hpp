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
		class ImmutableFileView
		{
		protected:
			mutable uint pos = 0;
			uint limit = 0;
			byte const *data = nullptr;
		public:
			ImmutableFileView() = default;
			ImmutableFileView( void const *data , uint limit ) :
				data( ( byte const* )data ) ,
				limit( limit )
			{}
			void const *getRaw() const
			{
				return data;
			}
			template< typename T >
			T get() const
			{
				pos -= sizeof( T );
				return *( T* )( data + pos );
			}
			template< typename T >
			T getInc() const
			{
				T val = *( T* )( data + pos );
				pos += sizeof( T );
				return val;
			}
			uint getLimit() const
			{
				return limit;
			}
			void reset() const
			{
				pos = 0;
			}
			void setPosition( uint p ) const
			{
				pos = p;
			}
		};
		class FileImage
		{
		private:
			uint limit = 0;
			byte *data = nullptr;
			Allocator *allocator = Allocator::singleton;
		public:
			FileImage( byte *data , Allocator *allocator , uint size ) :
				data( data ) ,
				limit( size ) ,
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
				this->~FileImage();
				this->allocator = fi.allocator;
				this->data = fi.data;
				this->limit = fi.limit;
				fi.data = nullptr;
				fi.limit = 0;
				fi.allocator = nullptr;
				return *this;
			}
			FileImage copy( Allocator *allocator )
			{
				FileImage out;
				out.allocator = allocator;
				out.data = ( byte* )allocator->alloc( this->limit );
				Allocator::copy< byte >( out.data , this->data , this->limit );
				out.limit = this->limit;
				return out;
			}
			ImmutableFileView const getView() const
			{
				return ImmutableFileView( data , limit );
			}
			~FileImage()
			{
				if( this->data )
				{
					allocator->free( this->data );
					this->limit = 0;
				}
			}
		};
		Options::Result< FileImage > load( String filename , Allocator *allocator = Allocator::singleton );
		void save( String filename , void const *data , uint length );
	}
}