#pragma once
#include <engine/data_struct/Array.hpp>
#include <engine/data_struct/HashSet.hpp>
#include <engine/mem/Allocators.hpp>
#include <engine/math/vec.hpp>
namespace Collection
{
	using namespace Math;
	namespace TKBiTree
	{
		template< int K , typename Real , typename T >
		struct TItem
		{
			TVector< K * 2 , Real > bound;
			T val;
		};
		template< int K , typename Real , typename T >
		class TNode
		{
			typedef TItem< K , Real , T > Item;
		private:
			Array< Item > items;
			TNode *children[ 2 ] = nullptr;
		};
	}
}