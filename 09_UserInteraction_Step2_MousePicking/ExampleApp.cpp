#include "ExampleApp.h"

#include <directxtk/DDSTextureLoader.h> // 큐브맵 읽을 때 필요
#include <tuple>
#include <vector>

#include "GeometryGenerator.h"

namespace hlab {

using namespace std;
using namespace DirectX;

ExampleApp::ExampleApp() : AppBase() {}

bool ExampleApp::Initialize() {

    if (!AppBase::Initialize())
        return false;

    m_cubeMapping.Initialize(
        m_device, L"../Assets/Textures/Cubemaps/skybox/cubemap_bgra.dds",
        L"../Assets/Textures/Cubemaps/skybox/cubemap_diffuse.dds",
        L"../Assets/Textures/Cubemaps/skybox/cubemap_specular.dds");

    MeshData ground = GeometryGenerator::MakeSquare(2.0f);
    ground.textureFilename = "../Assets/Textures/blender_uv_grid_2k.png";
    m_meshGroupGround.Initialize(m_device, {ground});
    m_meshGroupGround.m_diffuseResView = m_cubeMapping.m_diffuseResView;
    m_meshGroupGround.m_specularResView = m_cubeMapping.m_specularResView;

    // 바닥으로 사용하기 위해 회전
    Matrix modelMat = Matrix::CreateRotationX(DirectX::XM_PIDIV2);
    Matrix invTransposeRow = modelMat;
    invTransposeRow.Translation(Vector3(0.0f));
    invTransposeRow = invTransposeRow.Invert().Transpose();

    // ConstantBuffer 초기화 (바닥은 변경 없음)
    m_meshGroupGround.m_basicVertexConstantData.model = modelMat.Transpose();
    m_meshGroupGround.m_basicVertexConstantData.invTranspose =
        invTransposeRow.Transpose();
    m_meshGroupGround.m_basicPixelConstantData.useTexture = true;
    m_meshGroupGround.m_basicPixelConstantData.material.diffuse = Vector3(1.0f);
    m_meshGroupGround.UpdateConstantBuffers(m_device, m_context);

    // 물체 1
    {
        MeshData sphere = GeometryGenerator::MakeSphere(0.1f, 20, 20);
        m_meshGroupSphere.Initialize(m_device, {sphere});
        m_meshGroupSphere.m_diffuseResView = m_cubeMapping.m_diffuseResView;
        m_meshGroupSphere.m_specularResView = m_cubeMapping.m_specularResView;
        Matrix modelMat = Matrix::CreateTranslation({-0.2f, 0.1f, 0.6f});
        Matrix invTransposeRow = modelMat;
        invTransposeRow.Translation(Vector3(0.0f));
        invTransposeRow = invTransposeRow.Invert().Transpose();
        m_meshGroupSphere.m_basicVertexConstantData.model =
            modelMat.Transpose();
        m_meshGroupSphere.m_basicVertexConstantData.invTranspose =
            invTransposeRow.Transpose();
        m_meshGroupSphere.m_basicPixelConstantData.useTexture = false;
        m_meshGroupSphere.m_basicPixelConstantData.material.diffuse =
            Vector3(0.5f, 0.5f, 0.5f);
        m_meshGroupSphere.m_basicPixelConstantData.material.specular =
            Vector3(0.0f);
        m_meshGroupSphere.UpdateConstantBuffers(m_device, m_context);
    }

    // 물체 2
    {
        // TODO:
    }

    BuildFilters();

    return true;
}

void ExampleApp::Update(float dt) {

    // 카메라의 이동
    if (m_useFirstPersonView) {
        if (m_keyPressed[87])
            m_camera.MoveForward(dt);
        if (m_keyPressed[83])
            m_camera.MoveForward(-dt);
        if (m_keyPressed[68])
            m_camera.MoveRight(dt);
        if (m_keyPressed[65])
            m_camera.MoveRight(-dt);
    }

    Matrix viewRow = m_camera.GetViewRow();
    Matrix projRow = m_camera.GetProjRow();
    Vector3 eyeWorld = m_camera.GetEyePos();

    // 큐브 매핑 Constant Buffer 업데이트
    m_cubeMapping.UpdateConstantBuffers(
        m_device, m_context, viewRow.Transpose(), projRow.Transpose());
    m_meshGroupGround.m_basicPixelConstantData.eyeWorld = eyeWorld;
    m_meshGroupGround.m_basicVertexConstantData.view = viewRow.Transpose();
    m_meshGroupGround.m_basicVertexConstantData.projection =
        projRow.Transpose();
    m_meshGroupGround.UpdateConstantBuffers(m_device, m_context);

    // 다른 물체들 Constat Buffer 업데이트
    m_meshGroupSphere.m_basicPixelConstantData.eyeWorld = eyeWorld;
    m_meshGroupSphere.m_basicVertexConstantData.view = viewRow.Transpose();
    m_meshGroupSphere.m_basicVertexConstantData.projection =
        projRow.Transpose();

    // TODO:

    m_meshGroupSphere.UpdateConstantBuffers(m_device, m_context);

    // TODO:

    if (m_dirtyflag && m_filters.size() > 1) {
        m_filters[1]->m_pixelConstData.threshold = m_threshold;
        m_filters[1]->UpdateConstantBuffers(m_device, m_context);
        m_filters.back()->m_pixelConstData.strength = m_strength;
        m_filters.back()->UpdateConstantBuffers(m_device, m_context);
        m_dirtyflag = 0;
    }
}

void ExampleApp::Render() {

    // RS: Rasterizer stage
    // OM: Output-Merger stage
    // VS: Vertex Shader
    // PS: Pixel Shader
    // IA: Input-Assembler stage

    SetViewport();

    float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    m_context->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);

    // 마우스 피킹에 사용할 indexRenderTarget도 초기화
    m_context->ClearRenderTargetView(m_indexRenderTargetView.Get(), clearColor);

    m_context->ClearDepthStencilView(m_depthStencilView.Get(),
                                     D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                     1.0f, 0);
    // Multiple render targets
    // 인덱스를 저장할 RenderTarget을 추가
    ID3D11RenderTargetView *targets[] = {m_renderTargetView.Get(),
                                         m_indexRenderTargetView.Get()};
    m_context->OMSetRenderTargets(2, targets, m_depthStencilView.Get());
    m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

    if (m_drawAsWire) {
        m_context->RSSetState(m_wireRasterizerSate.Get());
    } else {
        m_context->RSSetState(m_rasterizerSate.Get());
    }

    m_meshGroupGround.Render(m_context);
    m_meshGroupSphere.Render(m_context);
    // TODO:

    // 물체 렌더링 후 큐브매핑
    m_cubeMapping.Render(m_context);

    // 후처리 필터 시작하기 전에 Texture2DMS에 렌더링 된 결과를 Texture2D로 복사
    // MSAA Texture2DMS to Texture2D
    // https://stackoverflow.com/questions/24269813/directx-newb-multisampled-texture2d-with-depth-on-a-billboard
    ComPtr<ID3D11Texture2D> backBuffer;
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
    m_context->ResolveSubresource(m_tempTexture.Get(), 0, backBuffer.Get(), 0,
                                  DXGI_FORMAT_R8G8B8A8_UNORM);

    // 후처리 필터
    if (m_usePostProcessing) {
        for (auto &f : m_filters) {
            f->Render(m_context);
        }
    }

    // Picking
    {
        // TODO: m_pickColor 업데이트
        //  GPU->CPU는 화면 캡쳐 코드 참고
    }
}

void ExampleApp::BuildFilters() {

    m_filters.clear();

    // 해상도를 낮춰서 다운 샘플링
    auto copyFilter =
        make_shared<ImageFilter>(m_device, m_context, L"Sampling", L"Sampling",
                                 m_screenWidth, m_screenHeight);
    copyFilter->SetShaderResources({m_shaderResourceView});
    m_filters.push_back(copyFilter);

    for (int down = 2; down <= m_down; down *= 2) {
        auto downFilter = make_shared<ImageFilter>(
            m_device, m_context, L"Sampling", L"Sampling", m_screenWidth / down,
            m_screenHeight / down);

        if (down == 2) {
            downFilter->SetShaderResources({m_shaderResourceView});
        } else {
            downFilter->SetShaderResources(
                {m_filters.back()->m_shaderResourceView});
        }

        downFilter->m_pixelConstData.threshold = 0.0f;
        downFilter->UpdateConstantBuffers(m_device, m_context);
        m_filters.push_back(downFilter);
    }

    for (int down = m_down; down >= 1; down /= 2) {
        for (int i = 0; i < m_repeat; i++) {
            auto &prevResource = m_filters.back()->m_shaderResourceView;
            m_filters.push_back(make_shared<ImageFilter>(
                m_device, m_context, L"Sampling", L"BlurX",
                m_screenWidth / down, m_screenHeight / down));
            m_filters.back()->SetShaderResources({prevResource});

            auto &prevResource2 = m_filters.back()->m_shaderResourceView;
            m_filters.push_back(make_shared<ImageFilter>(
                m_device, m_context, L"Sampling", L"BlurY",
                m_screenWidth / m_down, m_screenHeight / m_down));
            m_filters.back()->SetShaderResources({prevResource2});
        }

        if (down > 1) {
            auto upFilter = make_shared<ImageFilter>(
                m_device, m_context, L"Sampling", L"Sampling",
                m_screenWidth / down * 2, m_screenHeight / down * 2);
            upFilter->SetShaderResources(
                {m_filters.back()->m_shaderResourceView});
            upFilter->m_pixelConstData.threshold = 0.0f;
            upFilter->UpdateConstantBuffers(m_device, m_context);
            m_filters.push_back(upFilter);
        }
    }

    auto combineFilter =
        make_shared<ImageFilter>(m_device, m_context, L"Sampling", L"Combine",
                                 m_screenWidth, m_screenHeight);
    combineFilter->SetShaderResources({copyFilter->m_shaderResourceView,
                                       m_filters.back()->m_shaderResourceView});
    combineFilter->SetRenderTargets(
        {this->m_renderTargetView}); // 렌더타겟 교체
    combineFilter->m_pixelConstData.strength = m_strength;
    combineFilter->UpdateConstantBuffers(m_device, m_context);
    m_filters.push_back(combineFilter);
}

void ExampleApp::UpdateGUI() {

    ImGui::Checkbox("Use FPV", &m_useFirstPersonView);
    ImGui::Checkbox("Use PostProc", &m_usePostProcessing);

    m_dirtyflag = 0;
    // m_dirtyflag +=
    //     ImGui::SliderFloat("Bloom Threshold", &m_threshold, 0.0f, 1.0f);
    // m_dirtyflag +=
    //     ImGui::SliderFloat("Bloom Strength", &m_strength, 0.0f, 3.0f);

    ImGui::Checkbox("Wireframe", &m_drawAsWire);
}

} // namespace hlab
