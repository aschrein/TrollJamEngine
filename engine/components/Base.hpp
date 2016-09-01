#pragma once
#include <engine/data_struct/String.hpp>
#include <engine/data_struct/Array.hpp>
#include <engine/data_struct/HashSet.hpp>
#include <engine/components/Reflection.hpp>
#include <engine/data_struct/Buffer.hpp>
#include <engine/components/Event.hpp>
#include <engine/graphics/Graphics.hpp>
#include <engine/physics/Physics.hpp>
class Object;
using namespace Collections;
using namespace Physics;
using namespace EventSystem;
using namespace Allocators;
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
class PysicalWorld;
class Scene
{
private:
	Array< Object* > living_objects;
public:
	void addEvent( Event * );
	void subscribe( Object *obj , uint event_id );
	void unsubscribe( Object *obj , uint event_id );
	Array< Object* > getCollided( AABB const & );
	Array< Object* > getCollided( Sphere const & );
	Array< Object* > getObjectsByClassName( String const & );
	Array< Object* > getObjectsByClassId( uint const & );
};
class GraphicsComponent
{
protected:
	Object *object = nullptr;
public:
	GraphicsComponent( Object *obj , Allocator *allocator = Allocator::singleton ) :
		object( obj )
	{
	}
	virtual void render( Object *object , Graphics::CommandBuffer &cmd_buffer ) = 0;
	virtual AABB getBound() const = 0;
	virtual ~GraphicsComponent() {};
};
class PhysicsComponent
{
protected:
	Array< PhysicsShape > shapes;
	Object *object = nullptr;
	AABB aabb;
	PysicsBodyProperties prop;
	PysicsBodyState state;
	NONMOVABLE( PhysicsComponent );
public:
	PhysicsComponent( Object *obj , Allocator *allocator = Allocator::singleton ) :
		object( obj )
	{
		shapes.setAllocator( allocator );
	}
	Array< PhysicsShape > const &getShapeInfo() const
	{
		return shapes;
	}
	Object *getObject()
	{
		return object;
	}
	void update( float dt )
	{
		state.pos += state.vel * dt;
	}
	void appendForce( float3 const &pos , float3 const &force , float dt )
	{
		state.vel += force * prop.invmass;
	}
	AABB getBound() const
	{
		return aabb;
	}
	PysicsBodyProperties const &getPhysicsProperties() const
	{
		return prop;
	}
	PysicsBodyState &getPhysicsState()
	{
		return state;
	}
};
class PysicalWorld
{
protected:
	//QuadTree< PhysicsComponent* > components_tree; // separate by AABB or OBB?
	//KDTree< PhysicsComponent* > components_tree;
	NONMOVABLE( PysicalWorld );
	Allocator *allocator;
public:
	Pointers::Unique< PysicalWorld > create( Allocator *allocator = Allocator::singleton );
	void update();
	uint add( PhysicsComponent* );
	void remove( uint );
	Allocator *getAllocator()
	{
		return allocator;
	}
};
class VisualWorld
{
protected:
	//QuadTree< PhysicsComponent* > components_tree; // separate by AABB or OBB?
	//KDTree< PhysicsComponent* > components_tree;
	NONMOVABLE( VisualWorld );
	Allocator *allocator;
public:
	Pointers::Unique< VisualWorld > create( Allocator *allocator = Allocator::singleton );
	void render();
	int add( GraphicsComponent* );
	void remove( uint );
	Allocator *getAllocator()
	{
		return allocator;
	}
};
class Object : public Acceptor
{
private:
	static uint64_t ID_COUNTER;
	uint64_t id;
	int phys_component = -1;
	int graphics_component = -1;
public:
	Object( int graphics_component , int phys_component ) :
		id( ID_COUNTER++ ) ,
		graphics_component( graphics_component ) ,
		phys_component( phys_component  )
	{}
	virtual RTTI const *getRTTI() = 0;
	uint64_t getId() const
	{
		return id;
	}
	uint hash() const
	{
		return getId();
	}
	int getGraphicsComponent()
	{
		return graphics_component;
	}
	int getPhysicsComponent()
	{
		return phys_component;
	}
};