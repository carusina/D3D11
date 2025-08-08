#pragma once

#include "D3D11Utils.h"
#include "GeometryGenerator.h"
#include "Mesh.h"

namespace hlab {

using DirectX::SimpleMath::Vector4;

class ImageFilter {
  public:
    ImageFilter(ComPtr<ID3D11Device> &device,
                ComPtr<ID3D11DeviceContext> &context,
                const wstring vertexPrefix, const wstring pixelPrefix,
                int width, int height);

    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context,
                    const wstring vertexPrefix, const wstring pixelPrefix,
                    int width, int height);

    void UpdateConstantBuffers(ComPtr<ID3D11Device> &device,
                               ComPtr<ID3D11DeviceContext> &context);

    void Render(ComPtr<ID3D11DeviceContext> &context);

    void SetShaderResources(
        const std::vector<ComPtr<ID3D11ShaderResourceView>> &resources);

    void SetRenderTargets(
        const std::vector<ComPtr<ID3D11RenderTargetView>> &targets);

  public:
    ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
    ComPtr<ID3D11RenderTargetView> m_renderTargetView;

    struct SamplingPixelConstantData {
        float dx;
        float dy;
        float threshold;
        float strength;
        Vector4 options;
    };

    SamplingPixelConstantData m_pixelConstData;

  protected:
    shared_ptr<Mesh> m_mesh;

    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;
    ComPtr<ID3D11SamplerState> m_samplerState;
    ComPtr<ID3D11RasterizerState> m_rasterizerSate;

    D3D11_VIEWPORT m_viewport;

    // Do not delete pointers
    std::vector<ID3D11ShaderResourceView *> m_shaderResources;
    std::vector<ID3D11RenderTargetView *> m_renderTargets;
};
} // namespace hlab