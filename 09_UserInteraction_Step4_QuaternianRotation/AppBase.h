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
    virtual void UpdateMousePickColor();

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

    // 렌더타겟의 Texture2DMS를 Texture2D로 복사하기 위한 임시 텍스춰
    ComPtr<ID3D11Texture2D> m_tempTexture;

    // Picking을 위한 Index를 저장할 텍스춰
    ComPtr<ID3D11Texture2D> m_indexTexture;
    ComPtr<ID3D11Texture2D> m_indexTempTexture;    // Texture2DMS -> Texture2D
    ComPtr<ID3D11Texture2D> m_indexStagingTexture; // 1x1 작은 크기
    ComPtr<ID3D11RenderTargetView> m_indexRenderTargetView;
    uint8_t m_pickColor[4] = {
        0,
    }; // 이 색을 이용해서 물체가 선택(pick)되었는지 판단

    ComPtr<ID3D11RasterizerState> m_rasterizerSate;
    ComPtr<ID3D11RasterizerState> m_wireRasterizerSate;
    bool m_drawAsWire = false;
    bool m_usePostProcessing = true;

    // Depth buffer 관련
    ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    ComPtr<ID3D11DepthStencilState> m_depthStencilState;

    D3D11_VIEWPORT m_screenViewport;

    // 시점을 결정하는 카메라 클래스 추가
    Camera m_camera;
    bool m_useFirstPersonView = false;

    bool m_keyPressed[256] = {
        false,
    };

    bool m_leftButton = false;
    bool m_selected = false;

    // 다음 렌더링할 때 화면 캡쳐
    bool m_captureFlag = false;

    // 마우스 커서 위치 저장 (Picking에 사용)
    int m_cursorX = 0;
    int m_cursorY = 0;
    float m_cursorNdcX = 0.0f;
    float m_cursorNdcY = 0.0f;
};
} // namespace hlab