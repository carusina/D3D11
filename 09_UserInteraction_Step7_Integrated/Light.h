#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxtk/SimpleMath.h>
#include <memory>

#define MAX_LIGHTS 3

namespace hlab {

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;

// Α¶Έν
struct Light {
    Vector3 strength = Vector3(1.0f);              // 12
    float fallOffStart = 0.0f;                     // 4
    Vector3 direction = Vector3(0.0f, 0.0f, 1.0f); // 12
    float fallOffEnd = 10.0f;                      // 4
    Vector3 position = Vector3(0.0f, 0.0f, -2.0f); // 12
    float spotPower = 100.0f;                      // 4
};
}
