#pragma once
#define RETURN_ASSERT( x )\
if( !(x) )\
{\
	OS::IO::debugLogln( "assertation: " , #x , " failed\n" );\
	return false;\
}