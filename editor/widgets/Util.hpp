#pragma once
#include <math/vec.hpp>
#include <os/Input.hpp>
namespace Editor
{
	namespace Util
	{
		using namespace Math;
		using namespace OS::InputState;
		struct DragMaster
		{
			bool drag = false;
			i2 last_pos;
			std::function< void( i2 const & ) > handler;
			DragMaster( std::function< void( i2 const & ) > handler ) :
				handler( handler )
			{}
			void handle( OS::InputState::Event &ievent )
			{
				switch( ievent.type )
				{
				case OS::InputState::EventType::PTR_DOWN:
				{
					drag = true;
					last_pos = ievent.state.ptr_pos.xy();
				}
				break;
				case OS::InputState::EventType::PTR_MOVE:
				{
					if( drag )
					{
						i2 cur_pos = ievent.state.ptr_pos.xy();
						i2 dr = cur_pos - last_pos;
						handler( dr );
						last_pos = cur_pos;
					}
				}
				break;
				case OS::InputState::EventType::PTR_UP:
				{
					drag = false;
				}
				break;
				}
			}
		};
	}
}