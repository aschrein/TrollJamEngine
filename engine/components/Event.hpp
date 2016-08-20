#pragma once
#include <engine/data_struct\String.hpp>
#include <engine/data_struct\HashMap.hpp>
#include <engine/util\defines.hpp>
#include <engine/data_struct\Array.hpp>
#define REGISTER_EVENT( name )\
int name::id = Event::addEventType( #name );
#define STD_EVENT_BODY \
static int id;\
int getID() override\
{\
	return id;\
}
namespace EventSystem
{
	using namespace Collections;
	class Event
	{
	public:
		static int getID( String const & );
		static String getName( int );
		static int addEventType( String const & );
		virtual int getID() = 0;
		virtual ~Event(){}
	};
	/*class Result
	{
	private:
		Array< Event * > events;
	public:
		Result() = default;
		Result( Result const & ) = default;
		Result& operator=( Result const & ) = default;
		Result( Result && ) = default;
		Result& operator=( Result && ) = default;
		Result &append( Result const &res )
		{
			events += res.events;
			return *this;
		}
		Array< Event * > const &getEvents() const
		{
			return events;
		}
		Result &addEvent( Event *event )
		{
			events.push( event );
			return *this;
		}
		bool isEmpty() const
		{
			return events.getSize() == 0;
		}
		~Result()
		{
			for( auto event : events )
			{
				delete event;
			}
		}
	};*/
	class Acceptor
	{
	public:
		virtual void accept( Event* ) = 0;
	};
	class Notifier
	{
	private:
		Array< Acceptor * > acceptors;
	public:
		Notifier &addAcceptor( Acceptor *acc )
		{
			acceptors.push( acc );
			return *this;
		}
		Notifier &removeAcceptor( Acceptor *acc )
		{
			acceptors.removeFirst( acc );
			return *this;
		}
		void notify( Event *event )
		{
			for( auto acc : acceptors )
			{
				acc->accept( event );
			}
		}
	};
}
