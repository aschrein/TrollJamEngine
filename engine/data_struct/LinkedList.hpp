#pragma once
namespace Collections
{
	template< typename T >
	struct TLinkedItem
	{
		TLinkedItem *next = nullptr;
		T value;
		TLinkedItem( T &&value ) :
			value( std::move( value ) )
		{
		}
	};
	template< typename T >
	class LinkedList
	{
		typedef TLinkedItem< T > LinkedItem;
	private:
		LinkedItem *root = nullptr;
		int items_count = 0;
	public:
		LinkedList() = default;
		LinkedList( LinkedList const &list )
		{
			*this = list;
		}
		LinkedList& operator=( LinkedList const &list )
		{
			release();
			for( auto const &item : list )
			{
				pushUnique( list );
			}
		}
		LinkedList( LinkedList &&list )
		{
			*this = std::move( list );
		}
		LinkedList& operator=( LinkedList &&list )
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
			LinkedItem *cur_item = root;
			while( cur_item != nullptr )
			{
				auto tmp = cur_item->next;
				delete cur_item;
				cur_item = tmp;
			}
			root = nullptr;
			items_count = 0;
		}
		~LinkedList()
		{
			release();
		}
		LinkedList &pushUnique( T &&val )
		{
			if( root == nullptr )
			{
				root = new LinkedItem( std::move( val ) );
				items_count++;
			} else
			{
				LinkedItem *cur_item = root;
				while( true )
				{
					if( cur_item->value == val )
					{
						break;
					} else if( cur_item->next != nullptr )
					{
						cur_item = cur_item->next;
					} else
					{
						cur_item->next = new LinkedItem( std::move( val ) );
						items_count++;
						break;
					}
				}
			}
			return *this;
		}
		LinkedList &pushUnique( T const &val )
		{
			auto tmp = val;
			return pushUnique( std::move( tmp ) );
		}
		bool contains( T const &val ) const
		{
			LinkedItem *cur_item = root;
			while( cur_item != nullptr )
			{
				if( cur_item->value == val )
				{
					return true;
				} else
				{
					cur_item = cur_item->next;
				}
			}
			return false;
		}
		void remove( T const &val )
		{
			if( root == nullptr )
			{
				return;
			} else if( root->value == val )
			{
				auto tmp = root->next;
				root->next = nullptr;
				delete root;
				root = tmp;
				items_count--;
			} else
			{
				LinkedItem *cur_item = root;
				while( cur_item->next != nullptr )
				{
					if( cur_item->next->value == val )
					{
						auto tmp = cur_item->next->next;
						cur_item->next->next = nullptr;
						delete cur_item->next;
						cur_item->next = tmp;
						return;
					} else
					{
						cur_item = cur_item->next;
					}
				}
			}
		}
		LinkedItem *getContainer( T const &val )
		{
			LinkedItem *cur_item = root;
			while( cur_item != nullptr )
			{
				if( cur_item->value == val )
				{
					return cur_item;
				} else
				{
					cur_item = cur_item->next;
				}
			}
			return nullptr;
		}
		int getSize() const
		{
			return items_count;
		}
		LinkedItem *getRoot()
		{
			return root;
		}
		bool isEmpty() const
		{
			return root == nullptr;
		}
		struct LinkedListIterator
		{
			LinkedItem *cur_item;
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
			LinkedListIterator operator++( int )
			{
				step();
				return *this;
			}
			LinkedListIterator operator++()
			{
				LinkedListIterator out = *this;
				step();
				return out;
			}
			bool operator==( LinkedListIterator const &a ) const
			{
				return cur_item == a.cur_item;
			}
			bool operator!=( LinkedListIterator const &a ) const
			{
				return cur_item != a.cur_item;
			}
		};
		LinkedListIterator begin()
		{
			return LinkedListIterator{ root };
		}
		LinkedListIterator end()
		{
			return LinkedListIterator{ nullptr };
		}
		LinkedListIterator const begin() const
		{
			return LinkedListIterator{ root };
		}
		LinkedListIterator const end() const
		{
			return LinkedListIterator{ nullptr };
		}
		void print() const
		{
			for( auto const &item : *this )
			{
				OS::IO::log( item , "->" );
			}
			OS::IO::logln( "null" );
		}
	};
}