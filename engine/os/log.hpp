#pragma once
/*namespace Collections
{
	class String;
	std::ostream& operator<<( std::ostream& os , Collections::String const &string );
}*/
namespace OS
{
	namespace IO
	{
		
		template< typename F , typename ...T >
		struct DebugLog
		{
			static void log( F  const &first , T const & ...   argv )
			{
				DebugLog< F >::log( first );
				DebugLog< T... >::log( argv... );
			}
			static void log()
			{
			}
		};
		template< typename T >
		struct DebugLog< T >
		{
			static void log( T const & argv );
		};
		template< int N >
		struct DebugLog< const char (&)[ N ] >
		{
			static void log( const char (&argv)[ N ] )
			{
				DebugLog< char const * >::log( argv );
			}
		};
		template< int N >
		struct DebugLog< char[ N ] >
		{
			static void log( const char argv[ N ] )
			{
				DebugLog< char const * >::log( argv );
			}
		};
		template<>
		struct DebugLog< char * >
		{
			static void log( char * const &argv )
			{
				DebugLog< char const * >::log( argv );
			}
		};
		template<>
		struct DebugLog< unsigned char const * >
		{
			static void log( unsigned char const * const &argv )
			{
				DebugLog< char const * >::log( ( char const * )argv );
			}
		};
		template< typename ...T >
		void debugLog( T const & ... argv )
		{
			DebugLog< T... >::log( argv... );
		}
		template< typename ...T >
		void debugLogln( T const & ... argv )
		{
			DebugLog< T... >::log( argv... );
			DebugLog< const char * >::log( "\n" );
		}
		static void debugLogln()
		{
			DebugLog< const char * >::log( "\n" );
		}
	}
}