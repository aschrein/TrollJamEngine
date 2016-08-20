#pragma once
#ifndef ito
#define ito( x ) for( int i = 0; i < x; i++ )
#endif
#ifndef jto
#define jto( x ) for( int j = 0; j < x; j++ )
#endif
#ifndef xfor
#define xfor( x , n ) for( int x = 0; x < n; x++ )
#endif
#include <stdint.h>
typedef uint8_t byte;
typedef uint32_t uint;
#define CUSTOMCTR_NONMOVABLE( CLASS_NAME )\
CLASS_NAME( CLASS_NAME && ) = delete;\
CLASS_NAME( CLASS_NAME const & ) = delete;\
CLASS_NAME &operator=( CLASS_NAME const & ) = delete;\
CLASS_NAME &operator=( CLASS_NAME && ) = delete;
#define NONMOVABLE( CLASS_NAME )\
CLASS_NAME() = default;\
CLASS_NAME( CLASS_NAME && ) = delete;\
CLASS_NAME( CLASS_NAME const & ) = delete;\
CLASS_NAME &operator=( CLASS_NAME const & ) = delete;\
CLASS_NAME &operator=( CLASS_NAME && ) = delete;
#define MOVABLE( CLASS_NAME )\
CLASS_NAME() = default;\
CLASS_NAME( CLASS_NAME && ) = default;\
CLASS_NAME( CLASS_NAME const & ) = default;\
CLASS_NAME &operator=( CLASS_NAME const & ) = default;\
CLASS_NAME &operator=( CLASS_NAME && ) = default;