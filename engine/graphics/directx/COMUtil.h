#ifndef COMUTIL_H
#define COMUTIL_H
#define COMRelease( x ) if( x != NULL )\
	{\
		x->Release();\
		x = NULL;\
	}
#endif