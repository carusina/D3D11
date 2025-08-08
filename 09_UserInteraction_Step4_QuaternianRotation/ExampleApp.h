#pragma once

#include <algorithm>
#include <iostream>
#include <memory>

#include "AppBase.h"
#include "CubeMapping.h"
#include "GeometryGenerator.h"
#include "Light.h"
#include "BasicMeshGroup.h"
#include "ImageFilter.h"

namespace hlab {

using DirectX::SimpleMath::Vector3;
using DirectX::BoundingSphere;

class ExampleApp : public AppBase {
  public:
    ExampleApp();

    virtual bool Initialize() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

    void BuildFilters();

  protected:
    BasicMeshGroup m_mainSphere;
    BoundingSphere m_mainBoundingSphere;
    BasicMeshGroup m_cursorSphere;
    CubeMapping m_cubeMapping;

    bool m_usePerspectiveProjection = true;

    // 블룸 효과 옵션
    int m_dirtyflag = 1; // 처음에 한 번 실행
    int m_down = 4;
    int m_repeat = 3;
    float m_threshold = 0.0f;
    float m_strength = 0.8f;

    // 후처리 필터
    std::vector<shared_ptr<ImageFilter>> m_filters;

    // Euler Angles
    float m_eulerAlpha = 0.0f; // x축에 대한 회전
    float m_eulerBeta = 0.0f; // y축에 대한 회전
    float m_eulerGamma = 0.0f; // z축에 대한 회전

    // Quaternion Rotation
    float m_quatX = 1.0f;
    float m_quatY = 0.0f;
    float m_quatZ = 0.0f;
    float m_quatTheta = 0.0f;
};
} // namespace hlab