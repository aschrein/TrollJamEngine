#pragma once
#include <engine/os\log.hpp>
#include <functional>
namespace Collections
{
	template< template< typename > typename T , typename D >
	class Collection
	{
	public:
		T<D> const operator+( T<D> const &rval ) const
		{
			T<D> const *instance = static_cast< T<D> const* >( this );
			T<D> out;
			for( auto const &v : *instance )
			{
				out.push( v );
			}
			for( auto const &v : rval )
			{
				out.push( v );
			}
			return out;
		}
		template< typename M >
		T<D> const operator+( M const &rval ) const
		{
			T<D> const *instance = static_cast< T<D> const* >( this );
			T<D> out = *instance;
			return out += rval;
		}
		template< typename M >
		T<D> &operator+=( M const &rval )
		{
			T<D> *instance = static_cast< T<D>* >( this );
			instance->push( rval );
			return *this;
		}
		template< typename M >
		T<M> const map( std::function< M( D ) > map_func ) const
		{
			T<D> const *instance = static_cast< T<D> const* >( this );
			T<M> out;
			for( auto const &v : *instance )
			{
				out.push( map_func( v ) );
			}
			return out;
		}
		T<D> const filter( std::function< bool( D ) > filter_func ) const
		{
			T<D> const *instance = static_cast< T<D> const* >( this );
			T<D> out;
			for( auto const &v : *instance )
			{
				if( filter_func( v ) )
				{
					out.push( v );
				}
			}
			return out;
		}
		template< typename M >
		M translate() const
		{
			T<D> const *instance = static_cast< T<D> const* >( this );
			M out;
			for( auto const &v : *instance )
			{
				out.push( v );
			}
			return out;
		}
		void print() const
		{
			T<D> const *instance = static_cast< T<D> const* >( this );
			using namespace std;
			for( auto const &v : *instance )
			{
				OS::IO::debugLog( v , " " );
			}
			OS::IO::debugLogln();
		}
		bool allMatch( std::function< bool( D const & ) > matcher ) const
		{
			T<D> const *instance = static_cast< T<D> const* >( this );
			for( auto const &v : *instance )
			{
				if( !matcher( v ) )
				{
					return false;
				}
			}
			return true;
		}
	};
}
