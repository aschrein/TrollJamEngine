#ifndef LIB_OTREE_OTREE_H_
#define LIB_OTREE_OTREE_H_
#include <linalg/vec.h>
#define DR 10.0f
#define MAX_COUNT 40
#define MAX_DEPTH 5
#include <mem/Array.h>
struct Bound
{
	f3 pos , size;
};
class ONode
{
public:
	struct Item
	{
		int indx;
		Bound bound;$TM bool operator==( Item const &l ) const
		{
			return indx == l.indx;
		}
	};
	typedef Array< Item , MAX_COUNT * 4 > ItemArray;
private:
	ItemArray items;
	f3 pos;
	float dr;
	bool container = true;
	ONode *childs[ 8 ] = { nullptr };
	int depth;$TM void pushChildren( Item const &Item )
	{
		for( int i = 0; i < 8; i++ )
		{
			if( childs[ i ]->contains( Item ) )
			{
				childs[ i ]->push( Item );
			}
		}
	}
	$TM void slice()
	{
		ito( items.size() )
		{
			pushChildren( items[ i ] );
		}
		items.clear( );
		container = false;
	}
	const float EPS = 1.0e-3f;
public:
	$TM void operator=( ONode const & ) = default;
	$TM ONode( ONode const & ) = default;
	$TM bool intersectsRay( f3 const &rp , f3 const &rv ) const
	{
		ito( 3 )
		{
			if( fabsf( rv[ i ] ) > 0.0f )
			{
				if( rv[ i ] * ( pos[ i ] - rp[ i ] ) > 0.0f )
				{
					float t1 = ( pos[ i ] - dr - rp[ i ] ) / rv[ i ];
					float t2 = ( pos[ i ] + dr - rp[ i ] ) / rv[ i ];
					f3 dr = min( t1 , t2 ) * rv + rp - pos;
					jto( 3 )
					{
						if( j == i && j < 2 )
							j++;
						if( fabsf( dr[ j ] ) > getSize( ) + EPS )
							goto con;
					}
					return true;
					con: ;
				}
			}
		}
		return false;
	}
	$TM bool contains( Item const &Item ) const
	{
		f3 dr = Item.bound.pos - pos;
		ito( 3 )
			if( abs( dr[ i ] ) - Item.bound.size[ i ] > getSize( ) + EPS )
				return false;
		return true;
	}
	$TM f3 getPos() const
	{
		return pos;
	}
	$TM ItemArray const &getItems() const
	{
		return items;
	}
	$TM int getItemCount() const
	{
		return items.size( );
	}
	$TM float getSize() const
	{
		return dr;
	}
	$TM ONode( f3 const &p , float const s , int d ) :
			pos( p ), dr( s ), depth( d )
	{
	}
	$TM void release()
	{
		ito( 8 )
		{
			if( childs[ i ] != nullptr )
			{
				ito( 8 )
				{
					childs[ i ]->release( );
					delete childs[ i ];
				}
			}
		}
	}
	$TM void push( Item const &Item )
	{
		if( container )
		{
			items.push_back( Item );
			if( getItemCount( ) > MAX_COUNT && depth <= MAX_DEPTH )
			{
				for( int i = 0; i < 8; i++ )
				{
					childs[ i ] = new ONode( pos + f3( ( 1 - 2 * ( i & 1 ) ) * dr * 0.5f , ( 1 - 2 * ( ( i >> 1 ) & 1 ) ) * dr * 0.5f , ( 1 - 2 * ( ( i >> 2 ) & 1 ) ) * dr * 0.5f ) , dr * 0.5f ,
							depth + 1 );
				}
				slice( );
			}
		} else
		{
			pushChildren( Item );
		}
	}
	template< int N >
	$TM void fillListRay( f3 const &rp , f3 const &rv , SortedSet< ONode const* , N > &out ) const
	{
		if( container && intersectsRay( rp , rv ) )
		{
			out.push_back( this , pos.g_dist2( rp ) );
		} else
		{
			for( int i = 0; i < 8; i++ )
			{
				childs[ i ]->fillListRay( rp , rv , out );
			}
		}
	}
	$TM void execRecursively( std::function< void( ONode const & ) > func , std::function< void( Item const & ) > item_func ) const
	{
		func( *this );
		if( !container )
		{
			for( int i = 0; i < 8; i++ )
			{
				childs[ i ]->execRecursively( func , item_func );
			}
		} else
		{
			ito( items.size() )
			{
				item_func( items[ i ] );
			}
		}
	}
	$TM int getByteSize() const
	{
		int self_size = sizeof(ONode);
		int child_size = 0;
		if( childs[ 0 ] != nullptr )
		{
			ito( 8 )
				child_size += childs[ i ]->getByteSize( );
		}
		return self_size + child_size;
	}
	$TM int getTreeSize() const
	{
		return getByteSize( ) / sizeof(ONode);
	}
	$TM int fillCompressed( ONode *array , int const start_pos )
	{
		int filled = 1;
		array[ start_pos ] = *this;
		if( childs[ 0 ] != nullptr )
		{
			ito( 8 )
			{
				filled += childs[ i ]->fillCompressed( array , start_pos + filled );
			}
		}
		return filled;
	}
};
#endif /* LIB_OTREE_OTREE_H_ */
