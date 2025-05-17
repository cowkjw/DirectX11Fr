#include "InspectorPannel.h"
#include <Transform.h>
#include "GameObject.h"
#include "GameInstance.h"
#include <EditorManager.h>
#include "BoxCollider.h"
#include "CapsuleCollider.h"
#include "SphereCollider.h"

CInspectorPannel::CInspectorPannel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPannel(pDevice, pContext)
{
}


HRESULT CInspectorPannel::Initialize()
{
	return S_OK;
}

void CInspectorPannel::Update(_float fTimeDelta)
{
}

HRESULT CInspectorPannel::Render()
{

	DrawInspector();
	return S_OK;
}

void CInspectorPannel::TreePop()
{
	ImGui::TreePop();
}

_bool CInspectorPannel::TreeNode(const char* label)
{
	if (ImGui::TreeNode(label))
	{
		ImGui::SetNextItemOpen(label, true);
		return true;
	}
	return false;
}

_bool CInspectorPannel::Checkbox(const char* label, _bool* v)
{
	if (ImGui::Checkbox(label, v))
	{
		return true;
	}
	return false;
}

_bool CInspectorPannel::DragFloat(const char* label, _float* v, _float speed)
{
	if (ImGui::DragFloat(label, v, speed))
	{
		return true;
	}
	return false;
}

_bool CInspectorPannel::DragFloat3(const char* label, _float* v, _float speed)
{
	if (ImGui::DragFloat3(label, v, speed))
	{
		return true;
	}
	return false;
}

_bool CInspectorPannel::InputFloat(const char* label, _float* v)
{
	if (ImGui::InputFloat(label, v))
	{
		return true;
	}
	return false;
}

_bool CInspectorPannel::InputFloat3(const char* label, _float* v)
{
	if (ImGui::InputFloat3(label, v))
	{
		return true;
	}
	return false;
}

void CInspectorPannel::DrawInspector()
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
    // XMFLOAT3 rot = trans->GetRotation();
    _float3 scl = trans->Get_Scaled();
    // 슬라이드 거리 설정
    if (ImGui::DragFloat3("Position", reinterpret_cast<_float*>(&pos), 0.1f, -10000.f, 10000.f))
        trans->Set_State(STATE::POSITION, pos);
    /*  if (ImGui::InputFloat3("Rotation", reinterpret_cast<float*>(&rot)))
          trans->SetRotation(rot);*/
    if (ImGui::DragFloat3("Scale", reinterpret_cast<_float*>(&scl), 0.1f, 0.1f, 1000.f))
        trans->Scaling(scl);

	ImGui::Separator();
	DrawColliderInspector();
	// 분리된 컴포넌트 UI
	DrawComponentList();
	ImGui::Separator();
	DrawAddComponentPopup();

    ImGui::End();
}

void CInspectorPannel::DrawComponentList()
{
	ImGui::Text("Components:");
	auto components = CEditorManager::m_pSelectedObject->GetComponents();
	for (const auto& Pair : components)
	{
		const auto& tag = WStringToString(Pair.first);
		if (ImGui::TreeNode(tag.c_str()))
		{
			// 필요 시 컴포넌트별 상세 UI 호출
			if (CPhysXCollider* collider = dynamic_cast<CPhysXCollider*>(Pair.second))
			{
				 collider->RenderInspector(*this);
			}
			else if (CRigidBody* rigidbody = dynamic_cast<CRigidBody*>(Pair.second))
			{
				 rigidbody->RenderInspector(*this);
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

void CInspectorPannel::DrawAddComponentPopup()
{
	if (ImGui::Button("Add Component"))
		ImGui::OpenPopup("AddComponentPopup");

	if (!ImGui::BeginPopup("AddComponentPopup"))
		return;

	
	// 3) Collider 서브메뉴
	if (ImGui::BeginMenu("Collider"))
	{
		PxRigidActor* pActor = nullptr;

		// (A) 이미 Rigidbody가 붙어 있다면
		if (auto pRbodyComp = CEditorManager::m_pSelectedObject->Get_Component(TEXT("Com_Rigidbody")))
		{
			// CRigidbodyComponent에 GetActor()를 만들어 놓았다 가정
			pActor = static_cast<CRigidBody*>(pRbodyComp)->GetRigidActor();
		}
		else
		{
			// (B) 없으면 Transform 정보로 Static Actor 생성
			CTransform* pTrans = CEditorManager::m_pSelectedObject->GetTransform();
			_vector pos = pTrans->Get_State(STATE::POSITION);

			// Convert _vector to XMFLOAT3
			XMFLOAT3 posFloat3;
			XMStoreFloat3(&posFloat3, pos);
			// Quaternion rot = pTrans->GetRotation(); // 회전도 필요하면
			PxTransform physxT
			{
				{ posFloat3.x, posFloat3.y, posFloat3.z },
				//{ rot.x, rot.y, rot.z, rot.w } 
			};

			// PhysX 매니저에 만든 헬퍼를 사용해서 Static Actor 생성
			pActor = m_pGameInstance->CreateRigidStatic(physxT);
			// (선택) 생성한 Static Actor를 Rigidbody 컴포넌트로도 추가해둘 수 있습니다.
		}

		CComponent* pCollider = nullptr;
		if (ImGui::MenuItem("Box Collider"))
		{
			if (FAILED(CEditorManager::m_pSelectedObject->Add_Component(
				ToIndex(LEVEL::STATIC),
				TEXT("Prototype_Component_BoxCollider"),
				TEXT("Com_Collider"),
				&pCollider,
				pActor 
			)))
				return;
		}
		if (ImGui::MenuItem("Sphere Collider"))
		{
			if (FAILED(CEditorManager::m_pSelectedObject->Add_Component(
				ToIndex(LEVEL::STATIC),
				TEXT("Prototype_Component_SphereCollider"),
				TEXT("Com_Collider"),
				&pCollider,
				pActor
			)))
				return;
		}
		if (ImGui::MenuItem("Capsule Collider"))
		{
			if (FAILED(CEditorManager::m_pSelectedObject->Add_Component(
				ToIndex(LEVEL::STATIC),
				TEXT("Prototype_Component_CapsuleCollider"),
				TEXT("Com_Collider"),
				&pCollider,
				pActor
			)))
				return;
		}
		Safe_Release(pCollider);
		ImGui::EndMenu();
	}
	if (ImGui::MenuItem("Rigidbody"))
	{
		CComponent* pCollider = nullptr;
		CEditorManager::m_pSelectedObject->Add_Component(
			ToIndex(LEVEL::STATIC),
			TEXT("Prototype_Component_RigidBody"),
			TEXT("Com_Rigidbody"),
			&pCollider
		);
	}

	ImGui::EndPopup();
}

void CInspectorPannel::DrawColliderInspector()
{
	if (ImGui::Checkbox("Physics Debug Draw", &bPhysXDebug))
	{
		// PhysX 시각화 파라미터도 on/off
		_float v = bPhysXDebug ? 1.0f : 0.0f;
		auto scene = m_pGameInstance->GetScene();
		scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, v);
		scene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, v);
		// 필요하다면 더 추가…
	}
	RenderPhysicsDebug();
}

void CInspectorPannel::RenderPhysicsDebug()
{
	//if (!bPhysXDebug) return;
	//auto scene = m_pGameInstance->GetScene();
	//// simulate()/fetchResults() 뒤에 호출되어야 합니다.
	//PxRenderBuffer const& rb = scene->getRenderBuffer();
	//ImDrawList* draw = ImGui::GetForegroundDrawList();

	//draw->CmdBuffer.clear();
	//draw->IdxBuffer.clear();
	//draw->VtxBuffer.clear();
	//// 카메라 뷰·투영 매트릭스, 뷰포트 정보
	//// (이미 있는 카메라 정보를 가져와서 설정해주세요)
	//D3D11_VIEWPORT vp; 
	//UINT n = 1; 
	//m_pContext->RSGetViewports(&n, &vp);
	//auto camView =  m_pGameInstance->Get_Transform_Matrix(TRANSFORM::VIEW);
	//auto camProj =  m_pGameInstance->Get_Transform_Matrix(TRANSFORM::PROJECTION);
	//XMMATRIX viewProj = XMMatrixMultiply(camView, camProj);

	//auto WorldToScreen = [&](const PxVec3& p) {
	//	XMVECTOR v = XMVector3Transform(
	//		XMVectorSet(p.x, p.y, p.z, 1),
	//		viewProj
	//	);
	//	v = XMVectorDivide(v, XMVectorSplatW(v));
	//	float x = vp.TopLeftX + vp.Width * (XMVectorGetX(v) * 0.5f + 0.5f);
	//	float y = vp.TopLeftY + vp.Height * (1.0f - (XMVectorGetY(v) * 0.5f + 0.5f));
	//	return ImVec2(x, y);
	//	};

	//// 라인 그리기
	//for (PxU32 i = 0; i < rb.getNbLines(); ++i)
	//{
	//	auto& L = rb.getLines()[i];
	//	ImVec2 p0 = WorldToScreen(L.pos0);
	//	ImVec2 p1 = WorldToScreen(L.pos1);
	//	draw->AddLine(p0, p1, IM_COL32(0, 255, 0, 128), 2.0f);
	//}

	//if (!bPhysXDebug)
	//	return;

	//// 1) PhysX simulate()/fetchResults() 는 이미 메인 루프에서 처리되었다고 가정
	//auto scene = m_pGameInstance->GetScene();
	//const PxRenderBuffer& rb = scene->getRenderBuffer();

	//// 2) 전체 화면 크기의 투명 창을 하나 띄워서
	//ImGuiIO& io = ImGui::GetIO();
	//ImGui::SetNextWindowBgAlpha(0.0f);
	//ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	//ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);
	//ImGuiWindowFlags flags =
	//	ImGuiWindowFlags_NoTitleBar
	//	| ImGuiWindowFlags_NoResize
	//	| ImGuiWindowFlags_NoMove
	//	| ImGuiWindowFlags_NoInputs
	//	| ImGuiWindowFlags_NoScrollbar
	//	| ImGuiWindowFlags_NoSavedSettings;
	//ImGui::Begin("##PhysXDebugOverlay", nullptr, flags);

	//// 3) 이 윈도우의 DrawList 를 가져와서
	//ImDrawList* draw = ImGui::GetWindowDrawList();

	//// 4) 뷰·투영 매트릭스 + 뷰포트 설정 (기존 코드 재사용)
	//D3D11_VIEWPORT vp; UINT n = 1;
	//m_pContext->RSGetViewports(&n, &vp);
	//auto camView = m_pGameInstance->Get_Transform_Matrix(TRANSFORM::VIEW);
	//auto camProj = m_pGameInstance->Get_Transform_Matrix(TRANSFORM::PROJECTION);
	//XMMATRIX viewProj = XMMatrixMultiply(camView, camProj);
	//auto WorldToScreen = [&](const PxVec3& p) {
	//	XMVECTOR v = XMVector3Transform(
	//		XMVectorSet(p.x, p.y, p.z, 1.0f), viewProj
	//	);
	//	v = XMVectorDivide(v, XMVectorSplatW(v));
	//	float x = vp.TopLeftX + vp.Width * (XMVectorGetX(v) * 0.5f + 0.5f);
	//	float y = vp.TopLeftY + vp.Height * (1.0f - (XMVectorGetY(v) * 0.5f + 0.5f));
	//	return ImVec2(x, y);
	//	};

	//// 5) PhysX가 제공한 모든 라인을 이 윈도우 DrawList 에만 다시 그립니다
	//for (PxU32 i = 0; i < rb.getNbLines(); ++i)
	//{
	//	auto& L = rb.getLines()[i];
	//	ImVec2 p0 = WorldToScreen(L.pos0);
	//	ImVec2 p1 = WorldToScreen(L.pos1);
	//	draw->AddLine(p0, p1, IM_COL32(0, 255, 0, 128), 2.0f);
	//}

	//ImGui::End();

	if (!bPhysXDebug) return;

	auto scene = m_pGameInstance->GetScene();
	const PxRenderBuffer& rb = scene->getRenderBuffer();

	// BackgroundDrawList 을 가져옵니다.
	ImDrawList* draw = ImGui::GetBackgroundDrawList();

	// (뷰·투영 매트릭스 + 뷰포트 셋업 코드는 생략)
	D3D11_VIEWPORT vp; UINT n = 1;
	m_pContext->RSGetViewports(&n, &vp);
	XMMATRIX viewProj = XMMatrixMultiply(
		m_pGameInstance->Get_Transform_Matrix(TRANSFORM::VIEW),
		m_pGameInstance->Get_Transform_Matrix(TRANSFORM::PROJECTION)
	);

	auto WorldToScreen = [&](const PxVec3& p) {
		XMVECTOR v = XMVector3Transform(
			XMVectorSet(p.x, p.y, p.z, 1), viewProj
		);
		v = XMVectorDivide(v, XMVectorSplatW(v));
		float x = vp.TopLeftX + vp.Width * (XMVectorGetX(v) * 0.5f + 0.5f);
		float y = vp.TopLeftY + vp.Height * (1.0f - (XMVectorGetY(v) * 0.5f + 0.5f));
		return ImVec2(x, y);
		};

	// (선택) 클리핑 Rect 를 걸어서 게임 뷰포트 안에서만 그리기
	ImVec2 clip0(vp.TopLeftX, vp.TopLeftY);
	ImVec2 clip1(vp.TopLeftX + vp.Width, vp.TopLeftY + vp.Height);
	draw->PushClipRect(clip0, clip1, true);

	// PhysX 선분 그리기
	for (PxU32 i = 0; i < rb.getNbLines(); ++i)
	{
		auto& L = rb.getLines()[i];
		ImVec2 p0 = WorldToScreen(L.pos0);
		ImVec2 p1 = WorldToScreen(L.pos1);
		draw->AddLine(p0, p1, IM_COL32(0, 255, 0, 128), 2.0f);
	}

	draw->PopClipRect();
}

CInspectorPannel* CInspectorPannel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CInspectorPannel* pInstance = new CInspectorPannel(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CInspectorPannel");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CInspectorPannel::Free()
{
	__super::Free();
}
