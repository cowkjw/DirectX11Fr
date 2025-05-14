#include "Hierarchy.h"
#include <UICanvas.h>
#include "EditorManager.h"


CHierarchy::CHierarchy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CPannel(pDevice, pContext)
{

}

HRESULT CHierarchy::Initialize()
{
	return S_OK;
}

void CHierarchy::Update(_float fTimeDelta)
{

}

HRESULT CHierarchy::Render()
{
    DrawHierarchy();
	return S_OK;
}

void CHierarchy::DrawHierarchy()
{
    ImGui::Begin("Hierarchy");

    for (auto* obj : CEditorManager::m_vecSceneObjects)
    {
        if (!obj) continue;
        // UI 객체인지, 그리고 캔버스인지 체크
        _bool isUI = dynamic_cast<CUIObject*>(obj) != nullptr;
        _bool isCanvas = dynamic_cast<CUICanvas*>(obj) != nullptr;

        // 트리 노드 플래그 설정
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
            | ImGuiTreeNodeFlags_SpanAvailWidth
            | (obj == CEditorManager::m_pSelectedObject ? ImGuiTreeNodeFlags_Selected : 0);

        // 트리 노드 생성
        _bool open = ImGui::TreeNodeEx((void*)obj, flags, "%s",
            WStringToString(obj->Get_Name()).c_str());

        // 드래그 소스로 등록
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::SetDragDropPayload("DND_OBJ", &obj, sizeof(CGameObject*));
            ImGui::Text("%s",
                WStringToString(obj->Get_Name()).c_str());
            ImGui::EndDragDropSource();
        }

        // 캔버스에만 드롭 가능
        if (isCanvas && ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_OBJ"))
            {
                CGameObject* dragged = *(CGameObject**)payload->Data;
                if (dragged->GetParent() != obj)
                {
                    // 이전 부모에서 제거
                    if (auto* oldP = dragged->GetParent())
                        oldP->RemoveChild(dragged);
                    else
                        EraseFromVector(dragged);

                    // 새 부모에 추가
                    static_cast<CUICanvas*>(obj)->AddChildUI(static_cast<CUIObject*>(dragged));
                }
            }
            ImGui::EndDragDropTarget();
        }

        // 클릭 시 선택
        if (ImGui::IsItemClicked())
            CEditorManager::m_pSelectedObject = obj;

        // 자식이 열려 있으면 재귀적으로 그리기
        if (open)
        {
            DrawChildHierarchy(obj);
            ImGui::TreePop();
        }
    }

    ImGui::End();
}

void CHierarchy::DrawChildHierarchy(CGameObject* parent)
{
    for (auto* child : parent->GetChildren())
    {
        bool isCanvas = dynamic_cast<CUICanvas*>(child) != nullptr;
        ImGuiTreeNodeFlags leafFlags = ImGuiTreeNodeFlags_Leaf
            | ImGuiTreeNodeFlags_NoTreePushOnOpen
            | ImGuiTreeNodeFlags_SpanAvailWidth
            | (child == CEditorManager::m_pSelectedObject ? ImGuiTreeNodeFlags_Selected : 0);

        ImGui::TreeNodeEx((void*)child, leafFlags, "%s",
            WStringToString(child->Get_Name()).c_str());

        // 드래그 소스 등록
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::SetDragDropPayload("DND_OBJ", &child, sizeof(CGameObject*));
            ImGui::Text("%s",
                WStringToString(child->Get_Name()).c_str());
            ImGui::EndDragDropSource();
        }

        // 캔버스인 경우 드롭 처리
        if (isCanvas && ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_OBJ"))
            {
                CGameObject* dragged = *(CGameObject**)payload->Data;
                if (dragged->GetParent() != child)
                {
                    if (auto* oldP = dragged->GetParent())
                        oldP->RemoveChild(dragged);
                    else
                        EraseFromVector(dragged);

                    child->AddChild(dragged);
                    dragged->SetParent(child);
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::IsItemClicked())
            CEditorManager::m_pSelectedObject = child;
    }
}

void CHierarchy::EraseFromVector(CGameObject* pObj)
{
    auto it = find(CEditorManager::m_vecSceneObjects.begin(), CEditorManager::m_vecSceneObjects.end(), pObj);
    if (it != CEditorManager::m_vecSceneObjects.end())
        CEditorManager::m_vecSceneObjects.erase(it);
}

CHierarchy* CHierarchy::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHierarchy* pInstance = new CHierarchy(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CHierarchy");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CHierarchy::Free()
{
	__super::Free();
}
