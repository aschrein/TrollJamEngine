#pragma once
#include <iostream>
namespace Collections
{
	template< typename ...T >
	struct Tuple;
	template< int i , typename K , typename ...T >
	struct Getter
	{
		typedef typename Getter< i - 1 , T... >::Type Type;
		static Type get( Tuple< K , T... > const *tuple )
		{
			return Getter< i - 1 , T... >::get( &tuple->rest );
		}
	};
	template< typename K , typename ...T >
	struct Getter< 0 , K , T... >
	{
		typedef K Type;
		static Type get( Tuple< K , T... > const *tuple )
		{
			return tuple->data;
		}
	};
	template< typename K , typename ...T >
	struct TypeCut
	{
		typedef K FirstType;
		typedef Tuple< T... > RestType;
	};
	template< typename ...T >
	struct Tuple
	{
		typedef typename TypeCut< T... >::FirstType Type;
		typename TypeCut< T... >::FirstType data;
		typename TypeCut< T... >::RestType rest;
		Tuple() = default;
		template< typename M , typename... Y >
		Tuple( M first , Y... rest ) :
			data( first ) ,
			rest( rest... )
		{}
		template< int i >
		typename Getter< i , T... >::Type get() const
		{
			return Getter< i , T... >::get( this );
		}
		void print() const
		{
			std::cout << data << " ";
			rest.print();
		}
	};
	template< typename K >
	struct Tuple< K >
	{
		typedef K Type;
		K data;
		template< typename M >
		Tuple( M first ) :
			data( first )
		{}
		template< int i >
		K get() const
		{
			return data;
		}
		void print() const
		{
			std::cout << data << "\n";
		}
	};
	template< typename K , typename V >
	struct Pair
	{
		K key;
		V value;
	};
}
