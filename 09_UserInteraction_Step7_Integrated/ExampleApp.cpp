#include "ExampleApp.h"

#include <DirectXCollision.h> // 구와 광선 충돌 계산에 사용
#include <directxtk/DDSTextureLoader.h>
#include <tuple>
#include <vector>

#include "GeometryGenerator.h"

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

ExampleApp::ExampleApp() : AppBase() {}

bool ExampleApp::Initialize() {

    if (!AppBase::Initialize())
        return false;

    m_cubeMapping.Initialize(
        m_device, L"../Assets/Textures/Cubemaps/skybox/cubemap_bgra.dds",
        L"../Assets/Textures/Cubemaps/skybox/cubemap_diffuse.dds",
        L"../Assets/Textures/Cubemaps/skybox/cubemap_specular.dds");

    // Main Sphere
    {
        Vector3 center(0.0f, 0.3f, 3.0f);
        float radius = 1.3f;
        MeshData sphere = GeometryGenerator::MakeSphere(radius, 100, 100); // Mesh는 단순한 3D 모델 (Sphere, Square...)
        sphere.textureFilename = "../Assets/Textures/earth.jpg";
        m_mainSphere.Initialize(m_device, {sphere}); // BasicMeshGroup는 Mesh의 집합, 복잡한 3D 모델 (ex: Sphere와 Square를 혼합해 만든 사람 모양)
        m_mainSphere.m_diffuseResView = m_cubeMapping.m_diffuseResView;
        m_mainSphere.m_specularResView = m_cubeMapping.m_specularResView;
        m_mainSphere.UpdateModelWorld(Matrix::CreateTranslation(center));
        m_mainSphere.m_basicPixelConstantData.useTexture = true;
        m_mainSphere.m_basicPixelConstantData.material.diffuse = Vector3(1.0f);
        m_mainSphere.m_basicPixelConstantData.material.specular = Vector3(0.3f);
        m_mainSphere.m_basicPixelConstantData.indexColor =
            Vector4(1.0f, 0.0, 0.0, 0.0);
        m_mainSphere.UpdateConstantBuffers(m_device, m_context);

        // 동일한 크기와 위치에 BoundingSphere 만들기
        m_mainBoundingSphere = BoundingSphere(center, radius);
    }

    // Box
    {
        Vector3 center(2.0f, 0.3f, 3.0f);
        float extent = 0.5f;
        MeshData box = GeometryGenerator::MakeBox(extent);
        m_box.Initialize(m_device, {box});
        m_box.m_diffuseResView = m_cubeMapping.m_diffuseResView;
        m_box.m_specularResView = m_cubeMapping.m_specularResView;
        m_box.UpdateModelWorld(Matrix::CreateTranslation(center));
        m_box.m_basicPixelConstantData.useTexture = false;
        m_box.m_basicPixelConstantData.material.diffuse = Vector3(0.5f);
        m_box.m_basicPixelConstantData.material.specular = Vector3(0.1f);
        m_box.m_basicPixelConstantData.indexColor =
            Vector4(0.0f, 1.0f, 0.0f, 0.0f);
        m_box.UpdateConstantBuffers(m_device, m_context);

        m_boxBoundingSphere = BoundingSphere(center, extent * sqrt(2.0f));
    }

    // Cursor Sphere
    // Main sphere와의 충돌이 감지되면 월드 공간에 작게 그려지는 구
    {
        MeshData sphere = GeometryGenerator::MakeSphere(0.05f, 10, 10);
        m_cursorSphere.Initialize(m_device, {sphere});
        m_cursorSphere.m_diffuseResView = m_cubeMapping.m_diffuseResView;
        m_cursorSphere.m_specularResView = m_cubeMapping.m_specularResView;
        Matrix modelMat = Matrix::CreateTranslation({0.0f, 0.0f, 0.0f});
        Matrix invTransposeRow = modelMat;
        invTransposeRow.Translation(Vector3(0.0f));
        invTransposeRow = invTransposeRow.Invert().Transpose();
        m_cursorSphere.m_basicVertexConstantData.modelWorld =
            modelMat.Transpose();
        m_cursorSphere.m_basicVertexConstantData.invTranspose =
            invTransposeRow.Transpose();
        m_cursorSphere.m_basicPixelConstantData.useTexture = false;
        m_cursorSphere.m_basicPixelConstantData.material.diffuse =
            Vector3(1.0f, 1.0f, 0.0f);
        m_cursorSphere.m_basicPixelConstantData.material.specular =
            Vector3(0.0f);
        m_cursorSphere.m_basicPixelConstantData.indexColor = Vector4(0.0f);
        m_cursorSphere.UpdateConstantBuffers(m_device, m_context);
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

    static BasicMeshGroup *pSelectedObject = nullptr;
    static BoundingSphere *pSelectedBoundingSphere = nullptr;

    if (m_pickColor[0] == 255) {
        pSelectedObject = &m_mainSphere;
        pSelectedBoundingSphere = &m_mainBoundingSphere;
    } else if (m_pickColor[1] == 255) {
        pSelectedObject = &m_box;
        pSelectedBoundingSphere = &m_boxBoundingSphere;
    }

    // mainSphere의 이동 계산용
    Vector3 dragTranslation(0.0f);

    // mainSphere의 회전 계산용
    Quaternion q =
        Quaternion::CreateFromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), 0.0f);

    // 마우스 클릭했을 때만 업데이트
    if (m_leftButton || m_rightButton) {
        // ViewFrustum에서 가까운 면 위의 커서 위치
        // ViewFrustum에서 먼 면 위의 커서 위치
        Vector3 cursorNdcNear = Vector3(m_cursorNdcX, m_cursorNdcY, 0.0f);
        Vector3 cursorNdcFar = Vector3(m_cursorNdcX, m_cursorNdcY, 1.0f);

        // NDC 커서 위치를 월드 좌표계로 역변환 해주는 행렬
        Matrix inverseProjView = (viewRow * projRow).Invert();

        // ViewFrustum 안에서 PickingRay의 방향 구하기
        Vector3 cursorWorldNear =
            Vector3::Transform(cursorNdcNear, inverseProjView);
        Vector3 cursorWorldFar =
            Vector3::Transform(cursorNdcFar, inverseProjView);
        Vector3 dir = cursorWorldFar - cursorWorldNear;
        dir.Normalize();

        // 광선을 만들고 충돌 감지
        SimpleMath::Ray curRay = SimpleMath::Ray(cursorWorldNear, dir);
        float dist = 0.0f;
        
        if (pSelectedBoundingSphere) {
            m_selected = curRay.Intersects(*pSelectedBoundingSphere, dist);
        }

        if (m_selected && pSelectedBoundingSphere) {
            Vector3 pickPoint = cursorWorldNear + dist * dir;

            // 충돌 지점에 작은 구 그리기
            m_cursorSphere.UpdateModelWorld(
                Matrix::CreateTranslation(pickPoint));
            m_cursorSphere.m_basicVertexConstantData.view = viewRow.Transpose();
            m_cursorSphere.m_basicVertexConstantData.projection =
                projRow.Transpose();
            m_cursorSphere.m_basicPixelConstantData.eyeWorld = eyeWorld;
            m_cursorSphere.UpdateConstantBuffers(m_device, m_context);

            // 좌클릭시 이동
            if (m_leftButton) {
                static float prevRatio = 0.0f;
                static Vector3 prevPos(0.0f);

                // mainSphere를 어떻게 이동시킬지 결정
                if (m_dragStartFlag) { // 드래그를 시작하는 경우
                    m_dragStartFlag = false;
                    prevRatio =
                        dist / (cursorWorldFar - cursorWorldNear).Length();
                    prevPos = pickPoint;
                } else {
                    Vector3 newPos =
                        cursorWorldNear +
                        prevRatio * (cursorWorldFar - cursorWorldNear);

                    dragTranslation = newPos - prevPos;
                    prevPos = newPos;
                }
            }

            // 우클릭시 회전
            if (m_rightButton) {
                static Vector3 prevVector(0.0f);

                if (m_dragStartFlag) {
                    m_dragStartFlag = false;
                    prevVector = pickPoint - pSelectedBoundingSphere->Center;
                    // prevVector.Normalize();
                } else {
                    Vector3 currentVector =
                        pickPoint - pSelectedBoundingSphere->Center;
                    // currentVector.Normalize();
                    // float theta = acos(prevVector.Dot(currentVector));
                    if ((currentVector - prevVector).Length() >
                        1e-3f) { // if (theta > 3.141592f / 180.0f)
                        q = SimpleMath::Quaternion::FromToRotation(
                            prevVector, currentVector);
                        // Vector3 axis = prevVector.Cross(currentVector);
                        // axis.Normalize();
                        // q = SimpleMath::Quaternion::CreateFromAxisAngle(axis,
                        // theta);

                        prevVector = currentVector;
                    }
                }
            }
        }
    }

    if (pSelectedObject) {
        // 물체의 원래 위치 저장
        Vector3 translation = pSelectedObject->m_modelWorldRow.Translation();

        // 회전을 위해 원점으로 이동 (이동하지 않을 시 의도하지 않은 대로 회전)
        pSelectedObject->m_modelWorldRow.Translation(Vector3(0.0f));

        pSelectedObject->UpdateModelWorld(
            pSelectedObject->m_modelWorldRow * Matrix::CreateFromQuaternion(q) *
            Matrix::CreateTranslation(translation) *
            Matrix::CreateTranslation(dragTranslation));

        // Bounding Sphere도 같이 이동
        pSelectedBoundingSphere->Center =
            pSelectedObject->m_modelWorldRow.Translation();
    }

    m_mainSphere.m_basicVertexConstantData.view = viewRow.Transpose();
    m_mainSphere.m_basicVertexConstantData.projection = projRow.Transpose();
    m_mainSphere.m_basicPixelConstantData.eyeWorld = eyeWorld;
    m_mainSphere.UpdateConstantBuffers(m_device, m_context);

    m_box.m_basicVertexConstantData.view = viewRow.Transpose();
    m_box.m_basicVertexConstantData.projection = projRow.Transpose();
    m_box.m_basicPixelConstantData.eyeWorld = eyeWorld;
    m_box.UpdateConstantBuffers(m_device, m_context);

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

    m_mainSphere.Render(m_context);
    m_box.Render(m_context);

    if ((m_leftButton || m_rightButton) && m_selected)
        m_cursorSphere.Render(m_context);

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

    this->UpdateMousePickColor();
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
            downFilter->m_pixelConstData.threshold = m_threshold;
        } else {
            downFilter->SetShaderResources(
                {m_filters.back()->m_shaderResourceView});
            downFilter->m_pixelConstData.threshold = 0.0f;
        }

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
