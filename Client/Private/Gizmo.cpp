#include "Gizmo.h"
#include "Transform.h"
#include "GameInstance.h"

void CGizmo::Manipulate(CTransform* pTransform, Operation op, _bool isOrtho, const _float* snap)
{
    if (!pTransform) return;

    ImGuizmo::BeginFrame();
    auto& io = ImGui::GetIO();

    // 투영 모드 설정
    ImGuizmo::SetOrthographic(isOrtho);
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    _float matV[16], matP[16], matW[16];

    // 1) 뷰 매트릭스
    XMMATRIX xmV = isOrtho
        ? XMMatrixIdentity()   // UI용, 혹은 별도 직교 뷰
        : CGameInstance::Get_Instance()->Get_Transform_Matrix(TRANSFORM::VIEW);
    XMStoreFloat4x4((XMFLOAT4X4*)matV, xmV);


    // 2) 프로젝션 매트릭스
    if (isOrtho)
    {
        // 화면 픽셀 크기에 딱 맞춘 직교 매트릭스
        _float w = io.DisplaySize.x, h = io.DisplaySize.y;
        XMMATRIX xmP = XMMatrixOrthographicLH(w, h, 0.1f, 1.f);
        XMStoreFloat4x4((XMFLOAT4X4*)matP, xmP);
    }
    else
    {
        XMMATRIX xmP = CGameInstance::Get_Instance()->Get_Transform_Matrix(TRANSFORM::PROJECTION);
        XMStoreFloat4x4((XMFLOAT4X4*)matP, xmP);
    }

    XMFLOAT4X4 worldF4x4 = pTransform->Get_WorldMatrix();
    XMMATRIX   xmW = XMLoadFloat4x4(&worldF4x4);

    //  - XMStoreFloat4x4 로 matW 에 써 줍니다.
    XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(matW), xmW);

    // 4) 조작 모드
    ImGuizmo::OPERATION gizOp =
        op == Operation::TRANSLATE ? ImGuizmo::TRANSLATE :
        op == Operation::ROTATE ? ImGuizmo::ROTATE :
        ImGuizmo::SCALE;

    ImGuizmo::Manipulate(matV, matP, gizOp, ImGuizmo::WORLD, matW, nullptr, snap);

    // 5) 드래그 중이면 트랜스폼에 반영
    if (ImGuizmo::IsUsing())
    {
        XMFLOAT4X4 newW;
        memcpy(&newW, matW, sizeof(newW));
        pTransform->Set_WorldMatrix(newW);
    }
}

void CGizmo::Manipulate(CTransform* pTransform, Operation op, _bool isOrtho, const _float snapT[3], const _float snapR[3], const _float snapS[3])
{
    if (!pTransform) return;

    ImGuizmo::BeginFrame();
    auto& io = ImGui::GetIO();
    ImGuizmo::SetOrthographic(isOrtho);
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    // 뷰·프로젝션·월드 매트릭스 float[16] 준비
    _float matV[16], matP[16], matW[16];
    XMStoreFloat4x4((XMFLOAT4X4*)matV,
        isOrtho
        ? XMMatrixIdentity()
        : CGameInstance::Get_Instance()->Get_Transform_Matrix(TRANSFORM::VIEW));
    XMStoreFloat4x4((XMFLOAT4X4*)matP,
        isOrtho
        ? XMMatrixOrthographicLH(io.DisplaySize.x, io.DisplaySize.y, 0.1f, 1.f)
        : CGameInstance::Get_Instance()->Get_Transform_Matrix(TRANSFORM::PROJECTION));

    XMFLOAT4X4 worldF4x4 = pTransform->Get_WorldMatrix();
    XMMATRIX   xmW = XMLoadFloat4x4(&worldF4x4);
    //  - XMStoreFloat4x4 로 matW 에 써 줍니다.
    XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(matW), xmW);

    // 조작 모드 결정
    ImGuizmo::OPERATION gizOp =
        op == Operation::TRANSLATE ? ImGuizmo::TRANSLATE :
        op == Operation::ROTATE ? ImGuizmo::ROTATE :
        ImGuizmo::SCALE;

    // Shift 누르고 있을 때만 스냅 적용
    const _float* snapPtr = nullptr;
    if (ImGui::IsKeyDown(ImGuiMod_Shift))
    {
        switch (op)
        {
        case Operation::TRANSLATE: snapPtr = snapT; break;
        case Operation::ROTATE:    snapPtr = snapR; break;
        case Operation::SCALE:     snapPtr = snapS; break;
        }
    }

    // Manipulate에 스냅 배열 전달
    ImGuizmo::Manipulate(matV, matP, gizOp, ImGuizmo::WORLD, matW, nullptr, snapPtr);

    // 드래그 중이면 WorldMatrix 업데이트
    if (ImGuizmo::IsUsing())
    {
        XMFLOAT4X4 newW;
        memcpy(&newW, matW, sizeof(newW));
        pTransform->Set_WorldMatrix(newW);

        if (op == Operation::ROTATE)
        {
			pTransform->UpdateEulerAngles();
        }
    }
}
