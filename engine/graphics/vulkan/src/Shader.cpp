#include <stdafx.h>
#include <engine/graphics/Shader.hpp>
namespace Graphics
{
	Expr Expr::operator=( Expr const &expr ) const
	{
		return{ name + "=" + expr.name };
	}
	Expr Expr::operator+( Expr const &expr ) const
	{
		return{ name + "+" + expr.name };
	}
	Expr Expr::operator-( Expr const &expr ) const
	{
		return{ name + "-" + expr.name };
	}
	Expr Expr::operator+=( Expr const &expr ) const
	{
		return{ name + "+=" + expr.name };
	}
	Expr Expr::operator-=( Expr const &expr ) const
	{
		return{ name + "-=" + expr.name };
	}
	Expr Expr::operator*( Expr const &expr ) const
	{
		return{ name + "*" + expr.name };
	}
	Expr Expr::operator/( Expr const &expr ) const
	{
		return{ name + "/" + expr.name };
	}
	Expr Expr::operator*=( Expr const &expr ) const
	{
		return{ name + "*=" + expr.name };
	}
	Expr Expr::operator/=( Expr const &expr ) const
	{
		return{ name + "/=" + expr.name };
	}
	Expr Expr::operator()( String const &field ) const
	{
		return{ name + "." + field };
	}
	String Expr::getRepr( Type type )
	{
		static char const *ret[] =
		{
			"void" ,
			"float" ,
			"vec2" ,
			"vec3" ,
			"vec4"
		};
		return ret[ ( uint )type ];
	}
	Expr Expr::operator()( Swizzle swizzle ) const
	{
		static char const *ret[] =
		{
			".x" ,
			".y" ,
			".z" ,
			".xy" ,
			".yx" ,
			".xyz"
		};
		return{ name + ret[ ( uint )swizzle ] };
	}
	Expr Expr::sin( Expr const &val )
	{
		return{ "sin( " + val.name + " )" };
	}
	Expr Expr::cos( Expr const &val )
	{
		return{ "cos( " + val.name + " )" };
	}
	Expr Expr::pow( Expr const &val , Expr const &exp )
	{
		return{ "pow( " + val.name + " )" };
	}
	Expr Expr::ret( Expr const &expr )
	{
		return{ "return " + expr.name };
	}
	Expr Function::operator()( Array< String > const &argv ) const
	{
		String out = name + "( ";
		for( auto const &arg : argv )
		{
			out += arg + " , ";
		}
		out.pop();
		out.pop();
		out += ")";
		return{ out };
	}
	struct PipelineStageData
	{
		Allocator *allocator;
		String body;
		String cur_block;
		Array< uint > constants;
		Array< uint > textures;
		~PipelineStageData() = default;
	};
	Array< uint > PipelineStage::getConstantsLocation() const
	{
		auto nthis = ( PipelineStageData* )this;
		return nthis->constants;
	}
	Array< uint > PipelineStage::getTexturesLocation() const
	{
		auto nthis = ( PipelineStageData* )this;
		return nthis->textures;
	}
	PipelineStage *PipelineStage::create( Allocator *allocator )
	{
		auto out = allocator->alloc< PipelineStageData >();
		new( out ) PipelineStageData();
		out->body.setAllocator( allocator );
		out->allocator = allocator;
		out->body +=
			"#version 450\n\
			#extension GL_ARB_separate_shader_objects : enable\n\
			#extension GL_ARB_shading_language_420pack : enable\n";
		return ( PipelineStage * )out;
	}
	PipelineStage *PipelineStage::addOut( uint location , String const &type , String const &name )
	{
		auto nthis = ( PipelineStageData* )this;
		nthis->body += "layout( location = " + String( location ) + " ) out " + type + " " + name + ";\n";
		return this;
	}
	PipelineStage *PipelineStage::addIn( uint location , String const &type , String const &name )
	{
		auto nthis = ( PipelineStageData* )this;
		nthis->body += "layout( location = " + String( location ) + " ) in " + type + " " + name + ";\n";
		return this;
	}
	PipelineStage *PipelineStage::addAttribute( uint location , String const &type , String const &name )
	{
		auto nthis = ( PipelineStageData* )this;
		nthis->body += "layout( location = " + String( location ) + " , binding = 0) in " + type + " " + name + ";\n";
		return this;
	}
	PipelineStage *PipelineStage::addTexture( uint location , String const &name )
	{
		auto nthis = ( PipelineStageData* )this;
		nthis->body += "layout( location = " + String( location ) + " , binding = 0 ) uniform sampler2D " + name + ";\n";
		return this;
	}
	PipelineStage *PipelineStage::beginStruct( String const &name )
	{
		auto nthis = ( PipelineStageData* )this;
		nthis->body += "struct " + name + "\n{\n";
		return this;
	}
	PipelineStage *PipelineStage::endStruct()
	{
		auto nthis = ( PipelineStageData* )this;
		nthis->body += "}\n";
		return this;
	}
	PipelineStage *PipelineStage::beginConstantBlock( uint location , String const &name )
	{
		auto nthis = ( PipelineStageData* )this;
		nthis->body += "layout( location = " + String( location ) + " , binding = 0 ) uniform " + name + "TYPE\n{\n";
		nthis->cur_block = name;
		return this;
	}
	PipelineStage *PipelineStage::endConstantBlock()
	{
		auto nthis = ( PipelineStageData* )this;
		nthis->body += "} " + nthis->cur_block + ";\n";
		return this;
	}
	PipelineStage *PipelineStage::define( String const &type , String const &var )
	{
		auto nthis = ( PipelineStageData* )this;
		nthis->body += type + " " + var + ";\n";
		return this;
	}
	PipelineStage *PipelineStage::beginFunc( String const &return_type , String const &name , Array< Pair< String , String > >  const &argv )
	{
		auto nthis = ( PipelineStageData* )this;
		nthis->body += return_type + " " + name + "( ";
		for( auto const &arg : argv )
		{
			nthis->body += arg.key + " " + arg.value + " ,";
		}
		nthis->body.pop();
		nthis->body += ")\n{\n";
		return this;
	}
	PipelineStage *PipelineStage::beginBody()
	{
		auto nthis = ( PipelineStageData* )this;
		nthis->body += "void main()\n{\n";
		return this;
	}
	PipelineStage *PipelineStage::endFunc()
	{
		auto nthis = ( PipelineStageData* )this;
		nthis->body += "}\n";
		return this;
	}
	PipelineStage *PipelineStage::addExpr( Expr const &expr )
	{
		auto nthis = ( PipelineStageData* )this;
		nthis->body += expr.name + ";\n";
		return this;
	}
	String PipelineStage::getRepr() const
	{
		auto nthis = ( PipelineStageData* )this;
		return nthis->body;
	}
	PipelineStage::~PipelineStage()
	{
		auto nthis = ( PipelineStageData* )this;
		nthis->~PipelineStageData();
		//nthis->allocator->free( nthis );
	}
}
namespace Collections
{
	template<>
	void StringUtil::stringify< Graphics::Expr >( char *mem , Graphics::Expr const &val )
	{
		Allocator::copy( mem , val.name.getChars() , val.name.getLength() + 1 );
	}
}