#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <iostream>
#include <vector>

#include <d3d11.h>
#include <vector>
#include <windows.h>
#include <wrl/client.h> // ComPtr

namespace hlab {

using Microsoft::WRL::ComPtr;

struct Mesh {
    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> indexBuffer;

    ComPtr<ID3D11Buffer> vertexConstantBuffer;
    ComPtr<ID3D11Buffer> pixelConstantBuffer;

    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11ShaderResourceView> textureResourceView;

    UINT m_indexCount = 0;
};
} // namespace hlab
