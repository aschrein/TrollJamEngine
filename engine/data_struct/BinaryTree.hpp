#pragma once
#include <engine/os/log.hpp>
#include <engine/data_struct/String.hpp>
#include <engine/math/Math.hpp>
#include <engine/mem/Allocators.hpp>
#include <engine/data_struct/Optional.hpp>
#undef max
namespace Collections
{
	using namespace Allocators;
	using namespace Options;
	template< typename T >
	struct TBinaryTreeItem
	{
		TBinaryTreeItem *left = nullptr;
		TBinaryTreeItem *right = nullptr;
		TBinaryTreeItem *next = nullptr;
		T value;
		int height = 1;
		TBinaryTreeItem( T &&value ) :
			value( std::move( value ) )
		{}
		~TBinaryTreeItem()
		{}
		void recursiveDelete( Allocator *allocator )
		{
			if( left )
			{
				left->recursiveDelete( allocator );
				allocator->freeObj( left );
			}
			if( right )
			{
				right->recursiveDelete( allocator );
				allocator->freeObj( right );
			}
		}
		TBinaryTreeItem *getMostLeft()
		{
			if( !left )
			{
				return this;
			} else
			{
				return left->getMostLeft();
			}
		}
		TBinaryTreeItem *getMostRight()
		{
			if( !right )
			{
				return this;
			} else
			{
				return right->getMostRight();
			}
		}
		static int getHeight( TBinaryTreeItem const *item )
		{
			if( item )
			{
				return item->height;
			} else
			{
				return 0;
			}
		}
		TBinaryTreeItem *shiftRight()
		{
			auto lleft = left;
			auto left_right = left->right;
			left->right = this;
			this->left = left_right;
			updateHeight();
			lleft->updateHeight();
			return lleft;
		}
		TBinaryTreeItem *shiftLeft()
		{
			auto lright = right;
			auto right_lefy = right->left;
			right->left = this;
			this->right = right_lefy;
			updateHeight();
			lright->updateHeight();
			return lright;
		}
		void updateHeight()
		{
			height = Math::MathUtil< int >::max( getHeight( right ) , getHeight( left ) ) + 1;
		}
		template< typename T >
		struct TreeInsertWalk
		{
			TBinaryTreeItem< T > *last_item;
			TBinaryTreeItem< T > *new_item;
			bool last_turn;
			operator bool() const
			{
				return new_item != nullptr;
			}
		};
		TreeInsertWalk< T > pushUnique( T &&val , Allocator *allocator )
		{
			if( value == val )
			{
				value = std::move( val );
				return { nullptr };
			}
			bool turn;
			TreeInsertWalk< T > res = { nullptr , nullptr , false };
			if( val < value )
			{
				turn = false;
				if( left )
				{
					if( res = left->pushUnique( std::move( val ) , allocator ) )
					{
						left = res.last_item;
					}
				} else
				{
					left = allocator->alloc< TBinaryTreeItem >();
					new( left ) TBinaryTreeItem( std::move( val ) );
					left->next = this;
					updateHeight();
					return{ this , left , false };
				}
			} else
			{
				turn = true;
				if( right )
				{
					bool replace_next = val < next->value;
					if( res = right->pushUnique( std::move( val ) , allocator ) )
					{
						right = res.last_item;
						if( replace_next )
						{
							next = res.new_item;
						}
					}
				} else
				{
					right = allocator->alloc< TBinaryTreeItem >();
					new( right ) TBinaryTreeItem( std::move( val ) );
					auto tmp = next;
					right->next = next;
					next = right;
					updateHeight();
					return{ this , right , true };
				}
			}
			if( !res )
			{
				return{ nullptr , nullptr , false };
			}
			return { balance( res.last_turn ) , res.new_item , turn };
		}
		bool contains( T const &val ) const
		{
			if( value == val )
			{
				return true;
			} else if( val < value )
			{
				if( left )
				{
					return left->contains( val );
				} else
				{
					return false;
				}
			} else
			{
				if( right )
				{
					return right->contains( val );
				} else
				{
					return false;
				}
			}
			return false;
		}
		template< typename T >
		struct TreeRemoveWalk
		{
			TBinaryTreeItem< T > *new_item;
			TBinaryTreeItem< T > *item_removed;
		};
		TreeRemoveWalk< T > remove( T const &val )
		{
			if( value == val )
			{
				if( !right && left )
				{
					left->getMostRight()->next = next;
					return{ left , this };
				}
				if( right && !left )
				{
					return{ right , this };
				}
				if( !right && !left )
				{
					return{ nullptr , this };
				}
				if( right->height >= left->height )
				{
					if( right->left == nullptr )
					{
						left->getMostRight()->next = right;
						right->left = left;
						right->updateHeight();
						return{ right , this };
					}
					TBinaryTreeItem *most_left = right;
					while( most_left->left->left != nullptr )
					{
						most_left = most_left->left;
					}
					auto ll = most_left->left;
					most_left->left = most_left->left->right;
					ll->right = right;
					ll->left = left;
					left->getMostRight()->next = ll;
					ll->updateHeight();
					return{ ll , this };
				} else
				{
					if( left->right == nullptr )
					{
						left->right = right;
						left->next = next;
						left->updateHeight();
						return{ left , this };
					}
					TBinaryTreeItem *most_right = left;
					while( most_right->right->right != nullptr )
					{
						most_right = most_right->right;
					}
					auto rr = most_right->right;
					most_right->right = most_right->right->left;
					rr->right = right;
					rr->left = left;
					rr->next = next;
					rr->updateHeight();
					return{ rr , this };
				}
			}
			int turn = 0;
			TreeRemoveWalk< T > res;
			if( val < value )
			{
				turn = -1;
				if( left )
				{
					res = left->remove( val );
					left = res.new_item;
				} else
				{
					return{ this , nullptr };
				}
			} else
			{
				turn = 1;
				if( right )
				{
					res = right->remove( val );
					right = res.new_item;
					if( res.item_removed && next == res.item_removed )
					{
						next = res.item_removed->next;
					}
				} else
				{
					return{ this , nullptr };
				}
			}
			if( !res.item_removed )
			{
				return{ this , nullptr };
			}
			int balance = getHeight( right ) - getHeight( left );
			if( balance > 1 )
			{
				return{ shiftLeft() , res.item_removed };
			} else if( balance < -1 )
			{
				return{ shiftRight() , res.item_removed };
			}
			return { this , res.item_removed };
		}
		TBinaryTreeItem *balance( bool turn /*turn in wich element was added in child item*/ )
		{
			int balance = getHeight( right ) - getHeight( left );
			if( balance > 1 && turn )
			{
				return shiftLeft();
			}
			if( balance < -1 && !turn )
			{
				return shiftRight();
			}
			if( balance > 1 && !turn )
			{
				right = right->shiftRight();
				return shiftLeft();
			}
			if( balance < -1 && turn )
			{
				left = left->shiftLeft();
				return shiftRight();
			}
			updateHeight();
			return this;
		}
		TBinaryTreeItem *getContainer( T const &val )
		{
			if( value == val )
			{
				return this;
			} else if( val < value )
			{
				if( left )
				{
					return left->getContainer( val );
				} else
				{
					return nullptr;
				}
			} else
			{
				if( right )
				{
					return right->getContainer( val );
				} else
				{
					return nullptr;
				}
			}
			return nullptr;
		}
		struct NeighborWalk
		{
			TBinaryTreeItem *pre_turn_left = nullptr;
			TBinaryTreeItem *pre_turn_right = nullptr;
			TBinaryTreeItem *container = nullptr;
		};
		NeighborWalk getNeighbors( T const &val , NeighborWalk walk = NeighborWalk() )
		{
			if( value == val )
			{
				walk.container = this;
				return walk;
			} else if( val < value )
			{
				walk.pre_turn_left = this;
				if( left )
				{
					return left->getNeighbors( val , walk );
				} else
				{
					return walk;
				}
			} else
			{
				walk.pre_turn_right = this;
				if( right )
				{
					return right->getNeighbors( val , walk );
				} else
				{
					return walk;
				}
			}
		}
	};
	template< typename T >
	class BinaryTree
	{
		typedef TBinaryTreeItem< T > BinaryTreeItem;
	private:
		Allocator *allocator = Allocator::singleton;
		BinaryTreeItem *root = nullptr;
		int items_count = 0;
	public:
		BinaryTree( Allocator *allocator ):
			allocator( allocator )
		{

		}
		void setAllocator( Allocator *allocator )
		{
			release();
			this->allocator = allocator;
		}
		BinaryTree() = default;
		BinaryTree( BinaryTree const &list )
		{
			*this = list;
		}
		BinaryTree& operator=( BinaryTree const &list )
		{
			release();
			for( auto const &item : list )
			{
				pushUnique( item );
			}
		}
		BinaryTree( BinaryTree &&list )
		{
			*this = std::move( list );
		}
		BinaryTree& operator=( BinaryTree &&list )
		{
			release();
			root = list.root;
			items_count = list.items_count;
			list.root = nullptr;
			list.items_count = 0;
			return *this;
		}
		void release()
		{
			if( root )
			{
				root->recursiveDelete( allocator );
				allocator->freeObj( root );
			}
			root = nullptr;
			items_count = 0;
		}
		~BinaryTree()
		{
			release();
		}
		BinaryTree &pushUnique( T &&val )
		{
			if( !root )
			{
				root = allocator->alloc< BinaryTreeItem >();
				new( root ) BinaryTreeItem( std::move( val ) );
				items_count++;
			} else
			{
				auto tmp = root;
				if( root = root->pushUnique( std::move( val ) , allocator ).last_item )
				{
					items_count++;
				} else
				{
					root = tmp;
				}
			}
			return *this;
		}
		BinaryTree &pushUnique( T const &val )
		{
			auto tmp = val;
			return pushUnique( std::move( tmp ) );
		}
		bool contains( T const &val ) const
		{
			if( root )
			{
				return root->contains( val );
			}
			return false;
		}
		void remove( T const &val )
		{
			if( root == nullptr )
			{
				return;
			} else
			{
				auto res = root->remove( val );
				root = res.new_item;
				if( res.item_removed )
				{
					allocator->freeObj( res.item_removed );
					items_count--;
				}
			}
		}
		int fill( String *lines , BinaryTreeItem const *item , int offset = 0 , int counter = 0 ) const
		{
			if( item == nullptr )
			{
				return 0;
			}
			int width_left = fill( lines , item->left , offset , counter + 1 );
			String tmp = String( item->value );
			while( lines[ counter ].getLength() < offset + width_left )
			{
				lines[ counter ] << " ";
			}
			lines[ counter ] << tmp;
			int width_right = fill( lines , item->right , lines[ counter ].getLength() , counter + 1 );
			return width_right + width_left + tmp.getLength();
		}
		void print() const
		{
			if( root )
			{
				/*PrintNode< T >  *n = new PrintNode< T >( root );
				n->print();
				delete n;*/
				String *lines = new String[ root->height ];
				fill( lines , root );
				ito( root->height )
				{
					OS::IO::debugLogln( lines[ i ] );
				}
				delete[] lines;
				//root->print*();
			}
		}
		BinaryTreeItem *getContainer( T const &val )
		{
			if( root )
			{
				return root->getContainer( val );
			}
			return nullptr;
		}
		int getSize() const
		{
			return items_count;
		}
		struct Neighborhood
		{
			Result< T > value;
			Result< T > left;
			Result< T > right;
		};
		Neighborhood getNeighborhood( T value )
		{
			if( !root )
			{
				return Neighborhood();
			}
			auto nw = root->getNeighbors( value );
			Neighborhood out;
			if( nw.container )
			{
				out.value = Result< T >( nw.container->value );
			}
			if( nw.pre_turn_left )
			{
				out.right = Result< T >( nw.pre_turn_left->value );
			}
			if( nw.pre_turn_right )
			{
				out.left = Result< T >( nw.pre_turn_right->value );
			}
			return out;
		}
		BinaryTreeItem *getRoot()
		{
			return root;
		}
		bool isEmpty() const
		{
			return root == nullptr;
		}
		struct TreeIterator
		{
			TBinaryTreeItem *cur_item;
			T &operator*()
			{
				return cur_item->value;
			}
			T *operator->()
			{
				return &cur_item->value;
			}
			T const &operator*() const
			{
				return cur_item->value;
			}
			T const *operator->() const
			{
				return &cur_item->value;
			}
			void step()
			{
				if( cur_item != nullptr )
				{
					cur_item = cur_item->next;
				}
			}
			TreeIterator operator++( int )
			{
				step();
				return *this;
			}
			TreeIterator operator++()
			{
				TreeIterator out = *this;
				step();
				return out;
			}
			bool operator==( TreeIterator const &a ) const
			{
				return cur_item == a.cur_item;
			}
			bool operator!=( TreeIterator const &a ) const
			{
				return cur_item != a.cur_item;
			}
		};
		TreeIterator begin()
		{
			return TreeIterator{ root->getMostLeft() };
		}
		TreeIterator end()
		{
			return TreeIterator{ nullptr };
		}
		TreeIterator const begin() const
		{
			return TreeIterator{ root->getMostLeft() };
		}
		TreeIterator const end() const
		{
			return TreeIterator{ nullptr };
		}
	};
}