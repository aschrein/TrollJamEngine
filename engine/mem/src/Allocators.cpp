#include <engine/stdafx.h>
namespace Allocators
{
	Allocator *Allocator::singleton = new Allocator();
}