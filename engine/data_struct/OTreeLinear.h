/*
 * OTreeLinear.h
 *
 *  Created on: Oct 18, 2015
 *      Author: anton
 */

#ifndef OTREELINEAR_H_
#define OTREELINEAR_H_
#include <engine/math/vec.h>
#define DR 10.0f
#define MAX_COUNT 80
#define MAX_DEPTH 3
#include <functional>
#include <mem/Array.h>
struct Bound
{
	f3 pos , size;
};
struct Item
{
	int indx;
	Bound bound;$TM bool operator==( Item const &l ) const
	{
		return indx == l.indx;
	}
};
typedef Array< Item , MAX_COUNT * 2 > ItemBank;
template< typename T , int N >
struct BankArray
{
	T array[ N ];
public:
	$TM int findEmpty() const
	{
		ito( N )
		{
			if( array[ i ].isEmpty( ) )
			{
				return i;
			}
		}
		return -1;
	}
	$TM T &operator[]( const int i )
	{
		return array[ i ];
	}
	$TM T const &operator[]( const int i ) const
	{
		return array[ i ];
	}
	$TM void clear()
	{
		ito( N )
		{
			array[ i ].clear( );
		}
	}
};
class ONode;
template< int N , int M >
struct TreeData
{
	BankArray< ItemBank , N > item_bank_array;
	BankArray< ONode , M > node_bank_array;
public:
	$TM void clear()
	{
		item_bank_array.clear( );
		node_bank_array.clear( );
	}
};
class ONode
{
private:
	int self_id = 0;
	float dr = DR;
	f3 pos = { 0.0f };
	int depth = 0;
	int children_id[ 8 ] = { -1 , -1 , -1 , -1 , -1 , -1 , -1 , -1 };
	// -2 means not a container
	// -3 means empty node
	// -1 means empty container
	int item_id = -3;
	static constexpr float EPS = 1.0e-3f;
private:
	template< int N , int M >
	$TM void slice( TreeData< N , M > &tree_data )
	{
		ito( 8 )
		{
			children_id[ i ] = tree_data.node_bank_array.findEmpty( );
			ONode &child = tree_data.node_bank_array[ children_id[ i ] ];
			child.self_id = children_id[ i ];
			child.pos = pos + f3( ( 1 - 2 * ( i & 1 ) ) * dr * 0.5f , ( 1 - 2 * ( ( i >> 1 ) & 1 ) ) * dr * 0.5f , ( 1 - 2 * ( ( i >> 2 ) & 1 ) ) * dr * 0.5f );
			child.dr = dr * 0.5f;
			child.depth = depth + 1;
			child.item_id = -1;
		}
#ifndef __CUDACC__
		std::cout << "depth " << depth << "\n";
		printChilds();
		if( children_id[ 0 ] < 0 )
		exit( 0 );
#endif
		ItemBank &bank = tree_data.item_bank_array[ item_id ];
		ito( bank.count )
		{
			pushChildren( bank[ i ] , tree_data );
		}
		bank.clear( );
		item_id = -2;
	}
	template< int N , int M >
	$TM void pushChildren( Item const &item , TreeData< N , M > &tree_data )
	{
		jto( 8 )
		{
			tree_data.node_bank_array[ children_id[ j ] ].push( item , tree_data );
		}
	}
public:
	$TM void clear()
	{
		*this = ONode( );
	}
	$TM bool intersectsRay( f3 const &rp , f3 const &rv ) const
	{
		f3 d = pos - rp;
		float t = rv * d;
		if( t < -dr || d.g_mod2( ) - t * t > 3.0f * dr * dr )
		{
			return false;
		}
		ito( 3 )
		{
			if( fabsf( rv[ i ] ) > 0.0f )
			{
				xfor( sqn , 2 )
				{
					float t = ( pos[ i ] + ( 1 - 2 * sqn ) * dr - rp[ i ] ) / rv[ i ];
					if( t < 0.0f )
						continue;
					f3 _dr = t * rv + rp - pos;
					jto( 3 )
					{
						if( j == i && j < 2 )
							j++;
						if( fabsf( _dr[ j ] ) > getSize( ) + EPS )
							goto con;
					}
					return true;
					con: ;
				}
			}
		}
		return false;
	}
	void printChilds() const
	{
		ito( 8 )
		{
			std::cout << children_id[ i ] << "\n";
		}
	}
	$TM ONode( f3 const &p , float const s , int d ) :
			pos( p ), dr( s ), depth( d ), item_id( -1 )
	{
	}
	$TM void operator=( ONode const &n )
	{
		this->dr = n.dr;
		ito( 8 )
			this->children_id[ i ] = n.children_id[ i ];
		this->depth = n.depth;
		this->pos = n.pos;
		this->item_id = n.item_id;
	}
	$TM ONode() = default;
	$TM ONode( ONode const & ) = default;
	$TM float getSize() const
	{
		return dr;
	}
	$TM float getDepth() const
	{
		return depth;
	}
	$TM f3 getPos() const
	{
		return pos;
	}
	$TM bool contains( Item const &Item ) const
	{
		f3 dr = Item.bound.pos - pos;
		ito( 3 )
			if( fabsf( dr[ i ] ) - Item.bound.size[ i ] > getSize( ) + EPS )
				return false;
		return true;
	}
	$TM bool contains( f3 const &p ) const
		{
			f3 dr = p - pos;
			ito( 3 )
				if( fabsf( dr[ i ] ) > getSize( ) + EPS )
					return false;
			return true;
		}
	$TM bool isContainer() const
	{
		return item_id != -2;
	}
	$TM bool isEmpty() const
	{
		return item_id == -3;
	}
	template< int N , int M >
	$TM void push( Item const &item , TreeData< N , M > &tree_data )
	{
		if( contains( item ) )
		{
			if( isContainer( ) )
			{
				if( item_id < 0 )
				{
					item_id = tree_data.item_bank_array.findEmpty( );
				}
				auto &item_bank = tree_data.item_bank_array[ item_id ];
				item_bank.push_back( item );
				if( item_bank.size( ) >= MAX_COUNT && depth < MAX_DEPTH )
				{
					slice( tree_data );
				}
			} else
			{
				pushChildren( item , tree_data );
			}
		}
	}
	template< int N , int M >
	$TM ItemBank const &getItems( TreeData< N , M > const &tree_data ) const
	{
		return tree_data.item_bank_array[ item_id ];
	}
	template< int N , int K , int U >
	$TM static void fillListRay( ONode const *root , f3 const &rp , f3 const &rv , SortedSet< int , N > &out , TreeData< K , U > const &tree_data , bool checked = false )
	{
		Array< ONode const * , 100 > queue;
		queue.push_back( root );
		while( queue.size() != 0 )
		{
			ONode const *cur = queue[ ( queue.count-- ) - 1 ];
			if( checked || cur->intersectsRay( rp , rv ) )
			{
				if( cur->item_id >= 0 )
				//if( depth == 2 )
				{
					out.push_back( cur->self_id , cur->pos.g_dist2( rp ) );
				} else if( cur->children_id[ 0 ] > 0 )
				{
					for( int i = 0; i < 8; i++ )
					{
						if( tree_data.node_bank_array[ cur->children_id[ i ] ].intersectsRay( rp , rv ) )
						{
							//tree_data.node_bank_array[ children_id[ i ] ].fillListRay( rp , rv , out , tree_data , true );
							queue.push_back( &tree_data.node_bank_array[ cur->children_id[ i ] ] );
						}
					}
				}
			}
		}
	}
	template< int N , int M >
	void execRecursively( std::function< void( ONode const &o ) > func , TreeData< N , M > const &tree_data ) const
	{
		func( *this );
		if( children_id[ 0 ] != -1 )
		{
			ito( 8 )
				tree_data.node_bank_array[ children_id[ i ] ].execRecursively( func , tree_data );
		}
	}
};
#endif /* OTREELINEAR_H_ */
