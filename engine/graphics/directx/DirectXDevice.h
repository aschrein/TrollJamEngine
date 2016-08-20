#ifndef DIRECTXDEV_H
#define DIRECTXDEV_H
#include <Windows.h>
#include <d3d11.h>
#include <view\Window.hpp>
#include <vector>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
//#pragma comment(lib, "d3dx11.lib")
//#pragma comment(lib, "dxgi.lib")

//minimal directx helper class. no safe checks at all.
class DirectXDevice
{
public:
	ID3D11Device           *dev = NULL;
	ID3D11DeviceContext    *context = NULL;
	IDXGISwapChain         *swap_chain = NULL;
	ID3D11RenderTargetView *render_target_view = NULL;
	ID3D11DepthStencilView* depth_buffer_view = NULL;
	ID3D11RasterizerState*   rasterazer_state = NULL;
	ID3D11DepthStencilState *depthstencil_state = NULL;
	ID3D11BlendState* blend_state = NULL;
	int width , height;
public:
	void init( Window const &wnd )
	{
		HRESULT hr = S_OK;
		width = wnd.getWidth();
		height = wnd.getHeight();
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory( &sd , sizeof( sd ) );
		sd.BufferCount = 1;
		sd.BufferDesc.Width = wnd.getWidth();
		sd.BufferDesc.Height = wnd.getHeight();
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = wnd.getHandler();
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		D3D_FEATURE_LEVEL tour_fl[] =
		{
			D3D_FEATURE_LEVEL_11_0
		};
		D3D_FEATURE_LEVEL flRes;
		hr = D3D11CreateDeviceAndSwapChain(
			0 ,
			D3D_DRIVER_TYPE_HARDWARE ,
			NULL ,
			0 ,
			tour_fl ,
			1 ,
			D3D11_SDK_VERSION ,
			&sd ,
			&swap_chain ,
			&dev ,
			&flRes ,
			&context
			);

		dev->GetImmediateContext( &context );
		ID3D11Texture2D *texture;
		hr = swap_chain->GetBuffer( 0 , __uuidof( ID3D11Texture2D ) , ( LPVOID * )&texture );
		hr = dev->CreateRenderTargetView( texture , NULL , &render_target_view );
		texture->Release();
		{
			D3D11_TEXTURE2D_DESC desc;
			ZeroMemory( &desc , sizeof( desc ) );
			desc.Width = wnd.getWidth();
			desc.Height = wnd.getHeight();
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			ID3D11Texture2D* depth_texture = NULL;
			dev->CreateTexture2D( &desc , 0 , &depth_texture );
			dev->CreateDepthStencilView( depth_texture , 0 , &depth_buffer_view );
		}

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
	}
	void clearScreen()
	{
		bind();
		float ClearColor[ 4 ] = { 0.5f , 0.55f , 0.6f , 1.0f };
		context->ClearRenderTargetView( render_target_view , ClearColor );
	}
	void bind() const
	{
		context->OMSetRenderTargets( 1 , &render_target_view , depth_buffer_view );
		D3D11_VIEWPORT vp;
		vp.Width = width;
		vp.Height = height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		context->RSSetViewports( 1 , &vp );
		const float blendFactor[ 4 ] = { 0.f , 0.f , 0.f , 0.f };
		context->OMSetBlendState( blend_state , blendFactor , 0xffffffff );
		context->RSSetState( rasterazer_state );
		context->OMSetDepthStencilState( depthstencil_state , 0 );
	}
	void present()
	{
		swap_chain->Present( 0 , 0 );
	}
	void release()
	{
		render_target_view->Release();
		rasterazer_state->Release();
		depthstencil_state->Release();
		blend_state->Release();
		//depth_buffer_view->Release();
		dev->Release();
		context->Release();
		swap_chain->Release();
	}
	ID3D11PixelShader* createPixelShader( ID3DBlob* shader_blob )
	{
		ID3D11PixelShader* pixel_shader = nullptr;
		dev->CreatePixelShader( shader_blob->GetBufferPointer() , shader_blob->GetBufferSize() , nullptr , &pixel_shader );
		return pixel_shader;
	}
	ID3D11VertexShader* createVertexShader( ID3DBlob* shader_blob )
	{
		ID3D11VertexShader* vertex_shader = nullptr;
		dev->CreateVertexShader( shader_blob->GetBufferPointer() , shader_blob->GetBufferSize() , nullptr , &vertex_shader );
		return vertex_shader;
	}
	ID3D11InputLayout* createInputLayout( ID3DBlob* shader_blob , std::vector< D3D11_INPUT_ELEMENT_DESC > comp )
	{
		ID3D11InputLayout* input_layout = NULL;
		dev->CreateInputLayout( &( comp[ 0 ] ) , comp.size() , shader_blob->GetBufferPointer() , shader_blob->GetBufferSize() , &input_layout );
		return input_layout;
	}
	ID3D11Buffer *createBuffer( D3D11_BUFFER_DESC *buffer_desc , D3D11_SUBRESOURCE_DATA *subres )
	{
		ID3D11Buffer *out = NULL;
		dev->CreateBuffer( buffer_desc , subres , &out );
		return out;
	}
};
#endif