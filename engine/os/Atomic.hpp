#pragma once
#include <engine/util/defines.hpp>
namespace OS
{
	namespace Atomic
	{
		class AtomicCounter
		{
		private:
			volatile mutable int64_t counter = 0;
		public:
			NONMOVABLE( AtomicCounter );
			AtomicCounter( int64_t value )
			{
				*this = value;
			}
			AtomicCounter &operator=( int64_t value );
			int64_t get() const;
			bool operator==( AtomicCounter const &c ) const;
			bool operator!=( AtomicCounter const &c ) const;
			int64_t operator++();
			int64_t operator++( int );
			int64_t operator--();
			int64_t operator--( int );
		};
		class AtomicFlag
		{
		private:
			volatile mutable int64_t flag = 0;
		public:
			NONMOVABLE( AtomicFlag );
			//tryes to set flag returns false if fails and true on success
			bool capture();
			bool isSet() const;
			void set();
			void reset();
		};
	}
}