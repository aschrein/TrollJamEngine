#pragma once
#include <engine/data_struct/String.hpp>
#include <engine/data_struct/Array.hpp>
#include <engine/data_struct/HashMap.hpp>
#undef VOID
namespace Graphics
{
	using namespace Collections;
	enum class Type
	{
		VOID , FLOAT , FLOAT2 , FLOAT3 , FLOAT4
	};
	enum class Swizzle
	{
		x , y , z , xy , yx , xyz
	};
	struct Expr
	{
		String name;
		Expr operator=( Expr const & ) const;
		Expr operator+( Expr const & ) const;
		Expr operator-( Expr const & ) const;
		Expr operator+=( Expr const & ) const;
		Expr operator-=( Expr const & ) const;
		Expr operator*( Expr const & ) const;
		Expr operator/( Expr const & ) const;
		Expr operator*=( Expr const & ) const;
		Expr operator/=( Expr const & ) const;
		Expr operator()( String const &field ) const;
		Expr operator()( Swizzle swizzle ) const;
		static Expr ret( Expr const &Expr );
		static String getRepr( Type type );
		static Expr sin( Expr const &val );
		static Expr cos( Expr const &val );
		static Expr pow( Expr const &val , Expr const &exp );
		static Expr createFloat2( Expr const &x , Expr const &y )
		{
			return{ getRepr( Type::FLOAT2 ) + "( " + x + " , " + y + " )" };
		}
		static Expr createFloat3( Expr const &x , Expr const &y , Expr const &z )
		{
			return{ getRepr( Type::FLOAT3 ) + "( " + x + " , " + y + " , " + z + " )" };
		}
		static Expr createFloat4( Expr const &x , Expr const &y , Expr const &z , Expr const &w )
		{
			return{ getRepr( Type::FLOAT4 ) + "( " + x + " , " + y + " , " + z + " , " + w + " )" };
		}
		static Expr createFloat4( Expr const &xy , Expr const &z , Expr const &w )
		{
			return{ getRepr( Type::FLOAT4 ) + "( " + xy + " , " + z + " , " + w + " )" };
		}
		static Expr createFloat4( Expr const &xyz , Expr const &w )
		{
			return{ getRepr( Type::FLOAT4 ) + "( " + xyz + " , " + w + " )" };
		}
		operator String() const
		{
			return name;
		}
	};
	struct Function
	{
		String name;
		operator String() const
		{
			return name;
		}
		Expr operator()( Array< String > const & ) const;
	};
	struct VaryingSlot
	{
		String name;
		Type type;
		uint location;
	};
	enum class PipelineStageType
	{
		VERTEX , FRAGMENT , GEOMETRY , TESSELATION_CONTROL , TESSELATION_EVAL
	};
	typedef Pair< Type , String > Arg;
	class PipelineStage
	{
	private:
		NONMOVABLE( PipelineStage );
	public:
		static PipelineStage *create( Allocator *allocator = Allocator::singleton );
		PipelineStage *addOut( uint location , String const &type , String const &name );
		PipelineStage *addOut( uint location , Type type , String const &name )
		{
			return addOut( location , Expr::getRepr( type ) , name );
		}
		PipelineStage *addIn( uint location , String const &type , String const &name );
		PipelineStage *addIn( uint location , Type type , String const &name )
		{
			return addIn( location , Expr::getRepr( type ) , name );
		}
		PipelineStage *addAttribute( uint location , String const &type , String const &name );
		PipelineStage *addAttribute( uint location , Type type , String const &name )
		{
			return addAttribute( location , Expr::getRepr( type ) , name );
		}
		PipelineStage *addTexture( uint location , String const &name );
		PipelineStage *beginStruct( String const &name );
		PipelineStage *endStruct();
		PipelineStage *beginConstantBlock( uint location , String const &name );
		PipelineStage *endConstantBlock();
		PipelineStage *define( String const &type , String const &var );
		PipelineStage *define( Type type , String const &var )
		{
			return define( Expr::getRepr( type ) , var );
		}
		PipelineStage *beginFunc( String const &return_type , String const &name , Array< Pair< String ,String > > const &argv );
		PipelineStage *beginFunc( Type type , String const &name , Array< Pair< Type , String > > const &argv )
		{
			Array< Pair< String , String > > argv1;
			for( auto const &arg : argv )
			{
				argv1.push( { Expr::getRepr( arg.key ) , arg.value } );
			}
			return beginFunc( Expr::getRepr( type ) , name , argv1 );
		}
		PipelineStage *endFunc();
		PipelineStage *beginBody();
		PipelineStage *addExpr( Expr const &expr );
		Array< uint > getConstantsLocation() const;
		Array< uint > getTexturesLocation() const;
		String getRepr() const;
		~PipelineStage();
	};
	class Shader
	{

	};
}