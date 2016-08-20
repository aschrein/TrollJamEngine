#pragma once
#include <engine/mem/Allocators.hpp>
#include <engine/os/Atomic.hpp>
namespace Pointers
{
	template< typename T >
	struct PtrBase
	{
		T *ptr = nullptr;
		Allocators::Allocator *allocator = nullptr;
		int count = 0;
		void clear()
		{
			if( ptr )
			{
				ito( count )
				{
					ptr[ i ].~T();
				}
				allocator->free( ptr );
				ptr = nullptr;
			}
			allocator = nullptr;
			count = 0;
		}
		T &operator[]( int i )
		{
			return ptr[ i ];
		}
		T const &operator[]( int i ) const
		{
			return ptr[ i ];
		}
		T *operator->()
		{
			return ptr;
		}
		T const *operator->() const
		{
			return ptr;
		}
		T &operator*()
		{
			return *ptr;
		}
		T const &operator*() const
		{
			return *ptr;
		}
		T *get()
		{
			return ptr;
		}
		T const *get() const
		{
			return ptr;
		}
		operator bool() const
		{
			return ptr;
		}
		bool operator!() const
		{
			return !ptr;
		}
	};
	/*template< typename T >
	struct PtrBase< T[] >
	{
		T *ptr = nullptr;
		Allocators::Allocator *allocator;
		void clear()
		{
			if( ptr )
			{
				ito(  )
				allocator->free( ptr );
				ptr = nullptr;
			}
		}
		T &operator[]( int i )
		{
			return this->ptr[ i ];
		}
		T const &operator[]( int i ) const
		{
			return this->ptr[ i ];
		}
		operator bool() const
		{
			return ptr;
		}
		bool operator!() const
		{
			return !ptr;
		}
		T *get()
		{
			return ptr;
		}
	};*/
	template< typename T >
	class Unique : public PtrBase< T >
	{
	public:
		Unique() = default;
		Unique( T *ptr , Allocators::Allocator *allocator = Allocators::Allocator::singleton )
		{
			this->ptr = ptr;
			this->allocator = allocator;
			this->count = 1;
		}
		Unique( Unique const & ) = delete;
		Unique &operator=( Unique const & ) = delete;
		Unique( Unique &&up )
		{
			*this = std::move( up );
		}
		Unique &operator=( Unique &&up )
		{
			this->clear();
			this->ptr = up.ptr;
			this->allocator = up.allocator;
			this->count = up.count;
			up.ptr = nullptr;
			up.clear();
			return *this;
		}
		~Unique()
		{
			this->clear();
		}
		static Unique &&make( T *ptr )
		{
			return std::move( Unique( ptr ) );
		}
	};
	template< typename T >
	class Shared : public PtrBase< T >
	{
	private:
		OS::Atomic::AtomicCounter *counter;
	public:
		Shared() = default;
		Shared( T *ptr , Allocators::Allocator *allocator = Allocators::Allocator::singleton )
		{
			this->ptr = ptr;
			this->allocator = allocator;
			this->count = 1;
			counter = allocator->alloc< OS::Atomic::AtomicCounter >();
			new( counter ) OS::Atomic::AtomicCounter();
			*counter = 1;
		}
		Shared( Shared const &sp )
		{
			*this = sp;
		}
		Shared &operator=( Shared const &sp )
		{
			this->release();
			this->ptr = sp.ptr;
			this->allocator = sp.allocator;
			this->count = sp.count;
			counter = sp.counter;
			counter->operator++();
			return *this;
		}
		Shared( Shared &&up )
		{
			*this = std::move( up );
		}
		Shared &operator=( Shared &&up )
		{
			this->ptr = up.ptr;
			this->allocator = up.allocator;
			this->count = up.count;
			counter = up.counter;
			up.ptr = nullptr;
			up.release();
			return *this;
		}
		void release()
		{
			if( ptr )
			{
				if( counter->operator--( 0 ) == 0 )
				{
					//counter->~AtomicCounter();
					this->allocator->free( counter );
					this->clear();
				}
			}
			counter = nullptr;
			this->ptr = nullptr;
			this->count = 0;
			this->allocator = nullptr;
		}
		~Shared()
		{
			release();
		}
		static Shared &&make( T *ptr , Allocators::Allocator *allocator = Allocators::Allocator::singleton )
		{
			return std::move( Shared( ptr , allocator ) );
		}
	};
}