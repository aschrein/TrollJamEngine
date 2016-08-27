#pragma once
#include <engine/os/Input.hpp>
#include <engine/graphics/Renderer.hpp>
namespace OS
{
	using namespace OS::InputState;
	using namespace Graphics;

	struct WindowParam
	{
		int x , y , width , height;
		std::function< void() > init_func;
		std::function< void( int , int , int , int ) > resize_func;
		std::function< void() > release_func;
		std::function< void( EventType , State const * ) > input_handler;
	};
	class Window
	{
	private:
		WindowParam param;
	public:
#ifdef _WIN32
		HWND hwnd;
		HINSTANCE hinstance;
		HDC hdc;
#endif
		Window( WindowParam param )
		{
			this->param = param;
		}
		void setPosition( i2 const & );
		void setSize( i2 const & );
		i2 getSize() const;
		Renderer *createRenderer( Allocators::Allocator *allocator = Allocators::Allocator::singleton );
		~Window();
		void run();
	};
}
