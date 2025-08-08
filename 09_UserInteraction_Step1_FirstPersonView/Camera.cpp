#include "Camera.h"

namespace hlab {

using namespace std;
using namespace DirectX;

Matrix Camera::GetViewRow() {
    // 렌더링에 사용할 View 행렬을 만들어주는 부분
    // 이번 예제에서는 upDir이 Y로 고정되었다고 가정합니다.
    // 시점 변환은 가상 세계가 통째로 반대로 움직이는 것과 동일
    // m_pitch가 고개를 숙이는 회전이라서 -가 두번 붙어서 생략

    // TODO
    return XMMatrixLookAtLH(m_position, m_position + m_viewDir, m_upDir);
    }

Vector3 Camera::GetEyePos() { return m_position; }

void Camera::UpdateMouse(float mouseNdcX, float mouseNdcY) {
    // 얼마나 회전할지 계산
    // https://en.wikipedia.org/wiki/Aircraft_principal_axes
    m_yaw = mouseNdcX * DirectX::XM_2PI;     // 좌우 360도
    m_pitch = -mouseNdcY * DirectX::XM_PIDIV2; // 위 아래 90도

    // 이동할 때 기준이 되는 정면/오른쪽 방향 계산

    //TODO:
    m_viewDir = Vector3::Transform(
        Vector3(0.0f, 0.0f, 1.0f),
        Matrix::CreateRotationY(this->m_yaw) * Matrix::CreateRotationX(this->m_pitch)); // m_yaw만큼 회전
    m_viewDir.Normalize();

    const Vector3 worldUp = Vector3(0.0f, 1.0f, 0.0f);
    m_rightDir = worldUp.Cross(m_viewDir);
    m_rightDir.Normalize();

    m_upDir = m_viewDir.Cross(m_rightDir);
    m_upDir.Normalize();
}

void Camera::MoveForward(float dt) {
    // 이동후의_위치 = 현재_위치 + 이동방향 * 속도 * 시간차이;
    m_position += m_viewDir * m_speed * dt;
}

void Camera::MoveRight(float dt) {
    // 이동후의_위치 = 현재_위치 + 이동방향 * 속도 * 시간차이;
    m_position += m_rightDir * m_speed * dt;
}

void Camera::SetAspectRatio(float aspect) { m_aspect = aspect; }

Matrix Camera::GetProjRow() {
    return m_usePerspectiveProjection
               ? XMMatrixPerspectiveFovLH(XMConvertToRadians(m_projFovAngleY),
                                          m_aspect, m_nearZ, m_farZ)
               : XMMatrixOrthographicOffCenterLH(-m_aspect, m_aspect, -1.0f,
                                                 1.0f, m_nearZ, m_farZ);
}
} // namespace hlab