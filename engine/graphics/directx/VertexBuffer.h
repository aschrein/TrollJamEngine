#pragma once

#include "DirectXDevice.h"
#include "Shader.h"
#include <view\3DMesh.h>
#include <d3d11.h>
#include <functional>
#include <d3dcompiler.h>
#include <string>

class VertexBuffer
{
private:
	int vertex_buffer_bytes = 0 , index_buffer_bytes = 0;
	ID3D11Buffer *vertex_buffer = NULL;
	ID3D11Buffer *index_buffer = NULL;
public:
	static void drawMeshImmidiate( ID3D11Device *dev , ID3D11DeviceContext *context , Mesh const *mesh , Pipeline &pipe )
	{
		HRESULT err = 0;
		int vertex_buf_size = mesh->vertex_count * mesh->vertex_size;
		int index_buf_size = mesh->index_count * sizeof( int );
		D3D11_BUFFER_DESC desc;
		memset( &desc , 0 , sizeof( D3D11_BUFFER_DESC ) );
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = vertex_buf_size;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		ID3D11Buffer *vertex_buffer;
		{
			err = dev->CreateBuffer( &desc , NULL , &vertex_buffer );
			D3D11_MAPPED_SUBRESOURCE mapped_resource;
			err = context->Map( vertex_buffer , 0 , D3D11_MAP_WRITE_DISCARD , 0 , &mapped_resource );
			{
				void * raw_ptr = mapped_resource.pData;
				memcpy( raw_ptr , mesh->vertex_data , vertex_buf_size );
			}
			context->Unmap( vertex_buffer , 0 );
		}
		ID3D11Buffer *index_buffer;
		{
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			desc.ByteWidth = index_buf_size;
			err = dev->CreateBuffer( &desc , NULL , &index_buffer );
			D3D11_MAPPED_SUBRESOURCE mapped_resource;
			err = context->Map( index_buffer , 0 , D3D11_MAP_WRITE_DISCARD , 0 , &mapped_resource );
			{
				void * raw_ptr = mapped_resource.pData;
				memcpy( raw_ptr , mesh->index_data , index_buf_size );
			}
			context->Unmap( index_buffer , 0 );
		}

		ID3D11RasterizerState*   rasterazer_state = NULL;
		ID3D11DepthStencilState *depthstencil_state = NULL;
		ID3D11BlendState* blend_state = NULL;
		{
			D3D11_RASTERIZER_DESC desc;
			ZeroMemory( &desc , sizeof( desc ) );
			desc.FillMode = D3D11_FILL_SOLID;
			desc.CullMode = D3D11_CULL_NONE;
			desc.ScissorEnable = false;
			desc.DepthClipEnable = false;
			dev->CreateRasterizerState( &desc , &rasterazer_state );
		}
		{
			D3D11_DEPTH_STENCIL_DESC desc;
			ZeroMemory( &desc , sizeof( D3D11_DEPTH_STENCIL_DESC ) );
			desc.DepthEnable = FALSE;
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			desc.DepthFunc = D3D11_COMPARISON_LESS;
			desc.StencilEnable = FALSE;
			dev->CreateDepthStencilState( &desc , &depthstencil_state );
		}
		{
			D3D11_BLEND_DESC desc;
			ZeroMemory( &desc , sizeof( desc ) );
			desc.AlphaToCoverageEnable = false;
			desc.RenderTarget[ 0 ].BlendEnable = false;
			desc.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			desc.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			desc.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;
			desc.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			desc.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;
			desc.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			desc.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			dev->CreateBlendState( &desc , &blend_state );
		}

		const float blendFactor[ 4 ] = { 0.f , 0.f , 0.f , 0.f };
		context->OMSetBlendState( blend_state , blendFactor , 0xffffffff );
		context->RSSetState( rasterazer_state );
		context->OMSetDepthStencilState( depthstencil_state , 0 );

		unsigned int stride = mesh->vertex_size;
		unsigned int offset = 0;
		context->IASetIndexBuffer( index_buffer , DXGI_FORMAT_R32_UINT , 0 );
		context->IASetVertexBuffers( 0 , 1 , &vertex_buffer , &stride , &offset );
		context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		pipe.bind( context );
		context->DrawIndexed( mesh->index_count , 0 , 0 );

		vertex_buffer->Release();
		index_buffer->Release();
		blend_state->Release();
		rasterazer_state->Release();
		depthstencil_state->Release();
	}
	
};