#pragma once
#include <engine/data_struct/HashSet.hpp>
namespace Collections
{
	template< typename K , typename V >
	struct MapItem
	{
		K key;
		V value;
		bool operator==( MapItem const &a ) const
		{
			return key == a.key;
		}
		bool operator<( MapItem const &a ) const
		{
			return key < a.key;
		}
		int hash() const
		{
			return Hash< K >::hashFunc( key );
		}
	};
	template< typename K , typename V >
	class HashMap : public HashSet< MapItem< K , V > >
	{
	public:
		HashMap() = default;
		HashMap &push( K const &key , V const &val )
		{
			HashSet::push( MapItem< K , V >{ key , val } );
			return *this;
		}
		HashMap &push( K &&key , V const &&val )
		{
			HashSet::push( std::move( MapItem< K , V >{ std::move( key ) , std::move( val ) } ) );
			return *this;
		}
		bool contains( K const &key ) const
		{
			return HashSet::contains( { key } );
		}
		Options::Result< V > get( K const &key )
		{
			if( !this->chunks )
			{
				return Options::Result< V >();
			}
			int chunk_id = this->getChunkId( { key } );
			auto container = this->chunks[ chunk_id ].getContainer( { key } );
			if( container )
			{
				return Options::Result< V >( container->value.value );
			}
			return Options::Result< V >();
		}
		void remove( K const &key )
		{
			HashSet< MapItem< K , V > >::remove( MapItem< K , V >{ key } );
		}
		void print() const
		{
			for( auto const &item : *this )
			{
				OS::IO::debugLog( item.key , "->" , item.value , " " );
			}
			OS::IO::debugLogln();
		}
	};
}