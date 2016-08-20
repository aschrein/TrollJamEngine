#pragma once
#include <functional>
#include <engine/math\vec.hpp>
#include <algorithm>
namespace Rasterizer
{
	using namespace Math;
	class Surface
	{
	public:
		virtual i2 getSize() const = 0;
		virtual void setPixel( int x , int y , int val ) = 0;
	};
	class SurfaceRGB : public Surface
	{
	private:
		int w , h;
		char *data;
	public:
		void setPixel( int x , int y , int val )
		{
			data[ 3 * w * y + x ] = val >> 24 & 0xff;
			data[ 3 * w * y + x + 1 ] = val >> 16 & 0xff;
			data[ 3 * w * y + x + 2 ] = val >> 8 & 0xff;
		}
	};
	void drawLine( Surface *surface , i2 const &a , i2 const &b , std::function< int( float ) > filler )
	{
		int dy = b.y - a.y;
		int dx = b.x - a.x;
		if( dy * dy > dx * dx )
		{
			int sy = dy > 0 ? 1 : -1;
			int y = 0;
			float x = a.x;
			float sx = float( dx ) / dy;
			do
			{
				surface->setPixel( int( x + 0.5f ) , a.y + y , filler( float( sy ) / dy ) );
				y += sy;
				x += sx * sy;
			} while( y - sy != dy );
		} else
		{
			int sx = dx > 0 ? 1 : -1;
			int x = 0;
			float y = a.y;
			float sy = float( dy ) / dx;
			do
			{
				surface->setPixel( a.x + x , int( y + 0.5f ) , filler( float( sx ) / dx ) );
				y += sy * sx;
				x += sx;
			} while( x - sx != dx );
		}
	}
	void drawTriangle( Surface *surface , i2 const &a , i2 const &b , i2 const &c , std::function< int( f3 const & ) > filler )
	{
		i2 sorted[ 3 ] = { a , b , c };
		for( int i = 0; i < 2; i++ )
		{
			for( int j = i + 1; j < 3; j++ )
			{
				if( sorted[ i ].y > sorted[ j ].y )
				{
					auto tmp = sorted[ i ];
					sorted[ i ] = sorted[ j ];
					sorted[ j ] = tmp;
				}
			}
		}
		{
			i2 p1 = sorted[ 2 ] , p2 = sorted[ 2 ];

		}
	}
}
