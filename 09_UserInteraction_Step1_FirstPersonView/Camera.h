#pragma once

#include <directxtk/SimpleMath.h>

namespace hlab {

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

class Camera {
  public:
    Matrix GetViewRow();
    Matrix GetProjRow();
    Vector3 GetEyePos();
    Vector3 GetViewDir() const { return m_viewDir; }
    Vector3 GetRightDir() const { return m_rightDir; }
    float GetSpeed() const { return m_speed; }

    void UpdateMouse(float mouseNdcX, float mouseNdcY);
    void MoveForward(float dt);
    void MoveRight(float dt);
    void SetAspectRatio(float aspect);
    
    void AddToPosition(const Vector3 &offset) { m_position += offset; }

    float lerp(float a, float b, float t) { return a + t * (b - a); }
    void SetSprinting(bool isSprinting, float dt) {
        float targetSpeed = isSprinting ? m_sprintSpeed : m_walkSpeed;
        float targetFov =
            isSprinting ? m_sprintProjFovAngleY : m_walkProjFovAngleY;

        m_speed = targetSpeed;
        m_projFovAngleY = lerp(m_projFovAngleY, targetFov, 5.0f * dt);
    }

  private:
    // 1인칭 시점은 FPS 게임을 떠올리시면 됩니다.
    // 가상 세계에 1인칭 시점 게임 캐릭터가 서있는 상황입니다.
    // 그 캐릭터의 눈의 위치에 카메라가 있습니다.
    // 그 캐릭터의 정면 방향이 카메라가 보는 방향입니다.

    // m_position : 월드 좌표계에서 카메라의 위치
    // m_viewDir : 카메라가 보는 방향, 걸어가는 방향
    // m_upDir : 위쪽 방향, 중력의 반대방향이 기본
    // m_rightDir : 오른쪽 방향, eyeDir과 upDir로부터 계산

    Vector3 m_position = Vector3(0.0f, 0.15f, 0.0f); // 0.15f는 눈높이 정도
    Vector3 m_viewDir = Vector3(0.0f, 0.0f, 1.0f);
    Vector3 m_upDir = Vector3(0.0f, 1.0f, 0.0f); // 이번 예제에서는 고정
    Vector3 m_rightDir = Vector3(1.0f, 0.0f, 0.0f);

    // roll, pitch, yaw
    // https://en.wikipedia.org/wiki/Aircraft_principal_axes
    float m_pitch = 0.0f;
    float m_yaw = 0.0f;

    float m_speed = 1.0f; // 움직이는 속도
    float m_sprintSpeed = 1.5f;
    float m_walkSpeed = 1.0f;

    // 프로젝션 옵션도 카메라 클래스로 이동
    float m_projFovAngleY = 70.0f;
    float m_sprintProjFovAngleY = 55.0f;
    float m_walkProjFovAngleY = 70.0f;

    float m_nearZ = 0.01f;
    float m_farZ = 100.0f;
    float m_aspect = 16.0f / 9.0f;
    bool m_usePerspectiveProjection = true;
};
} // namespace hlab
