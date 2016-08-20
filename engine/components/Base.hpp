#pragma once
#include <data_struct/String.hpp>
#include <data_struct/Array.hpp>
#include <data_struct/HashSet.hpp>
#include <util/Reflection.hpp>
#include <data_struct/Buffer.hpp>
class Object;
using namespace Collections;
class RTTI
{
private:
	static int ID_COUNTER;
	int id;
	HashSet< RTTI const* > child_rtti;
	HashSet< Object const* > instances;
public:
	RTTI() :
		id( ID_COUNTER++ )
	{}
	virtual RTTI const *getParentRTTI() const = 0;
	virtual String getName() const = 0;
	virtual ArrayView< PlainField > const &getPlainStructure() const = 0;
	HashSet< RTTI const* > const &getChildRTTI() const
	{
		return child_rtti;
	}
	HashSet< Object const* > const &getAllInstances() const
	{
		return instances;
	}
	void addChildRTTI( RTTI const *rtti )
	{
		child_rtti.push( rtti );
	}
	void addInstance( Object const *obj )
	{
		instances.push( obj );
	}
	void removeInstance( Object const *obj )
	{
		instances.remove( obj );
	}
	bool instanceof( RTTI const *rtti ) const
	{
		if( getId() == rtti->getId() )
		{
			return true;
		} else if( getParentRTTI() != nullptr )
		{
			return getParentRTTI()->instanceof( rtti );
		} else
		{
			return false;
		}
	}
	int getId() const
	{
		return id;
	}
	int hash() const
	{
		return getId();
	}
};
class Object
{
private:
	static int ID_COUNTER;
	int id;
public:
	Object() :
		id( ID_COUNTER++ )
	{}
	virtual RTTI const *getRTTI() = 0;
	int getId() const
	{
		return id;
	}
	int hash() const
	{
		return getId();
	}
};