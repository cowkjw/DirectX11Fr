#include "Hierarchy.h"
#include <UICanvas.h>
#include "EditorManager.h"
#include "GameInstance.h"


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

	for (auto* root : CEditorManager::m_vecSceneObjects)
	{
		if (!root || root->GetParent()) continue;  // 최상위만
		DrawChildHierarchy(root);
	}

	ImGui::End();

}

void CHierarchy::DrawChildHierarchy(CGameObject* parent)
{
	if (!parent) return;

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
		| ImGuiTreeNodeFlags_SpanAvailWidth
		| (parent == CEditorManager::m_pSelectedObject ? ImGuiTreeNodeFlags_Selected : 0);

	// 2) TreeNode 생성
	_bool open = ImGui::TreeNodeEx((void*)parent, flags, "%s",
		WStringToString(parent->Get_Name()).c_str());

	if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonRight))
	{
		if (ImGui::MenuItem("Delete"))
		{
			RemoveAndDelete(parent);
			ImGui::EndPopup();
			if (open)
				ImGui::TreePop();
			return;   // 삭제했으면 이 노드는 더 그리지 않습니다.
		}
		ImGui::EndPopup();
	}

	// 3) 드래그 소스
	if (ImGui::BeginDragDropSource())
	{
		ImGui::SetDragDropPayload("DND_OBJ", &parent, sizeof(parent));
		ImGui::Text("%s", WStringToString(parent->Get_Name()).c_str());
		ImGui::EndDragDropSource();
	}

	// 4) 드롭 타겟
	if (ImGui::BeginDragDropTarget())
	{
		if (auto payload = ImGui::AcceptDragDropPayload("DND_OBJ"))
		{
			CGameObject* dragged = *(CGameObject**)payload->Data;
			if (dragged != parent && dragged->GetParent() != parent)
			{
				// 이전 부모에서 제거
				if (auto oldP = dragged->GetParent())
					oldP->RemoveChild(dragged);
				else
					EraseFromVector(dragged);

				// 캔버스면 AddChildUI, 아니면 일반 AddChild
				if (auto canvas = dynamic_cast<CUICanvas*>(parent))
					canvas->AddChildUI(static_cast<CUIObject*>(dragged));
				else
					parent->AddChild(dragged);
			}
		}
		ImGui::EndDragDropTarget();
	}

	// 5) 클릭 시 선택
	if (ImGui::IsItemClicked())
		CEditorManager::m_pSelectedObject = parent;

	// 6) 열렸으면 자식 모두 재귀 호출
	if (open)
	{
		for (auto* child : parent->GetChildren())
			if (child)
			    DrawChildHierarchy(child);
		ImGui::TreePop();
	}
}

void CHierarchy::EraseFromVector(CGameObject* pObj)
{
	auto it = find(CEditorManager::m_vecSceneObjects.begin(), CEditorManager::m_vecSceneObjects.end(), pObj);
	if (it != CEditorManager::m_vecSceneObjects.end())
		CEditorManager::m_vecSceneObjects.erase(it);
}

void CHierarchy::RemoveAndDelete(CGameObject* obj)
{
	if (!obj) return;

	if (auto parent = obj->GetParent())
		parent->RemoveChild(obj);
	else
		EraseFromVector(obj);

	// 2) 선택 해제
	if (CEditorManager::m_pSelectedObject == obj)
		CEditorManager::m_pSelectedObject = nullptr;

	if (auto canvas = dynamic_cast<CUICanvas*>(obj))
	{
		m_pGameInstance->RemoveCanvasUI(canvas->Get_Name());
	}
	else if (auto ui = dynamic_cast<CUIObject*>(obj))
	{
		auto canvas = dynamic_cast<CUICanvas*>(ui->GetParent());
		if (canvas)
			canvas->RemoveChildUI(ui->Get_Name());
	}
	else
		m_pGameInstance->Delete_GameObject(obj->GetCreateLevel(), obj);

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
