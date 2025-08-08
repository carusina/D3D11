#pragma once

#include <string>
#include <wrl/client.h>

#include "D3D11Utils.h"
#include "GeometryGenerator.h"
#include "Material.h"
#include "Vertex.h"

namespace hlab {

using Microsoft::WRL::ComPtr;

class CubeMapping {
  public:
    void Initialize(ComPtr<ID3D11Device> &device,
                    const wchar_t *originalFilename,
                    const wchar_t *diffuseFilename,
                    const wchar_t *specularFilename);

    void UpdateConstantBuffers(ComPtr<ID3D11Device> &device,
                               ComPtr<ID3D11DeviceContext> &context,
                               const Matrix &viewCol, const Matrix &projCol);

    void Render(ComPtr<ID3D11DeviceContext> &context);

  public:
    struct VertexConstantData {
        Matrix viewProj; // 미리 곱해서 사용
    };

    static_assert((sizeof(VertexConstantData) % 16) == 0,
                  "Constant Buffer size must be 16-byte aligned");

  public:
    // Pre-filter가 되지 않은 원본 텍스춰
    ComPtr<ID3D11ShaderResourceView> m_originalResView;

    // IBL을 위해 다른 물체들 그릴때 사용    
    ComPtr<ID3D11ShaderResourceView> m_diffuseResView;
    ComPtr<ID3D11ShaderResourceView> m_specularResView;

  private:
    std::shared_ptr<Mesh> m_cubeMesh;

    ComPtr<ID3D11SamplerState> m_samplerState;

    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;

    CubeMapping::VertexConstantData vertexConstantData;
};
} // namespace hlab