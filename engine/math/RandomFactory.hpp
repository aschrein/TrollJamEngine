#pragma once
#include <engine/math/vec.hpp>
#include <engine/math/Math.hpp>
namespace Math
{
	template< typename T >
	class RandomFactory
	{
	public:
		typedef TVector< 3 , T > vec3;
		typedef TVector< 2 , T > vec2;
		static vec3 getRandomHalfSphere( float k )
		{
			float phi = MathUtil<T>::randomUniform() * Math<T>::PI * 2.0f;
			float cp = MathUtil<T>::cos( phi );
			float sp = MathUtil<T>::sin( phi );
			float ct = MathUtil<T>::randomUniform();
			ct = MathUtil<T>::sin( ct * PI * 0.5f ) * k + ct * ( 1.0f - k );
			float st = MathUtil<T>::sqrt( 1.0f - ct * ct );
			return vec3( st * cp , st * sp , ct );
		}
		static vec3 convert( vec3 const &t , vec3 const &b , vec3 const &n , vec3 const &k )
		{
			return b * k.x + t * k.y + n * k.z;
		}
		static vec3 getRandomOnSphereArea()
		{
			float phi = getRandom() * PI * 2.0f;
			float cp = std::cos( phi );
			float sp = std::sin( phi );
			float ct = 2.0f * getRandom() - 1.0f;
			float st = std::sqrt( 1.0f - ct * ct );
			return vec3( st * cp , st * sp , ct );
		}
		static vec3 getRandomInSphere()
		{
			float phi = getRandom() * PI * 2.0f;
			float cp = std::cos( phi );
			float sp = std::sin( phi );
			float ct = 2.0f * getRandom() - 1.0f;
			float st = std::sqrt( 1.0f - ct * ct );
			return sqrtf( getRandom() ) * vec3( st * cp , st * sp , ct );
		}
		static vec2 getRandomCircle()
		{
			float phi = getRandom() * PI * 2.0f;
			float r = powf( getRandom() , 0.5f );
			return vec2( cos( phi ) , sin( phi ) ) * r;
		}
		static vec3 getReflected( vec3 const &v , vec3 const &n )
		{
			return v - 2.0f * n * ( n * v );
		}
	};
}
