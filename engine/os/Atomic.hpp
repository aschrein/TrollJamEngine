#pragma once
#include <engine/util/defines.hpp>
namespace OS
{
	namespace Atomic
	{
		class AtomicCounter
		{
		private:
			mutable uint64_t counter = 0;
		public:
			NONMOVABLE( AtomicCounter );
			AtomicCounter( uint64_t value )
			{
				*this = value;
			}
			AtomicCounter &operator=( uint64_t value );
			uint64_t get() const;
			bool operator==( AtomicCounter const &c ) const;
			bool operator!=( AtomicCounter const &c ) const;
			unsigned long operator++();
			unsigned long operator++( int );
			unsigned long operator--();
			unsigned long operator--( int );
		};
		class AtomicFlag
		{
		private:
			long flag = 0;
		public:
			NONMOVABLE( AtomicFlag );
			//tryes to set flag returns false if fails and true on success
			bool capture();
			bool isSet();
			void set();
			void reset();
		};
	}
}