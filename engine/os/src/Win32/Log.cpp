#include <stdafx.h>
namespace OS
{
	namespace IO
	{
		/*void STDIN::wait()
		{
		getchar();
		}*/
		template< typename T >
		void DebugLog< T >::log( T const &argv )
		{
			std::cout << argv;
			std::cout.flush();
		}
		template<>
		void DebugLog< Collections::String >::log( Collections::String const &argv )
		{
			if( argv.getLength() > 0 )
			{
				std::cout << argv.getChars();
			}
			//std::cout.flush();
		}
		template struct DebugLog< int >;
		template struct DebugLog< float >;
		template struct DebugLog< byte >;
		template struct DebugLog< const char * >;
		template struct DebugLog< uint64_t >;
		template struct DebugLog< uint32_t >;
		template struct DebugLog< Collections::String >;
	}
}