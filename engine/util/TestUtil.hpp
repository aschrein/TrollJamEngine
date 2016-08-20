#pragma once
#define FATAL_ASSERT( x )\
if( !(x) )\
{\
	OS::IO::debugLogln( "assertation: " , #x , " failed\n" );\
	OS::IO::STDIN::wait(); \
	OS::exit(); \
}
#define RETURN_ASSERT( x )\
if( !(x) )\
{\
	OS::IO::debugLogln( "assertation: " , #x , " failed\n" );\
	return false;\
}