#pragma once
#include <engine/math/vec.hpp>
#include <engine/os/Input.hpp>
#include <engine/view/GUI.hpp>
#include <editor/widgets/Util.hpp>
#include <engine/os/Window.hpp>
namespace Editor
{
	namespace Widgets
	{
		using namespace GUI;
		using namespace OS;
		class WindowWidget : public Widget
		{
		private:
			Widget *focused = nullptr;
			Widget *selected = nullptr;
			OS::Window *wnd;
		public:
			void setWindow( OS::Window *wnd )
			{
				this->wnd = wnd;
			}
			WindowWidget()
			{
				rect = { { 100 , 100 } ,{ 100 , 50 } };
			}
			Widget *getCollided( i2 const &pos )
			{
				int z = 0;
				Widget *collided = nullptr;
				for( auto child : childs )
				{
					if( child->getRect().isIn( pos ) )
					{
						collided = child;
						z = child->getLayer();
					}
				}
				return collided;
			}
			void consumeInput( OS::InputState::Event &ievent ) override
			{
				switch( ievent.type )
				{
				case OS::InputState::EventType::PTR_DOWN:
				{
					auto collided = getCollided( ievent.state.ptr_pos.xy() );
					if( collided )
					{
						if( collided != focused )
						{
							if( focused )
							{
								focused->setFocused( false );
							}
							collided->setFocused( true );
							focused = collided;
						}
					} else
					{
						if( focused )
						{
							focused->setFocused( false );
							focused = nullptr;
						}
					}
				}
				break;
				}
				if( focused )
				{
					focused->consumeInput( ievent );
				}
			}
			/*void draw( Graphics::Graphics2D &g2d ) override
			{
			rect = g2d.getRect();
			for( auto child : childs )
			{
			child->draw( g2d );
			}
			}*/
			void setRect( Graphics::Rect2D const &rect ) override
			{
				Widget::setRect( rect );
				wnd->setSize( rect.size );
			}
		};
	}
}