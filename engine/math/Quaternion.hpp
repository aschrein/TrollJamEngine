#pragma once
#include <engine/math\mat.hpp>
namespace Math
{
	template< typename T >
	struct Quaternion
	{
		union
		{
			T __data[ 4 ];
			struct
			{
				T x , y , z , w;
			};
		};
		CALLMOD Quaternion( T x , T y , T z , T w ) :
			x( x ) , y( y ) , z( z ) , w( w )
		{}
		CALLMOD Quaternion() :
			Quaternion( T( 0 ) , T( 0 ) , T( 0 ) , T( 0 ) )
		{}
		CALLMOD Quaternion( Quaternion const & ) = default;
		CALLMOD Quaternion( Quaternion && ) = default;
		CALLMOD Quaternion &operator=( Quaternion const & ) = default;
		CALLMOD Quaternion &operator=( Quaternion && ) = default;
		CALLMOD Quaternion( TVector< 3 , T > const &axis , T angle )
		{
			auto cos = MathUtil< T >::cos( angle / 2 );
			auto sin = MathUtil< T >::sin( angle / 2 );
			x = axis.x * sin;
			y = axis.y * sin;
			z = axis.z * sin;
			w = cos;
		}
		CALLMOD Quaternion( TVector< 3 , T > const &axis )
		{
			x = axis.x;
			y = axis.y;
			z = axis.z
			w = T( 0 );
		}
		CALLMOD Quaternion const operator+( Quaternion const &a ) const
		{
			return Quaternion( *this ) += a;
		}
		CALLMOD Quaternion &operator+=( Quaternion const &a )
		{
			x += a.x;
			y += a.y;
			z += a.z;
			w += a.w;
			return *this;
		}
		CALLMOD Quaternion const operator-( Quaternion const &a ) const
		{
			return Quaternion( *this ) -= a;
		}
		CALLMOD Quaternion &operator-=( Quaternion const &a )
		{
			x -= a.x;
			y -= a.y;
			z -= a.z;
			w -= a.w;
			return *this;
		}
		CALLMOD Quaternion &operator*=( T const &a )
		{
			x *= a;
			y *= a;
			z *= a;
			w *= a;
			return *this;
		}
		CALLMOD Quaternion const operator*( T const &a ) const
		{
			return Quaternion( *this ) *= a;
		}
		CALLMOD Quaternion &operator/=( T const &a )
		{
			auto ia = T( 1 ) / a;
			return *this *= ia;
		}
		CALLMOD Quaternion operator/( T const &a ) const
		{
			return Quaternion( *this ) /= a;
		}
		CALLMOD Quaternion const operator*( Quaternion const &a ) const
		{
			return Quaternion(
				x * a.x - y * a.y - z * a.z - w * a.w ,
				x * a.y + y * a.x + z * a.w - w * a.z ,
				x * a.z - y * a.w + z * a.x + w * a.y ,
				x * a.w + y * a.z - z * a.y + w * a.x
				);
		}
		CALLMOD Quaternion &operator*=( Quaternion const &a )
		{
			*this = *this * a;
			return *this;
		}
		CALLMOD Quaternion const operator/( Quaternion const &a ) const
		{
			return ( *this * !a ) / a.mod2();
		}
		CALLMOD Quaternion operator/=( Quaternion const &a )
		{
			return *this = *this * a;
		}
		CALLMOD T mod2() const
		{
			return x * x + y * y + z * z + w * w;
		}
		CALLMOD T mod() const
		{
			return MathUtil< T >::sqrt( mod2() );
		}
		CALLMOD TMatrix< T , 3 , 3 > getRotationMatrix() const
		{
			return TMatrix< T , 3 , 3 >(
				1 - 2 * y*y - 2 * z*z , 2 * x*y - 2 * z*w , 2 * x*z + 2 * y*w ,
				2 * x*y + 2 * z*w , 1 - 2 * x*x - 2 * z*z , 2 * y*z - 2 * x*w ,
				2 * x*z - 2 * y*w , 2 * y*z + 2 * x*w , 1 - 2 * x*x - 2 * y * y
				);
		}
		CALLMOD Quaternion operator!() const
		{
			return Quaternion( x , -y , -z , -w );
		}
		TVector< 3 , T > rotate( TVector< 3 , T > const &a ) const
		{
			return 
		}
	};
	template< typename T >
	inline Quaternion< T > operator*( T const &a , Quaternion< T > const &b )
	{
		return b * a;
	}
	template< typename T >
	inline Quaternion< T > lerp( Quaternion< T > const &a , Quaternion< T > const &b , float t )
	{
		float it = 1.0f - t;
		return Quaternion< T >( a.x * it + b.x * t , a.y * it + b.y * t , a.z * it + b.z * t , a.w * it + b.w * t );
	}
	typedef Quaternion< float > qf;
}