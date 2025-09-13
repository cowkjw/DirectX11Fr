#include "InspectorPanel.h"
#include <Transform.h>
#include "GameObject.h"
#include "GameInstance.h"
#include <EditorManager.h>
#include "UIObject.h"
#include "BoxCollider.h"
#include "CapsuleCollider.h"
#include "SphereCollider.h"

CInspectorPanel::CInspectorPanel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPanel(pDevice, pContext)
{
}


HRESULT CInspectorPanel::Initialize()
{
	return S_OK;
}

void CInspectorPanel::Update(_float fTimeDelta)
{
}

HRESULT CInspectorPanel::Render()
{

	DrawInspector();
	return S_OK;
}

void CInspectorPanel::TreePop()
{
	ImGui::TreePop();
}

_bool CInspectorPanel::TreeNode(const char* label)
{
	if (ImGui::TreeNode(label))
	{
		ImGui::SetNextItemOpen(label, true);
		return true;
	}
	return false;
}

_bool CInspectorPanel::Checkbox(const char* label, _bool* v)
{
	if (ImGui::Checkbox(label, v))
	{
		return true;
	}
	return false;
}

_bool CInspectorPanel::DragFloat(const char* label, _float* v, _float speed)
{
	if (ImGui::DragFloat(label, v, speed))
	{
		return true;
	}
	return false;
}

_bool CInspectorPanel::DragFloat3(const char* label, _float* v, _float speed)
{
	if (ImGui::DragFloat3(label, v, speed))
	{
		return true;
	}
	return false;
}

_bool CInspectorPanel::InputFloat(const char* label, _float* v)
{
	if (ImGui::InputFloat(label, v))
	{
		return true;
	}
	return false;
}

_bool CInspectorPanel::InputFloat3(const char* label, _float* v)
{
	if (ImGui::InputFloat3(label, v))
	{
		return true;
	}
	return false;
}

_bool CInspectorPanel::InputInt(const char* label, int* v)
{
	if (ImGui::InputInt(label, v))
	{
		return true;
	}
	return false;
}

void CInspectorPanel::DrawInspector()
{
    ImGui::Begin("Inspector");

    if (!CEditorManager::m_pSelectedObject)
    {
        ImGui::Text("No object selected");
        ImGui::End();
        return;
    }

	_bool bIsActive = CEditorManager::m_pSelectedObject->IsActive();
	// 활성화 체크박스
	if (ImGui::Checkbox("Active", &bIsActive))
	{
		CEditorManager::m_pSelectedObject->SetActive(bIsActive);
	}

    // 이름 편집
    char nameBuf[128];

    strcpy_s(nameBuf, WStringToString(CEditorManager::m_pSelectedObject->Get_Name()).c_str());
    if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf), ImGuiInputTextFlags_EnterReturnsTrue))
        CEditorManager::m_pSelectedObject->Set_Name(StringToWString(nameBuf));

    // Transform 편집
    CTransform* trans = CEditorManager::m_pSelectedObject->GetTransform();
    _vector pos = trans->Get_State(STATE::POSITION);
     _float3 rot = trans->Get_EulerAngles();
    _float3 scl = trans->Get_Scaled();
    // 슬라이드 거리 설정
    if (ImGui::DragFloat3("Position", reinterpret_cast<_float*>(&pos), 0.1f, -10000.f, 10000.f))
        trans->Set_State(STATE::POSITION, pos);
      if (ImGui::DragFloat3("Rotation", reinterpret_cast<float*>(&rot), 0.1f, -10000.f, 10000.f))
          trans->Rotate_EulerAngles(rot);
    if (ImGui::DragFloat3("Scale", reinterpret_cast<_float*>(&scl), 0.1f, 0.001f, 1000.f))
        trans->Scaling(scl);

	if (auto pUI = dynamic_cast<CUIObject*>(CEditorManager::m_pSelectedObject))
	{
		// UI 전용 속성
		ImGui::Text("UI Properties:");
		_int sort = pUI->GetSortingOrder();
		ImGui::InputInt("SortOrder", &sort);
		pUI->SetSortingOrder(sort);
	}

	ImGui::Separator();
//	DrawColliderInspector();
	// 분리된 컴포넌트 UI
	DrawComponentList();
	ImGui::Separator();
	DrawAddComponentPopup();

    ImGui::End();
}

void CInspectorPanel::DrawComponentList()
{
	ImGui::Text("Components:");
	auto components = CEditorManager::m_pSelectedObject->GetComponents();
	for (const auto& Pair : components)
	{
		const auto& tag = WStringToString(Pair.first);
		if (ImGui::TreeNode(tag.c_str()))
		{
			// 필요 시 컴포넌트별 상세 UI 호출
			if (CCollider* collider = dynamic_cast<CCollider*>(Pair.second))
			{
				 collider->RenderInspector(*this);
			}
			else
			{
				ImGui::Text("Unknown Component");
				// CEditorManager::m_pSelectedObject->Get_Component(tag)->OnImGuiInspector();
			}
			ImGui::TreePop();
		}
	}
}

void CInspectorPanel::DrawAddComponentPopup()
{
	if (ImGui::Button("Add Component"))
		ImGui::OpenPopup("AddComponentPopup");

	if (!ImGui::BeginPopup("AddComponentPopup"))
		return;

	
	// 3) Collider 서브메뉴
	if (ImGui::BeginMenu("Collider"))
	{
		static _uint iColliderCount = 0;
		CComponent* pCollider = nullptr;
		if (ImGui::MenuItem("Box Collider"))
		{
			if (FAILED(CEditorManager::m_pSelectedObject->Add_Component(TEXT("Com_Collider") + to_wstring(iColliderCount++), CBoxCollider::Create(m_pDevice, m_pContext), reinterpret_cast<CComponent**>(&pCollider))))
			{
				--iColliderCount;
				return;
			}
			pCollider->Initialize(nullptr);
		}
		if (ImGui::MenuItem("Sphere Collider"))
		{

			if (FAILED(CEditorManager::m_pSelectedObject->Add_Component(TEXT("Com_Collider") + to_wstring(iColliderCount++), CSphereCollider::Create(m_pDevice, m_pContext,1.f), reinterpret_cast<CComponent**>(&pCollider))))
			{
				--iColliderCount;
				return;
			}
			pCollider->Initialize(nullptr);
		}
		if (ImGui::MenuItem("Capsule Collider"))
		{
			if (FAILED(CEditorManager::m_pSelectedObject->Add_Component(TEXT("Com_Collider") + to_wstring(iColliderCount++), CCapsuleCollider::Create(m_pDevice, m_pContext), reinterpret_cast<CComponent**>(&pCollider))))
			{
				--iColliderCount;
				return;
			}
			pCollider->Initialize(nullptr);
		}

		Safe_Release(pCollider);
		ImGui::EndMenu();
	}


	ImGui::EndPopup();
}

void CInspectorPanel::DrawColliderInspector()
{
	if (ImGui::Checkbox("Physics Debug Draw", &bPhysXDebug))
	{

		// 필요하다면 더 추가…
	}
	RenderPhysicsDebug();
}

void CInspectorPanel::RenderPhysicsDebug()
{
	
}

CInspectorPanel* CInspectorPanel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CInspectorPanel* pInstance = new CInspectorPanel(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CInspectorPanel");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CInspectorPanel::Free()
{
	__super::Free();
}
