#pragma once
#include <engine/math/vec.hpp>
namespace OS
{
	namespace InputState
	{
		using namespace Math;
		enum class EventType
		{
			PTR_MOVE , PTR_DOWN , PTR_UP , KEY_DOWN , KEY_UP
		};
		struct State
		{
			static const int MAX_KEY = 0x100;
			static const int MAX_PTR_KEY = 10;
			bool keyboard_state[ MAX_KEY ] = { false };
			bool ptr_key_state[ MAX_PTR_KEY ] = { false };
			i3 ptr_pos;
		};
		struct Event
		{
			State state;
			EventType type;
		};
	}
}