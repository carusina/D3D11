#pragma once

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <iostream>
#include <vector>

#include "Camera.h"
#include "D3D11Utils.h"

namespace hlab {

using Microsoft::WRL::ComPtr;
using std::shared_ptr;
using std::vector;
using std::wstring;

// 모든 예제들이 공통적으로 사용할 기능들을 가지고 있는
// 부모 클래스
class AppBase {
  public:
    AppBase();
    virtual ~AppBase();

    float GetAspectRatio() const;

    int Run();

    virtual bool Initialize();
    virtual void UpdateGUI() = 0;
    virtual void Update(float dt) = 0;
    virtual void Render() = 0;

    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // 마우스 입력 처리
    virtual void OnMouseMove(WPARAM btnState, int mouseX, int mouseY);

  protected: // 상속 받은 클래스에서도 접근 가능
    bool InitMainWindow();
    bool InitDirect3D();
    bool InitGUI();

    void SetViewport();
    bool CreateRenderTargetView();

  public:
    // 변수 이름 붙이는 규칙은 VS DX11/12 기본 템플릿을 따릅니다.
    // 다만 변수 이름을 줄이기 위해 d3d는 생략했습니다.
    // 예: m_d3dDevice -> m_device
    int m_screenWidth; // 렌더링할 최종 화면의 해상도
    int m_screenHeight;
    int m_guiWidth = 0;
    HWND m_mainWindow;
    UINT numQualityLevels = 0;

    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;
    ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
    ComPtr<IDXGISwapChain> m_swapChain;

    ComPtr<ID3D11RasterizerState> m_rasterizerSate;
    ComPtr<ID3D11RasterizerState> m_wireRasterizerSate;
    bool m_drawAsWire = false;

    // Depth buffer 관련
    ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    ComPtr<ID3D11DepthStencilState> m_depthStencilState;

    D3D11_VIEWPORT m_screenViewport;

    // 시점을 결정하는 카메라 클래스 추가
    Camera m_camera;

    // 현재 키보드가 눌렸는지 상태를 저장하는 배열
    bool m_keyPressed[256] = {
        false,
    };
};
} // namespace hlab