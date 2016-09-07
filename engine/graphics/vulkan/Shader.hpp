#pragma once
#include <engine/graphics/vulkan/defines.hpp>
#include <engine/graphics/ShaderBuilder.hpp>
#include <engine/assets/FileManager.hpp>
#include <engine/os/Files.hpp>
#include <engine/os/Window.hpp>
namespace VK
{
	enum class PipelineStageType
	{
		VERTEX , FRAGMENT , GEOMETRY , TESSELATION_CONTROL , TESSELATION_EVAL
	};
	struct ShaderIn
	{
		uint location;
		uint binding;
	};
	inline String getTypeRepr( Graphics::Shaders::Type type )
	{
		static String repr[] =
		{
			"void" , "float" , "vec2" , "vec3" , "vec4" , "int" , "ivec2" , "ivec3" , "ivec4"
		};
		return repr[ ( uint )type ];
	}
	inline String getTokenRepr( Graphics::Shaders::Token const &token )
	{
		if( token.type == Graphics::Shaders::TokenType::SYMBOL || token.type == Graphics::Shaders::TokenType::CONST )
		{
			return token.name;
		}
		if( token.type == Graphics::Shaders::TokenType::SWIZZLE )
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
			return ret[ ( uint )token.swizzle ];
		}
		if( token.type == Graphics::Shaders::TokenType::BUILTIN_VARIABLE )
		{
			const char *repr[] =
			{
				"gl_Position"
			};
			return repr[ ( uint8_t )token.builtin_var ];
		}
		if( token.type == Graphics::Shaders::TokenType::BUILTIN_TYPE )
		{
			return VK::getTypeRepr( token.builtin_type );
		}
		if( token.type == Graphics::Shaders::TokenType::BUILTIN_FUNCTION )
		{
			const char *repr[] =
			{
				"texture" , "cross" , "dot" , "sin" , "cos" , "pow" , "normalize"
			};
			return repr[ ( uint )token.builtin_fn ];
		}
		if( token.type == Graphics::Shaders::TokenType::OPERATOR )
		{
			const char *repr[] =
			{
				"+" , "-" , "*" , "/" , "=" , "." , ","
			};
			return repr[ ( uint )token.op ];
		}
		if( token.type == Graphics::Shaders::TokenType::TUPLE_BEGIN )
		{
			return "(";
		}
		if( token.type == Graphics::Shaders::TokenType::TUPLE_END )
		{
			return ")";
		}
		if( token.type == Graphics::Shaders::TokenType::RETURN )
		{
			return "return ";
		}
		return "";
	}
	inline String getExprRepr( Graphics::Shaders::Expr const &expr )
	{
		String out;
		expr.token_array.foreach( [ &out ]( Graphics::Shaders::Token const tk )
		{
			out += VK::getTokenRepr( tk );
		} );
		return out;
	}
	inline String getFuncRepr( Graphics::Shaders::Function const &func )
	{
		String out;
		out += VK::getTypeRepr( func.ret_type ) + " " + func.name + "( ";
		func.argv.foreach( [ &out ]( Graphics::Shaders::ArgDef const &argdef )
		{
			out += VK::getTypeRepr( argdef.key ) + " " + argdef.value + " , ";
		} );
		if( !func.argv.isEmpty() )
		{
			out.pop();
			out.pop();
		}
		out += ")\n{\n";
		func.expressions.foreach( [ &out ]( Graphics::Shaders::Expr const &expr )
		{
			out += VK::getExprRepr( expr ) + ";\n";
		} );
		out += "}\n";
		return out;
	}
	class Stage
	{
		friend class Shader;
		VK_OBJECT( Stage );
	private:
		Unique< VkShaderModule > handler;
		VkShaderStageFlagBits stage_flag;
	public:
		VkShaderModule const &gethandler() const
		{
			return *handler;
		}
		VkShaderStageFlagBits getFlags() const
		{
			return stage_flag;
		}
	};
	class Shader
	{
		VK_OBJECT( Shader );
	private:
		LocalArray< Stage , 5 > stages;
		LocalArray< ShaderIn , 20 > constants;
		LocalArray< ShaderIn , 20 > attributes;
		LocalArray< uint , 10 > textures;
	public:
		static Shader create( Device const &device , Graphics::ShaderInfo const &info )
		{
			Shader out;
			for( auto const &stage_info : info.stages )
			{
				auto shader_text = genShader( stage_info.value );
				OS::IO::debugLogln( shader_text );
				String ext = ".vert";
				Stage stage;
				if( stage_info.key == Graphics::StageType::VERTEX )
				{
					ext = ".vert";
					stage.stage_flag = VK_SHADER_STAGE_VERTEX_BIT;
					for( auto const &attr : stage_info.value.in )
					{
						out.attributes.push( { attr.get< 0 >() , 0 } );
					}
					/*for( auto const &attr : stage_info.value.in )
					{
						out.attributes.push( { attr.get< 0 >() , 0 } );
					}*/
				} else if( stage_info.key == Graphics::StageType::FRAGMENT )
				{
					ext = ".frag";
					stage.stage_flag = VK_SHADER_STAGE_FRAGMENT_BIT;
				}
				OS::Files::save( info.name + ext , shader_text.getChars() , shader_text.getLength() );
				OS::exec( "glslangValidator -o " + info.name + ext + ".spv -V " + info.name + ext );
				auto res = OS::Files::load( info.name + ext + ".spv" );
				if( res.isPresent() )
				{
					auto image = res.getValue();
					stage.handler = device.createShaderModule( image.getView().getRaw() , image.getView().getLimit() );
					out.stages.push( std::move( stage ) );
				}
			}
			return out;
		}
		static String genShader( Graphics::Shaders::Stage const &stage_info )
		{
			String out;
			out +=
				"#version 450\n\
			#extension GL_ARB_separate_shader_objects : enable\n\
			#extension GL_ARB_shading_language_420pack : enable\n";
			stage_info.in.foreach( [ &out ]( auto const &expr )
			{
				out += "layout( location = " + String( expr.get< 0 >() ) + " ) in "
					+ VK::getTypeRepr( expr.get< 1 >() ) + " " + expr.get< 2 >() + ";\n";
			} );
			stage_info.out.foreach( [ &out ]( auto const &expr )
			{
				out += "layout( location = " + String( expr.get< 0 >() ) + " ) out "
					+ VK::getTypeRepr( expr.get< 1 >() ) + " " + expr.get< 2 >() + ";\n";
			} );
			stage_info.textures2d.foreach( [ &out ]( auto const &expr )
			{
				out += "layout( location = " + String( expr.key ) + " ) uniform sampler2D " + expr.value + ";\n";
			} );
			stage_info.constants.foreach( [ &out ]( auto const &expr )
			{
				out += "layout( location = " + Graphics::Shaders::getTypeRepr( expr.key ) + " ) uniform " + expr.value + ";\n";
			} );
			stage_info.functions.foreach( [ &out ]( auto const &expr )
			{
				out += VK::getFuncRepr( expr );
			} );
			out += VK::getFuncRepr( stage_info.body );
			return out;
		}
		auto const &getAttribures() const
		{
			return attributes;
		}
		auto const &getStages() const
		{
			return stages;
		}
		auto const &getConstants() const
		{
			return constants;
		}
		auto const &getTextures() const
		{
			return textures;
		}
	};
}