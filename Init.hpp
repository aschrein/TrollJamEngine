#pragma once
#include <engine/components/Base.hpp>
#include <engine/components/Event.hpp>
namespace Allocators
{
	Allocator *Allocator::singleton = new Allocator();
}
int Object::ID_COUNTER = 0;
int RTTI::ID_COUNTER = 0;
namespace EventSystem
{
	int EVENT_TYPE_COUNTER = 0;
	HashMap< String , int > ID_MAP;
	Array< String > NAME_MAP;
}