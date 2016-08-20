#pragma once
#include <engine/os/Input.hpp>
#include <engine/graphics/Renderer.hpp>
namespace OS
{
	using namespace OS::InputState;
	struct WindowParam
	{
		int x , y , width , height;
		std::function< void() > init_func;
		std::function< void( WindowParam , float ) > update_func;
		std::function< void( int , int , int , int ) > resize_func;
		std::function< void() > release_func;
		std::function< void( EventType , State const * ) > input_handler;
	};
	class Window
	{
	private:
#ifdef _WIN32
		HWND hwnd;
		HDC hdc;
#endif
		WindowParam param;
	public:
		Window( WindowParam param )
		{
			this->param = param;
		}
		void setPosition( i2 const & );
		void setSize( i2 const & );
		//i4 getRect() const;
		i2 getSize() const;
		Renderer *create( Allocators::Allocator *allocator = Allocators::Allocator::singleton );
		~Window();
		void run();
	};
}
