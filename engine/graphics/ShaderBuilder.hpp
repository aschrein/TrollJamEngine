#pragma once
#include <engine/data_struct/String.hpp>
#include <engine/data_struct/Array.hpp>
#include <engine/data_struct/Tuple.hpp>
#include <engine/data_struct/HashMap.hpp>
#undef VOID
#undef CONST
namespace Graphics
{
	using namespace Collections;
	namespace Shaders
	{
		enum class Type
		{
			VOID , FLOAT , FLOAT2 , FLOAT3 , FLOAT4 , INT , INT2 , INT3 , INT4
		};
		inline String getTypeRepr( Type type )
		{
			static String repr[] =
			{
				"void" , "float" , "float2" , "float3" , "float4" , "int" , "int2" , "int3" , "inr4"
			};
			return repr[ ( uint )type ];
		}
		enum class Swizzle
		{
			x , y , z , xy , yx , xyz
		};
		enum class OperatorType
		{
			PLUS , MINUS , MUL , DIV , EQU , DOT , COMMA
		};
		enum class BuiltInFunction
		{
			TEX_FETCH2D , VECX , VDOT , SIN , COS , POW , NORM
		};
		enum class BuiltInVariable
		{
			VERTEX_POS
		};
		enum class TokenType : uint
		{
			SWIZZLE , SYMBOL , CONST , BUILTIN_FUNCTION , OPERATOR , TUPLE_BEGIN , TUPLE_END , RETURN , BUILTIN_TYPE , BUILTIN_VARIABLE
		};
		struct Token
		{
			union
			{
				char name[ 12 ];
				Swizzle swizzle;
				BuiltInVariable builtin_var;
				OperatorType op;
				Type builtin_type;
				BuiltInFunction builtin_fn;
			};
			TokenType type;
			Token()
			{
				type = TokenType::CONST;
				Allocator::zero( &name );
			}
			Token( Token const &tk )
			{
				*this = tk;
			}
			Token &operator=( Token const &tk )
			{
				Allocator::copy( this , &tk );
				return *this;
			}
			Token( TokenType tt ) :
				type( tt )
			{}
			Token( BuiltInFunction tt ) :
				builtin_fn( tt ) ,
				type( TokenType::BUILTIN_FUNCTION )
			{}
			Token( BuiltInVariable tt ) :
				builtin_var( tt ) ,
				type( TokenType::BUILTIN_VARIABLE )
			{}
			Token( Type tt ) :
				builtin_type( tt ) ,
				type( TokenType::BUILTIN_TYPE )
			{}
			Token( OperatorType tt ) :
				op( tt ) ,
				type( TokenType::OPERATOR )
			{}
			Token( char const *ch )
			{
				Allocator::copy( name , ch , 12 );
				type = TokenType::SYMBOL;
			}
			Token( String const &f )
			{
				Allocator::copy( name , f.getChars() , f.getLength() + 1 );
				type = TokenType::CONST;
			}
			Token( Swizzle s )
			{
				swizzle = s;
				type = TokenType::SWIZZLE;
			}
			Token( float t )
			{
				StringUtil::stringify( name , t );
				type = TokenType::CONST;
			}
			Token( double t )
			{
				StringUtil::stringify( name , t );
				type = TokenType::CONST;
			}
			Token( uint t )
			{
				StringUtil::stringify( name , t );
				type = TokenType::CONST;
			}
			Token( int t )
			{
				StringUtil::stringify( name , t );
				type = TokenType::CONST;
			}
			String getRepr() const
			{
				if( type == TokenType::SYMBOL || type == TokenType::CONST )
				{
					return name;
				}
				if( type == TokenType::SWIZZLE )
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
					return ret[ ( uint )swizzle ];
				}
				if( type == TokenType::BUILTIN_FUNCTION )
				{
					const char *repr[] =
					{
						"tex2Dfetch" , "vecx" , "dot" , "sin" , "cos" , "pow" , "norm"
					};
					return repr[ ( uint8_t )builtin_fn ];
				}
				if( type == TokenType::OPERATOR )
				{
					const char *repr[] =
					{
						"+" , "-" , "*" , "/" , "=" , "." , ","
					};
					return repr[ ( uint8_t )op ];
				}
				if( type == TokenType::BUILTIN_VARIABLE )
				{
					const char *repr[] =
					{
						"builtin_vertex_pos"
					};
					return repr[ ( uint8_t )builtin_var ];
				}
				if( type == TokenType::BUILTIN_TYPE )
				{
					return getTypeRepr( builtin_type );
				}
				if( type == TokenType::TUPLE_BEGIN )
				{
					return "(";
				}
				if( type == TokenType::TUPLE_END )
				{
					return ")";
				}
				if( type == TokenType::RETURN )
				{
					return "return ";
				}
			}
			~Token()
			{
			}
		};
		struct Expr
		{
			Array< Token > token_array;
			Expr() = default;
			Expr( Token const &tk )
			{
				token_array.push( tk );
			}
			Expr( Expr const &tk )
			{
				this->token_array = tk.token_array;
			}
			Expr( Expr &&tk )
			{
				this->token_array = std::move( tk.token_array );
			}
			Expr &operator=( Expr &&tk )
			{
				this->token_array = std::move( tk.token_array );
				return *this;
			}
			Expr &operator<<( Token const &t )
			{
				token_array.push( t );
				return *this;
			}
			Expr &operator<<( Expr const &t )
			{
				add( t );
				return *this;
			}
			Expr &add( Expr const &expr )
			{
				for( auto const &e : expr.token_array )
				{
					token_array.push( e );
				}
				return *this;
			}
			Expr &operator=( Expr const &expr )
			{
				this->token_array = expr.token_array;
				return *this;
			}
			Expr operator+( Expr const &expr ) const
			{
				Expr out = *this;
				out.token_array.push( { OperatorType::PLUS } );
				return out.add( expr );
			}
			Expr operator-( Expr const &expr ) const
			{
				Expr out = *this;
				out.token_array.push( { OperatorType::MINUS } );
				return out.add( expr );
			}
			Expr operator+=( Expr const &expr ) const
			{
				Expr out = *this;
				out.token_array.push( { OperatorType::PLUS } );
				out.token_array.push( { OperatorType::EQU } );
				return out.add( expr );
			}
			Expr operator-=( Expr const &expr ) const
			{
				Expr out = *this;
				out.token_array.push( { OperatorType::MINUS } );
				out.token_array.push( { OperatorType::EQU } );
				return out.add( expr );
			}
			Expr operator*( Expr const &expr ) const
			{
				Expr out = *this;
				out.token_array.push( { OperatorType::MUL } );
				return out.add( expr );
			}
			Expr operator/( Expr const &expr ) const
			{
				Expr out = *this;
				out.token_array.push( { OperatorType::DIV } );
				return out.add( expr );
			}
			Expr operator*=( Expr const &expr ) const
			{
				Expr out = *this;
				out.token_array.push( { OperatorType::MUL } );
				out.token_array.push( { OperatorType::EQU } );
				return out.add( expr );
			}
			Expr operator/=( Expr const &expr ) const
			{
				Expr out = *this;
				out.token_array.push( { OperatorType::DIV } );
				out.token_array.push( { OperatorType::EQU } );
				return out.add( expr );
			}
			Expr operator&( String const &field ) const
			{
				Expr out = *this;
				out.token_array.push( { OperatorType::DOT } );
				out.token_array.push( field );
				return out;
			}
			Expr operator&( Swizzle swizzle ) const
			{
				Expr out = *this;
				out << Token( swizzle );
				return out;
			}
			String getRepr() const
			{
				String out;
				token_array.foreach( [ &out ]( auto const &tk )
				{
					out += tk.getRepr();
				} );
				return out;
			}
			static Expr ret( Expr const &val )
			{
				Expr out;
				out << Token( TokenType::RETURN );
				out << val;
				return out;
			}
			static Expr sin( Expr const &val )
			{
				Expr out;
				out << Token{ BuiltInFunction::SIN };
				out << Token{ TokenType::TUPLE_BEGIN };
				out << val;
				out << Token{ TokenType::TUPLE_END };
				return out;
			}
			static Expr cos( Expr const &val )
			{
				Expr out;
				out << Token{ BuiltInFunction::COS };
				out << Token{ TokenType::TUPLE_BEGIN };
				out << val;
				out << Token{ TokenType::TUPLE_END };
				return out;
			}
			static Expr fetch2D( Token const &name , Expr const &val )
			{
				Expr out;
				out << Token{ BuiltInFunction::TEX_FETCH2D };
				out << Token{ TokenType::TUPLE_BEGIN };
				out << name;
				out << Token( OperatorType::COMMA );
				out << val;
				out << Token{ TokenType::TUPLE_END };
				return out;
			}
			static Expr pow( Expr const &val , Expr const &exp )
			{

			}
			static Expr createFloat2( Expr const &x , Expr const &y )
			{
				Expr out;
				out << Token( Type::FLOAT2 );
				out << Token( TokenType::TUPLE_BEGIN );
				out << x;
				out << Token( OperatorType::COMMA );
				out << y;
				out << Token( TokenType::TUPLE_END );
				return out;
			}
			static Expr createFloat3( Expr const &x , Expr const &y , Expr const &z )
			{
				Expr out;
				out << Token( Type::FLOAT3 );
				out << Token( TokenType::TUPLE_BEGIN );
				out << x;
				out << Token( OperatorType::COMMA );
				out << y;
				out << Token( OperatorType::COMMA );
				out << z;
				out << Token( TokenType::TUPLE_END );
				return out;
			}
			static Expr createFloat4( Expr const &x , Expr const &y , Expr const &z , Expr const &w )
			{
				Expr out;
				out << Token( Type::FLOAT4 );
				out << Token( TokenType::TUPLE_BEGIN );
				out << x;
				out << Token( OperatorType::COMMA );
				out << y;
				out << Token( OperatorType::COMMA );
				out << z;
				out << Token( OperatorType::COMMA );
				out << w;
				out << Token( TokenType::TUPLE_END );
				return out;
			}
			static Expr createFloat4( Expr const &xy , Expr const &z , Expr const &w )
			{
				Expr out;
				out << Token( Type::FLOAT4 );
				out << Token( TokenType::TUPLE_BEGIN );
				out << xy;
				out << Token( OperatorType::COMMA );
				out << z;
				out << Token( OperatorType::COMMA );
				out << w;
				out << Token( TokenType::TUPLE_END );
				return out;
			}
			static Expr createFloat4( Expr const &xyz , Expr const &w )
			{
				Expr out;
				out << Token( Type::FLOAT4 );
				out << Token( TokenType::TUPLE_BEGIN );
				out << xyz;
				out << Token( OperatorType::COMMA );
				out << w;
				out << Token( TokenType::TUPLE_END );
				return out;
			}
			static Expr setVertexPos( Expr const &pos )
			{
				Expr out;
				out << Token( BuiltInVariable::VERTEX_POS );
				out << Token( OperatorType::EQU );
				out << pos;
				return out;
			}
		};
		static inline Expr construct( Token const &a , Token const &b , OperatorType type )
		{
			Expr out( a );
			out << Token{ type };
			out << b;
			return out;
		}
		static inline Expr construct( Token const &a , Token const &b , OperatorType type0 , OperatorType type1 )
		{
			Expr out( a );
			out << Token{ type0 };
			out << Token{ type1 };
			out << b;
			return out;
		}
		static inline Expr operator<<( Token const &a , Token const &b )
		{
			return construct( a , b , OperatorType::EQU );
		}
		static inline Expr operator<<( Token const &a , Expr const &b )
		{
			Expr out( a );
			out << Token{ OperatorType::EQU };
			out << b;
			return out;
		}
		static inline Expr operator+( Token const &a , Token const &b )
		{
			return construct( a , b , OperatorType::PLUS );
		}
		static inline Expr operator-( Token const &a , Token const &b )
		{
			return construct( a , b , OperatorType::MINUS );
		}
		static inline Expr operator+=( Token const &a , Token const &b )
		{
			return construct( a , b , OperatorType::PLUS , OperatorType::EQU );
		}
		static inline Expr operator-=( Token const &a , Token const &b )
		{
			return construct( a , b , OperatorType::MINUS , OperatorType::EQU );
		}
		static inline Expr operator*( Token const &a , Token const &b )
		{
			return construct( a , b , OperatorType::MUL );
		}
		static inline Expr operator/( Token const &a , Token const &b )
		{
			return construct( a , b , OperatorType::DIV );
		}
		static inline Expr operator*=( Token const &a , Token const &b )
		{
			return construct( a , b , OperatorType::MUL , OperatorType::EQU );
		}
		static inline Expr operator/=( Token const &a , Token const &b )
		{
			return construct( a , b , OperatorType::DIV , OperatorType::EQU );
		}
		static Expr operator&( Token const &a , String const &field )
		{
			Expr out = a;
			out << Token{ OperatorType::DOT };
			out << Token( field );
			return out;
		}
		static Expr operator&( Token const &a , Swizzle swizzle )
		{
			Expr out = a;
			out << Token{ swizzle };
			return out;
		}
		typedef Pair< Type , String > ArgDef;
		struct Function
		{
			String name;
			Array< Pair< Type , String > > argv;
			Array< Expr > expressions;
			Type ret_type;
			Function() = default;
			Function( Type ret_type , String name , Array< Pair< Type , String > > const &argv ) :
				ret_type( ret_type ) ,
				argv( argv ) ,
				name( name )
			{

			}
			Function( Type ret_type , String name ) :
				ret_type( ret_type ) ,
				name( name )
			{

			}
			operator String() const
			{
				return name;
			}
			Expr operator()( Array< Expr > const &argv ) const
			{
				Expr out;
				out << name;
				out << Token{ TokenType::TUPLE_BEGIN };
				argv.foreach( [ &out , this ]( Expr const &expr )
				{
					out << expr;
					out << Token( OperatorType::COMMA );
				} );
				out.token_array.pop();
				out << Token{ TokenType::TUPLE_END };
				return out;
			}
			Function &addExpr( Expr const &expr )
			{
				expressions.push( expr );
				return *this;
			}
			String getRepr() const
			{
				String out;
				out += Graphics::Shaders::getTypeRepr( ret_type ) + " " + name + "( ";
				argv.foreach( [ &out , this ]( ArgDef const &argdef )
				{
					out += Graphics::Shaders::getTypeRepr( argdef.key ) + " " + argdef.value + " , ";
				} );
				if( !argv.isEmpty() )
				{
					out.pop();
					out.pop();
				}
				out += ")\n{\n";
				expressions.foreach( [ &out , this ]( Expr const &expr )
				{
					out += expr.getRepr() + ";\n";
				} );
				out += "}\n";
				return out;
			}
		};
		struct VaryingSlot
		{
			String name;
			Type type;
			uint location;
		};
		typedef Pair< Type , String > Arg;
		class Stage
		{
		public:
			Array< Tuple< uint , Type , String > > in;
			Array< Tuple< uint , Type , String > > out;
			Array< Pair< Type , String > > constants;
			Array< Pair< uint , String > > textures2d;
			Array< Function > functions;
			Function body;
			Stage() :
				body( Type::VOID , "main" )
			{

			}
			Stage &addOut( uint location , Type const &type , String const &name )
			{
				out.push( { location , type , name } );
				return *this;
			}
			Stage &addIn( uint location , Type const &type , String const &name )
			{
				in.push( { location , type , name } );
				return *this;
			}
			Stage &addTexture2D( uint location , String const &name )
			{
				textures2d.push( { location , name } );
				return *this;
			}
			Function &getBody()
			{
				return body;
			}
			Stage &addFunction( Function const &func )
			{
				functions.push( func );
				return *this;
			}
			String getRepr() const
			{
				String out;
				in.foreach( [ &out , this ]( auto const &expr )
				{
					out += "in location =" + String( expr.get< 0 >() ) + " "
						+ Graphics::Shaders::getTypeRepr( expr.get< 1 >() ) + " " + expr.get< 2 >() + ";\n";
				} );
				this->out.foreach( [ &out , this ]( auto const &expr )
				{
					out += "out location =" + String( expr.get< 0 >() ) + " "
						+ Graphics::Shaders::getTypeRepr( expr.get< 1 >() ) + " " + expr.get< 2 >() + ";\n";
				} );
				textures2d.foreach( [ &out , this ]( auto const &expr )
				{
					out += "texture2D location =" + String( expr.key ) + " " + expr.value + ";\n";
				} );
				constants.foreach( [ &out , this ]( auto const &expr )
				{
					out += "constant location =" + Graphics::Shaders::getTypeRepr( expr.key ) + " " + expr.value + ";\n";
				} );
				functions.foreach( [ &out , this ]( auto const &expr )
				{
					out += expr.getRepr();
				} );
				out += body.getRepr();
				return out;
			}
		};
	}
}