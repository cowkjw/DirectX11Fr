#include "Gizmo.h"
#include "Transform.h"
#include "GameInstance.h"

void CGizmo::Manipulate(CTransform* pTransform, Operation op, const _float* snap)
{
    if (!pTransform) return;

    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    auto& io = ImGui::GetIO();
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    static auto pGameIns = CGameInstance::Get_Instance();

    _float matW[16], matV[16], matP[16];


    XMMATRIX xmView = pGameIns->Get_Transform_Matrix(TRANSFORM::VIEW);
    XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(matV), xmView);

    // proj
    XMMATRIX xmProj = pGameIns->Get_Transform_Matrix(TRANSFORM::PROJECTION);
    XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(matP), xmProj);
   // memcpy(matW, &pTransform->GetMatrix(), sizeof(matW));


    ImGuizmo::OPERATION gizOp =
        (op == Operation::TRANSLATE) ? ImGuizmo::TRANSLATE :
        (op == Operation::ROTATE) ? ImGuizmo::ROTATE :
        ImGuizmo::SCALE;

    ImGuizmo::Manipulate(
        matV, matP,
        gizOp,
        ImGuizmo::LOCAL,
        matW,
        nullptr,
        snap
    );

    //if (ImGuizmo::IsUsing()) {
    //    _float4x4 newW;
    //    memcpy(&newW, matW, sizeof(newW));
    //    pTransform->SetMatrix(newW);
    //}
}
