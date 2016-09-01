#pragma once
#include <engine/assets/Mesh.hpp>
#include <engine/assets/FileManager.hpp>
namespace Assets
{
	enum class AssetEventType
	{
		UPDATE
	};
	struct AssetEvent
	{
		AssetEventType type;
		void const *product;
		String name;
	};
	class AssetNode : public LockFree::Consumer< AssetEvent , 10 >
	{
	private:
		typedef std::function< void *( HashMap< String , void const* > const & , Allocator * ) > Pipe;
		typedef std::function< void( void * , Allocator * ) > Releaser;
		Allocator *allocator = nullptr;
		Pipe pipe;
		Releaser releaser;
		void *product = nullptr;
		Array< AssetNode* > consumers;
		HashMap< String , void const* > dependencies;
		String product_name;
		uint dependency_count;
	public:
		NONMOVABLE( AssetNode );
		AssetNode( uint dependency_count , Pipe pipe , Releaser releaser , String product_name , Allocator *allocator = Allocator::singleton ):
			pipe( pipe ) ,
			releaser( releaser  ) ,
			product_name( product_name ) ,
			allocator( allocator ) ,
			dependency_count( dependency_count )
		{
			consumers.setAllocator( allocator );
			dependencies.setAllocator( allocator );
		}
		~AssetNode()
		{
			if( product )
			{
				releaser( product , allocator );
				product = nullptr;
			}
		}
		void update()
		{
			bool update = false;
			while( !isEmpty() )
			{
				auto event = popEvent();
				if( dependencies.contains( event.name ) )
				{
					if( dependency_count == 0 )
					{
						update = true;
					}
				} else
				{
					dependencies.push( event.name , event.product );
					dependency_count--;
				}
				if( dependency_count == 0 && !product )
				{
					update = true;
				}
			}
			if( update )
			{
				if( product )
				{
					releaser( product , allocator );
				}
				product = pipe( dependencies , allocator );
			}
			if( update )
			{
				AssetEvent event = { AssetEventType::UPDATE , product , product_name };
				for( auto &consumer : consumers )
				{
					consumer->pushEvent( event );
				}
			}
		}
		void const *getProduct()
		{
			return product;
		}
		bool isPresent() const
		{
			return product;
		}
	};
}