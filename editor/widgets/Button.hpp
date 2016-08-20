#pragma once
#include <engine/math/vec.hpp>
#include <engine/os/Input.hpp>
#include <engine/view/GUI.hpp>
#include <editor/widgets/Util.hpp>
namespace Editor
{
	namespace Widgets
	{
		using namespace GUI;
		class Button : public Widget
		{
		private:
			DragMaster drag_master;
		public:
			Button() :
				drag_master( [ & ]( i2 dr )
			{
				rect.pos += dr;
			}
				)
			{
				rect = { { 100 , 100 } ,{ 100 , 50 } };
			}
			void consumeInput( OS::InputState::Event &ievent ) override
			{
				drag_master.handle( ievent );
			}
			/*void draw( Graphics2D &g2d ) override
			{
			i2 text_position = rect.pos;
			i2 offset = rect.size / 2;
			text_position += offset;
			byte r = isfocused ? 0x80 : 0x30;
			g2d.setColor( { r , 0x30 , 0x30 , 0x80 } ).drawRect( rect.pos , rect.pos + rect.size );
			}*/
		};
	}
}