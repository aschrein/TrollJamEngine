#pragma once
#include <engine/assets/Mesh.hpp>
#include <engine/assets/FileManager.hpp>
namespace Assets
{
	class Factory
	{

	};
	template< typename T >
	class PipeLine : public FileConsumer
	{
	private:
		Array< String > filenames;
		Allocator *allocator = nullptr;
		FileManager *file_manager = nullptr;
		std::function< T( T , FileImage const* ) > factory = nullptr;
		T final_data;
		bool present = false;
		NONMOVABLE( PipeLine );
		PipeLine *parent = nullptr;
	public:
		PipeLine( Array< String > filenames , std::function< T( FileImage * ) > factory , PipeLine *parent = nullptr , FileManager *file_manager = FileManager::singleton , Allocator *allocator = Allocator::singleton ):
			factory( factory ) ,
			allocator( allocator ) ,
			file_manager( file_manager ) ,
			parent( parent )
		{
			this->filenames.setAllocator( allocator );
			this->filenames = filenames;
		}
		void update()
		{
			auto file_res = popEvent();
			if( file_res.isPresent() )
			{
				do
				{
					auto file_event = file_res.getValue();
					if( file_event.type == FileEvent::EventType::LOADED && file_event.file_result.isPresent() )
					{
						FileImage file_image = std::move( file_event.file_result.getValue() );
						final_data = factory( final_data , &file_image );
						present = true;
					} else if( file_event.type == FileEvent::EventType::UPDATED )
					{
						file_manager->loadFile( filename , this , allocator );
					}
				} while( ( file_res = popEvent() ).isPresent() );
			}
		}
		T getValue()
		{
			return final_data;
		}
		bool isPresent() const
		{
			return present;
		}
	};
}