#pragma once
#include <engine/components/Event.hpp>
#include <engine/graphics/Graphics.hpp>
#include <engine/math/vec.hpp>
#include <engine/os/Input.hpp>
namespace GUI
{
	using namespace Collections;
	using namespace Math;
	class Widget : public EventSystem::Notifier
	{
	public:
		class WidgetEvent : public EventSystem::Event
		{
		public:
			STD_EVENT_BODY;
			enum class EventType
			{
				RESIZE , HOVER , UNHOVER , FOCUS , UNFOCUS
			};
			EventType type;
			Widget *widget;
			WidgetEvent( Widget *widget , EventType type ) :
				widget( widget ) ,
				type( type )
			{}
		};
	protected:
		Widget* parent;
		Array< Widget* > childs;
		Graphics::Rect2D rect;
		int z = 0;
		bool isfocused = false;
	public:
		virtual void onCreate()
		{
			for( auto child : childs )
			{
				child->onCreate();
			}
		}
		virtual void onDestroy()
		{
			for( auto child : childs )
			{
				child->onDestroy();
			}
		}
		virtual void setFocused( bool f )
		{
			isfocused = f;
		}
		virtual void consumeInput( OS::InputState::Event & ) = 0;
		virtual void update( Graphics::CommandBuffer & , float dt ) = 0;
		Graphics::Rect2D getRect() const
		{
			return rect;
		}
		int getLayer() const
		{
			return z;
		}
		Widget *getParent()
		{
			return parent;
		}
		virtual Widget &addChild( Widget *child )
		{
			childs.push( child );
			child->parent = this;
			return *this;
		}
		virtual Widget &removeChild( Widget *child )
		{
			childs.removeFirst( child );
			child->parent = nullptr;
			return *this;
		}
		virtual void setRect( Graphics::Rect2D const &rect )
		{
			this->rect = rect;
		}
	};
}
