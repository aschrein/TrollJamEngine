#include <stdafx.h>
#include <engine/components/Event.hpp>
namespace EventSystem
{
	using namespace Collections;
	extern int EVENT_TYPE_COUNTER;
	extern HashMap< String , int > ID_MAP;
	extern Array< String > NAME_MAP;
	int Event::addEventType( String const &name )
	{
		int new_id = EVENT_TYPE_COUNTER++;
		ID_MAP.push( name , new_id );
		NAME_MAP.push( name );
		return new_id;
	}
	int Event::getID( String const &name )
	{
		auto result = ID_MAP.get( name );
		if( result.isPresent() )
		{
			return result.getValue();
		} else
		{
			return -1;
		}
	}
	String Event::getName( int id )
	{
		if( id >= 0 && id < NAME_MAP.getSize() )
		{
			return NAME_MAP[ id ];
		} else
		{
			return "unknown";
		}
	}
}