#include "UIController.h"
#include "GameInstance.h"
#include "UICanvas.h"
#include "UIButton.h"
#include "UIImage.h"
#include "Gizmo.h"

using Gizmo = CGizmo;

CUIController::CUIController(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance()}
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pDevice);
}
HRESULT CUIController::Initialize()
{
    return S_OK;
}

void CUIController::Update(_float fTimeDelta)
{

	if (m_pGameInstance->IsKeyPressed('T'))
	{
		Gizmo::Manipulate(nullptr, Gizmo::Operation::TRANSLATE);
	}

	if (m_pGameInstance->IsKeyPressed('S'))
	{
		Gizmo::Manipulate(nullptr, Gizmo::Operation::SCALE);
	}

	if (m_pGameInstance->IsKeyPressed('R'))
	{
		Gizmo::Manipulate(nullptr, Gizmo::Operation::ROTATE);
	}
}

HRESULT CUIController::Render()
{
	DrawHierarchy();
	ImGui::Begin("Inspector");
	DrawInspector();
	ImGui::End();

	DrawToolbar();

	return S_OK;
}

void CUIController::DrawHierarchy()
{
	ImGui::Begin("Hierarchy");
	////for (auto* obj : m_vecSceneObjects)
	////{
	////	string label = WStringToString(obj->Get_Name());
	////	if (ImGui::Selectable(label.c_str(), obj == m_pSelectedObject))
	////	{
	////		m_pSelectedObject = obj;
	////	}
	////}

	if (ImGui::BeginPopupContextWindow("HierarchyContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
	{
		if (ImGui::MenuItem("Create Canvas"))
		{
			_uint idx = (_uint)m_vCanvases.size();
			m_vCanvases.push_back(CUICanvas::Create(m_pDevice, m_pContext));
			m_vCanvases.back()->Set_Name(L"Canvas");
		}
		ImGui::EndPopup();
	}

	// 2) 기존 스탠드얼론 버튼도 가능
	if (ImGui::Button("Create Canvas"))
	{
		int idx = (_uint)m_vCanvases.size();
		m_vCanvases.push_back(CUICanvas::Create(m_pDevice,m_pContext));
		m_vCanvases.back()->Set_Name(L"Canvas");
		m_pGameInstance->AddCanvasUI(m_vCanvases.back());

	}
	ImGui::Separator();
	for (int i = 0; i < (int)m_vCanvases.size(); ++i)
	{
		auto* cv = m_vCanvases[i];
		std::string name = WStringToString(cv->Get_Name());

		// 1) ID용 포인터와, 화면 표시용 텍스트 분리
		//    이 오버로드: TreeNodeEx(void* ptr_id, flags, fmt, ...) 
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
			| ImGuiTreeNodeFlags_SpanAvailWidth
			| ((cv == m_pSelectedCanvas)
				? ImGuiTreeNodeFlags_Selected
				: 0);
		bool open = ImGui::TreeNodeEx((void*)cv, flags, "%s", name.c_str());

		// 2) 클릭 처리
		if (ImGui::IsItemClicked())
		{
			m_pSelectedCanvas = cv;
			m_pSelectedObject = nullptr;
		}

		// 3) 열렸을 때만 자식 렌더링
		if (open)
		{
			for (_int j = 0; j < (_int)cv->GetChildren().size(); ++j)
			{
				auto* child = cv->GetChildren()[j];
				std::string cname = WStringToString(child->Get_Name());
				// Leaf 노드로 표시 (접기 화살표 없음)
				ImGuiTreeNodeFlags leafFlags = ImGuiTreeNodeFlags_Leaf
					| ImGuiTreeNodeFlags_NoTreePushOnOpen
					| ImGuiTreeNodeFlags_SpanAvailWidth
					| ((child == m_pSelectedObject)
						? ImGuiTreeNodeFlags_Selected
						: 0);
				ImGui::TreeNodeEx((void*)(intptr_t)((i << 16) | j),
					leafFlags,
					"%s",
					cname.c_str());
				if (ImGui::IsItemClicked())
					m_pSelectedObject = child;
			}
			ImGui::TreePop();
		}
	}
	ImGui::End();

	//ImGui::Begin("Hierarchy");

	//// Create Canvas 버튼
	//if (ImGui::Button("Create Canvas"))
	//{
	//	int idx = (int) m_vCanvases.size();
	//	m_canvases.push_back(new UICanvas(L"Canvas" + std::to_wstring(idx)));
	//}
	//ImGui::Separator();

	//// 트리 뷰
	//for (int i = 0; i < (int)m_vCanvases.size(); ++i)
	//{
	//	auto* cv = m_vCanvases[i];
	//	// 고유 ID 확보
	//	std::string treeLabel = "Canvas##" + std::to_string(i);

	//	// 1) TreeNodeEx 으로 접고 펼칠 수 있게
	//	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow
	//		| ImGuiTreeNodeFlags_SpanAvailWidth
	//		| ((cv == m_pSelectedCanvas) ? ImGuiTreeNodeFlags_Selected : 0);
	//	bool open = ImGui::TreeNodeEx(treeLabel.c_str(), nodeFlags, "Canvas");

	//	// 2) 클릭 시 선택 처리
	//	if (ImGui::IsItemClicked())
	//	{
	//		m_pSelectedCanvas = cv;
	//		m_pSelectedObject = nullptr;
	//	}

	//	// 3) 열려 있으면 자식 요소도 트리로 그리기
	//	if (open)
	//	{
	//		for (int j = 0; j < (int)cv->GetChildren().size(); ++j)
	//		{
	//			auto* el = cv->GetChildren()[j];
	//			std::string childLabel = std::string(el->GetName().begin(), el->GetName().end())
	//				+ "##" + std::to_string(i) + "_" + std::to_string(j);

	//			// 자식은 Leaf 노드로 표시
	//			ImGuiTreeNodeFlags leafFlags = ImGuiTreeNodeFlags_Leaf
	//				| ImGuiTreeNodeFlags_NoTreePushOnOpen
	//				| ImGuiTreeNodeFlags_SpanAvailWidth
	//				| ((el == m_pSelectedObject) ? ImGuiTreeNodeFlags_Selected : 0);
	//			ImGui::TreeNodeEx(childLabel.c_str(), leafFlags, "%s", childLabel.c_str());

	//			if (ImGui::IsItemClicked())
	//				m_pSelectedObject = el;
	//		}
	//		ImGui::TreePop();
	//	}
	//}

	//ImGui::End();
}

void CUIController::DrawInspector()
{
}

void CUIController::DrawToolbar()
{
	ImGui::Begin("Toolbar");

	ImGui::End();
}

CUIController* CUIController::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUIController* pInstance = new CUIController(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CUIController");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUIController::Free()
{
	__super::Free();
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}
