#include "TriangleD3D11.h"

void TriangleD3D11::draw(ID3D11Device1* device,
                         kwui::CustomElementPaintContextInterface& painter,
                         const kwui::CustomElementPaintOption& po)
{
    auto dpi_scale = painter.getDpiScale();
    auto pixel_width = po.width * dpi_scale;
    auto pixel_height = po.height * dpi_scale;

    // Create backbuffer
    CD3D11_TEXTURE2D_DESC tex_desc{
        DXGI_FORMAT_B8G8R8A8_UNORM,
        (UINT)pixel_width,
        (UINT)pixel_height,
        1,
        1,
        D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
    };
    ComPtr<ID3D11Texture2D> tex;
    device->CreateTexture2D(&tex_desc, nullptr, tex.GetAddressOf());
    ComPtr<ID3D11RenderTargetView> rtv;
    CD3D11_RENDER_TARGET_VIEW_DESC rtv_desc{
        D3D11_RTV_DIMENSION_TEXTURE2D,
        tex_desc.Format
    };
    device->CreateRenderTargetView(tex.Get(), &rtv_desc, rtv.GetAddressOf());
    ComPtr<ID3D11ShaderResourceView> srv;
    CD3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{
        D3D11_SRV_DIMENSION_TEXTURE2D,
        tex_desc.Format
    };
    device->CreateShaderResourceView(tex.Get(), &srv_desc, srv.GetAddressOf());

    // Create Rasterizer
    ComPtr<ID3D11RasterizerState> rasterizer;
    {
        D3D11_RASTERIZER_DESC desc = {};
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_BACK;

        device->CreateRasterizerState(&desc, &rasterizer);
    }

    // Create triangle
    struct Vertex
    {
        float position[3];
        float color[4];
    };

    ComPtr<ID3D11Buffer> vbuf;
    {
        Vertex vertices[] = {
            {{+0.0f, +0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
            {{+0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
            {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        };

        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = sizeof(vertices);
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA data = {};
        data.pSysMem = vertices;
        device->CreateBuffer(&desc, &data, &vbuf);
    }

    // Create shader
    HINSTANCE dll = LoadLibraryA("d3dcompiler_47.dll");
    pD3DCompile d3d_compile_proc = (pD3DCompile)GetProcAddress(dll, "D3DCompile");
#define D3DCompile d3d_compile_proc

    const char* hlsl = R"(
    struct VertexIn {
      float3 position : POS;
      float4 color : COL;
    };

    struct VertexOut {
      float4 position : SV_POSITION;
      float4 color : COL;
    };

    VertexOut vs_main(VertexIn input) {
      VertexOut output;
      output.position = float4(input.position, 1.0);
      output.color = input.color;
      return output;
    }

    float4 ps_main(VertexOut input) : SV_TARGET {
      return input.color;
    }
  )";

    UINT compile_flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR |
        D3DCOMPILE_ENABLE_STRICTNESS |
        D3DCOMPILE_WARNINGS_ARE_ERRORS;
    compile_flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

    ComPtr<ID3D11InputLayout> input_layout;
    ComPtr<ID3D11VertexShader> vertex_shader;
    {
        ID3DBlob* vs_blob = nullptr;
        D3DCompile(hlsl, strlen(hlsl), nullptr, nullptr,
                   D3D_COMPILE_STANDARD_FILE_INCLUDE, "vs_main", "vs_5_0",
                   compile_flags, 0, &vs_blob, nullptr);

        device->CreateVertexShader(vs_blob->GetBufferPointer(),
                                   vs_blob->GetBufferSize(), nullptr,
                                   &vertex_shader);

        D3D11_INPUT_ELEMENT_DESC desc[] = {
            {
                "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
                D3D11_INPUT_PER_VERTEX_DATA, 0
            },
            {
                "COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
                D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
            },
        };

        device->CreateInputLayout(desc, ARRAYSIZE(desc),
                                  vs_blob->GetBufferPointer(),
                                  vs_blob->GetBufferSize(), &input_layout);
    }

    ComPtr<ID3D11PixelShader> pixel_shader;
    {
        ID3DBlob* ps_blob = nullptr;
        D3DCompile(hlsl, strlen(hlsl), nullptr, nullptr,
                   D3D_COMPILE_STANDARD_FILE_INCLUDE, "ps_main", "ps_5_0",
                   compile_flags, 0, &ps_blob, nullptr);

        device->CreatePixelShader(ps_blob->GetBufferPointer(),
                                  ps_blob->GetBufferSize(), nullptr, &pixel_shader);
        ps_blob->Release();
    }

    // Render
    ComPtr<ID3D11DeviceContext1> ctx;
    device->GetImmediateContext1(ctx.GetAddressOf());

    float bg_color[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    ID3D11Buffer* vbufs[] = {vbuf.Get()};
    ID3D11RenderTargetView* rtvs[] = {rtv.Get()};

    D3D11_VIEWPORT viewport = {
        0.0f, 0.0f, pixel_width,
        pixel_height, 0.0f, 1.0f
    };

    ctx->ClearRenderTargetView(rtv.Get(), bg_color);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->IASetInputLayout(input_layout.Get());
    ctx->IASetVertexBuffers(0, 1, vbufs, &stride, &offset);
    ctx->VSSetShader(vertex_shader.Get(), nullptr, 0);
    ctx->RSSetViewports(1, &viewport);
    ctx->RSSetState(rasterizer.Get());
    ctx->PSSetShader(pixel_shader.Get(), nullptr, 0);
    ctx->OMSetRenderTargets(1, rtvs, nullptr);
    ctx->Draw(3, 0);

    painter.setFillBitmap(rtv.Get(), dpi_scale);
    painter.drawRoundedRect(po.left, po.top, po.width, po.height, 8);
}
