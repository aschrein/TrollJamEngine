#pragma once
#pragma once
#include <util\Event.hpp>
#include <os\log.hpp>
using namespace OS::IO;
using namespace EventSystem;
class MagazineEvent : public Event
{
public:
	static int id;
	int getID() override
	{
		return id;
	}
	String magazine;
	MagazineEvent( String magazine ) :
		magazine( magazine )
	{}
	
};
class PublishEvent : public Event
{
public:
	static int id;
	String author;
	String magazine;
	PublishEvent( String magazine , String author ) :
		magazine( magazine ) ,
		author( author )
	{}
	int getID() override
	{
		return id;
	}
};
REGISTER_EVENT( MagazineEvent )
REGISTER_EVENT( PublishEvent )
class MagazineShop : public Notifier , public Acceptor
{
public:
	String name;
	MagazineShop( String name ) :
		name( name )
	{}
	Result accept( Event *event , void *src ) override
	{
		if( event->getID() == PublishEvent::id )
		{
			PublishEvent *ievent = static_cast< PublishEvent* >( event );
			logln( "magazine shop " , name , " got his magazine: " , ievent->magazine , " from author: " , ievent->author );
			notify( &MagazineEvent( ievent->magazine ) , src );
		}
		return Result();
	}
};
class Author : public Notifier
{
public:
	String name;
	Author( String name ) :
		name( name )
	{}
	void publish( String magazine )
	{
		notify( &PublishEvent( magazine , name ) , this );
	}
};
class Customer : public Acceptor
{
public:
	String name;
	Customer( String name ) :
		name( name )
	{}
	Result accept( Event *event , void *src ) override
	{
		if( event->getID() == MagazineEvent::id )
		{
			MagazineEvent *ievent = static_cast< MagazineEvent* >( event );
			logln( "customer " , name , " got his magazine: " , ievent->magazine );
		}
		return Result();
	}
};
bool EventTest()
{
	Author king( "king" );
	MagazineShop shop( "wallmart" );
	Customer rick( "rick" ) , jemy( "jemy" ) , ivan( "ivan" );
	king.addAcceptor( &shop );
	shop.addAcceptor( &rick )
		.addAcceptor( &jemy )
		.addAcceptor( &ivan );
	//while( true )
	{
		king.publish( "black shooter" );
	}
	shop.removeAcceptor( &rick );
	king.publish( "caravan of wizdom" );
	return true;
}