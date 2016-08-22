#pragma once
#include <engine/util/defines.hpp>
#ifndef __host__
#define __host__
#endif
#ifndef __device__
#define __device__
#endif
#define CALLMOD __host__ __device__ inline
#ifndef PACKED
#define PACKED __attribute__ ( ( __packed__ ) )
#endif
#include <engine/math/Math.hpp>
#include <engine/os/log.hpp>
namespace Math
{
	template< int N , typename T , typename P , typename ...S >
	struct unpacker
	{
		CALLMOD static void unpack( T *data , int C , P x , S ...arg )
		{
			data[ C - N ] = static_cast< T >( x );
			unpacker< N - 1 , T , S... >::unpack( data , C , arg... );
		}
	};
	template< typename T , typename P >
	struct unpacker < 1 , T , P >
	{
		CALLMOD static void unpack( T *data , int C , P x )
		{
			data[ C - 1 ] = static_cast< T >( x );
		}
	};
	template< int N , typename T >
	struct TVecBase
	{
		T __data[ N ];
	};
	template< int N , typename T >
	struct TVector;
	template< >
	struct TVecBase < 3 , float >
	{
		union
		{
			float __data[ 3 ];
			struct
			{
				float x , y , z;
			};
		};
	};
	template<>
	struct TVecBase < 3 , int >
	{
		union
		{
			int __data[ 3 ];
			struct
			{
				int x , y , z;
			};
		};
	};
	template<>
	struct TVecBase < 2 , int >
	{
		union
		{
			int __data[ 2 ];
			struct
			{
				int x , y;
			};
		};
	};
	template<>
	struct TVecBase < 4 , float >
	{
		union
		{
			float __data[ 4 ];
			struct
			{
				float x , y , z , w;
			};
		};
	};
	template<>
	struct TVecBase < 2 , float >
	{
		union
		{
			float __data[ 2 ];
			struct
			{
				float x , y;
			};
		};
	};
#define DATA this->__data
	template< int N , typename T >
	struct TVector : public TVecBase < N , T >
	{
		typedef MathUtil< T > M;
		CALLMOD TVector< 2 , T > xy() const
		{
			return{ x , y };
		}
		CALLMOD T const &operator[]( const int i ) const
		{
			return DATA[ i ];
		}
		CALLMOD T &operator[]( const int i )
		{
			return DATA[ i ];
		}
		void print() const
		{
			ito( N )
				OS::IO::debugLog( DATA[ i ] , " " );
			OS::IO::debugLogln();
		}
		template< typename ...D >
		CALLMOD TVector( D ...arg )
		{
			unpacker< N , T , D... >::unpack( this->__data , N , arg... );
		}
		CALLMOD TVector():
			TVector( T( 0 ) )
		{}
		CALLMOD TVector( TVector const & ) = default;
		CALLMOD TVector(TVector &&) = default;
		CALLMOD TVector &operator=( TVector const & ) = default;
		CALLMOD TVector &operator=( TVector && ) = default;
		CALLMOD TVector( T d )
		{
			ito( N )
				DATA[ i ] = d;
		}
		template< typename ...D >
		static TVector createVec( D ...arg )
		{
			TVector out;
			unpacker< N >::unpack( out.__data , N , arg... );
			return out;
		}
		CALLMOD TVector( TVector< N - 1 , T > const &v , T rest )
		{
			ito( N - 1 )
				DATA[ i ] = v[ i ];
			DATA[ N - 1 ] = rest;
		}
		template< int H >
		CALLMOD TVector( TVector< H , T > const &v1 , TVector< N - H , T > const &v2 )
		{
			ito( H )
				DATA[ i ] = v1[ i ];
			ito( N - H )
				DATA[ i + H ] = v2[ i ];
		}
		template< typename P >
		CALLMOD TVector( TVector< N , P > const &v )
		{
			ito( N )
				DATA[ i ] = static_cast< T >( v[ i ] );
		}
		template< typename P >
		CALLMOD void operator=( TVector< N , P > const &v )
		{
			ito( N )
				DATA[ i ] = static_cast< T >( v[ i ] );
		}
		CALLMOD TVector &mulInplace( float k )
		{
			ito( N )
				DATA[ i ] *= k;
			return *this;
		}
		CALLMOD TVector &addInplace( TVector const &v )
		{
			ito( N )
				DATA[ i ] += v[ i ];
			return *this;
		}
		CALLMOD TVector &subInplace( TVector const &v )
		{
			ito( N )
				DATA[ i ] -= v[ i ];
			return *this;
		}
		CALLMOD TVector &negInplace()
		{
			ito( N )
				DATA[ i ] *= T( -1 );
			return *this;
		}
		CALLMOD TVector &divInplace( T const &val )
		{
			return mulInplace( T( 1 ) / val );
		}
		CALLMOD T dist2( TVector const &v ) const
		{
			return ( *this - v ).mod2();
		}
		CALLMOD T dist( TVector< N , T > const &v ) const
		{
			return ( *this - v ).mod();
		}
		CALLMOD T mod2() const
		{
			T d = T( 0 );
			ito( N )
				d += M::sqr( DATA[ i ] );
			return d;
		}
		CALLMOD T mod() const
		{
			return M::sqrt( mod2() );
		}
		CALLMOD TVector norm() const
		{
			T m = mod();
			if( m < M::EPS )
				return TVector< N , T >( static_cast< T >( 0 ) );
			if( M::abs( m - 1.0f ) < M::EPS )
				return *this;
			return *this * ( 1.0f / m );
		}
		CALLMOD TVector operator&( TVector const &v ) const
		{
			TVector out;
			ito( N )
				out[ i ] = DATA[ i ] * v[ i ];
			return out;
		}
		CALLMOD TVector< N , T > operator-( TVector const &v ) const
		{
			TVector out = *this;
			return out -= v;
		}
		CALLMOD TVector &operator-=( TVector const &v )
		{
			ito( N )
				DATA[ i ] -= v[ i ];
			return *this;
		}
		CALLMOD TVector< N , T > operator+( TVector const &v ) const
		{
			TVector out = *this;
			return out += v;
		}
		CALLMOD TVector &operator+=( TVector const &v )
		{
			ito( N )
				DATA[ i ] += v[ i ];
			return *this;
		}
		CALLMOD TVector operator*( T k ) const
		{
			TVector out = *this;
			return out *= k;
		}
		CALLMOD TVector &operator*=( T k )
		{
			ito( N )
				DATA[ i ] *= k;
			return *this;
		}
		CALLMOD TVector operator/( T k ) const
		{
			TVector< N , T > out = *this;
			return out /= k;
		}
		CALLMOD TVector &operator/=( T k )
		{
			ito( N )
				DATA[ i ] /= k;
			return *this;
		}
		CALLMOD TVector operator/( TVector const &k ) const
		{
			TVector out = *this;
			return out /= k;
		}
		CALLMOD TVector &operator/=( TVector const &k )
		{
			ito( N )
				DATA[ i ] /= k[ i ];
			return *this;
		}
		CALLMOD float operator*( TVector const &v ) const
		{
			T out = 0.0f;
			ito( N )
				out += DATA[ i ] * v[ i ];
			return out;
		}
		CALLMOD bool operator==( TVector const &v ) const
		{
			ito( N )
				if( DATA[ i ] != v[ i ] )
					return false;
			return true;
		}
		CALLMOD bool operator!=( TVector const &v ) const
		{
			return !( *this == v );
		}
		CALLMOD TVector operator-() const
		{
			TVector out = *this;
			return out.negInplace();
		}
		CALLMOD TVector clamp( T const &min , T const &max ) const
		{
			TVector out;
			ito( N )
				out[ i ] = M::min( max , M::max( min , DATA[ i ] ) );
			return out;
		}
	};
	template< int N , typename T >
	CALLMOD TVector< N , T > operator*( float k , TVector< N , T > const &v )
	{
		TVector< N , T > out;
		ito( N )
			out[ i ] = v[ i ] * k;
		return out;
	}
	template< typename T >
	CALLMOD TVector< 2 , T > operator!( TVector< 2 , T > const &v )
	{
		return TVector< 2 , T >( -v.y , v.x );
	}
	template< typename T >
	CALLMOD TVector< 3 , T > perp_norm( TVector< 3 , T > const &v , TVector< 3 , T > const &p )
	{
		return ( p - v * ( p * v ) ).norm();
	}

	template< typename T >
	CALLMOD TVector< 3 , T > operator^( TVector< 3 , T > const &a , TVector< 3 , T > const &b )
	{
		return TVector< 3 , T >( a.y * b.z - b.y * a.z , b.x * a.z - a.x * b.z ,
			a.x * b.y - b.x * a.y );
	}
	template< typename T >
	CALLMOD TVector< 2 , T > rotate( TVector< 2 , T > const &v , float a )
	{
		float c = TVector< 2 , T >::M::cos( a );
		float s = TVector< 2 , T >::M::sin( a );
		return TVector< 2 , T >( v.x * c - v.y * s , v.y * c + v.x * s );
	}
	typedef TVector< 2 , float > f2;
	typedef TVector< 2 , int > i2;
	typedef TVector< 4 , int > i4;
	typedef TVector< 2 , unsigned int > u2;
	typedef TVector< 3 , float > f3;
	typedef TVector< 4 , float > f4;
	typedef TVector< 3 , unsigned char > b3;
	typedef TVector< 2 , unsigned short > short2;
	typedef TVector< 3 , int > i3;
#undef DATA
}
