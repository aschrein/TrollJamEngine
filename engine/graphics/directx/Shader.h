#ifndef SHADER_H
#define SHADER_H
#include "DirectXDevice.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <string>
#include "COMUtil.h"
#include <locale>
#include <codecvt>
#include <view\3DMesh.h>
#include <linalg\mat.h>
#include <map>
enum ShaderType : int
{
	VERTEX = 0 , FRAGMENT = 1
};
class ShaderException : public std::exception
{
private:
	std::string msg;
public:
	ShaderException( std::string m ) : msg( m )
	{}
	const char *what() const override
	{
		return msg.c_str();
	}
};
class Shader
{
protected:
	ID3DBlob *shader_blob = nullptr , *error_blob = nullptr;
	ShaderType type;
public:
};
class PixelShader : public Shader
{
public:
	ID3D11PixelShader* pixel_shader = nullptr;
public:
	PixelShader( ID3D11Device *dev , std::string filename )
	{
		type = ShaderType::FRAGMENT;
		std::wstring_convert< std::codecvt_utf8_utf16< wchar_t > > converter;
		std::wstring widefilename = converter.from_bytes( filename );
		D3DCompileFromFile( widefilename.c_str() , NULL , NULL , "main" , "ps_4_0" , D3DCOMPILE_ENABLE_STRICTNESS , 0 , &shader_blob , &error_blob );
		dev->CreatePixelShader( shader_blob->GetBufferPointer() , shader_blob->GetBufferSize() , NULL , &pixel_shader );
		COMRelease( shader_blob );
		COMRelease( error_blob );  
	}
	~PixelShader()
	{
		COMRelease( pixel_shader );
	}
	void bind( ID3D11DeviceContext *context )
	{
		context->PSSetShader( pixel_shader , NULL , 0 );
	}
};
class VertexShader : public Shader
{
public:
	ID3D11VertexShader* vertex_shader = nullptr;
	ID3D11InputLayout* input_layout = nullptr;
public:
	static DXGI_FORMAT getDXFormat( ElemType type )
	{
		switch( type )
		{
		case ElemType::F32:
			return DXGI_FORMAT_R32_FLOAT;
		case ElemType::F32x2:
			return DXGI_FORMAT_R32G32_FLOAT;
		case ElemType::F32x3:
			return DXGI_FORMAT_R32G32B32_FLOAT;
		case ElemType::F32x4:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		default:
			return DXGI_FORMAT_R32_FLOAT;
		}
	}
	VertexShader( ID3D11Device *dev , std::string filename , std::vector< VertexElem > comp )
	{
		type = ShaderType::VERTEX;
		std::wstring_convert< std::codecvt_utf8_utf16< wchar_t > > converter;
		std::wstring widefilename = converter.from_bytes( filename );
		D3DCompileFromFile( widefilename.c_str() , NULL , NULL , "main" , "vs_4_0" , D3DCOMPILE_ENABLE_STRICTNESS , 0 , &shader_blob , &error_blob );
		dev->CreateVertexShader( shader_blob->GetBufferPointer() , shader_blob->GetBufferSize() , NULL , &vertex_shader );
		D3D11_INPUT_ELEMENT_DESC *layout_desc = new D3D11_INPUT_ELEMENT_DESC[ comp.size() ];
		for( int i = 0; i < comp.size(); i++ )
		{
			layout_desc[ i ] =
			{
				VertexElem::getAttributeName( comp[ i ].target )
				, 0 , getDXFormat( comp[ i ].type ) , 0 , ( UINT )comp[ i ].offset , D3D11_INPUT_PER_VERTEX_DATA , 0
			};
		}
		dev->CreateInputLayout( layout_desc , comp.size() , shader_blob->GetBufferPointer() , shader_blob->GetBufferSize() , &input_layout );
		delete[] layout_desc;
		COMRelease( shader_blob );
		COMRelease( error_blob );
	}
	void bind( ID3D11DeviceContext *context )
	{
		context->IASetInputLayout( input_layout );
		context->VSSetShader( vertex_shader , NULL , 0 );
	}
	~VertexShader()
	{
		COMRelease( vertex_shader );
		COMRelease( input_layout );
	}
};
class Pipeline
{
public:
	PixelShader pixel_shader;
	VertexShader vertex_shader;
	std::map< int , ID3D11Buffer* > constant_buffers;
public:
	Pipeline( ID3D11Device *dev , std::string fragment_filename , std::string vertex_filename , std::vector< VertexElem > comp ):
		pixel_shader( dev , fragment_filename ) ,
		vertex_shader( dev , vertex_filename , comp )
	{
	}
	void setConstantData( ID3D11Device *dev , ID3D11DeviceContext *context , int slot , void const *dataptr , size_t size )
	{
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = size;
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		ID3D11Buffer* buffer;
		dev->CreateBuffer( &cbDesc , NULL , &buffer );
		auto it = constant_buffers.find( slot );
		if( it != constant_buffers.end() )
		{
			it->second->Release();
			constant_buffers.erase( it );
		}
		constant_buffers[ slot ] = buffer;
		D3D11_MAPPED_SUBRESOURCE map;
		context->Map( buffer , 0 , D3D11_MAP_WRITE_DISCARD , 0 , &map );
		void *raw_ptr = map.pData;
		memcpy( raw_ptr , dataptr , size );
		context->Unmap( buffer , 0 );
	}
	~Pipeline()
	{
		for( auto buf : constant_buffers )
		{
			buf.second->Release();
		}
	}
	void bind( ID3D11DeviceContext *context )
	{
		pixel_shader.bind( context );
		vertex_shader.bind( context );
		for( auto buf : constant_buffers )
		{
			context->VSSetConstantBuffers( buf.first , 1 , &buf.second );
		}
	}
};
#endif