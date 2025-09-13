#include "EditorManager.h"
#include "GameInstance.h"
#include "UIProgressBar.h"
#include "Transform.h"
#include "UICanvas.h"
#include "UIButton.h"
#include "UIImage.h"
#include "Mesh.h"
#include "Toolbar.h"
#include "Gizmo.h"
#include <Terrain.h>

CGameObject* CEditorManager::m_pSelectedObject = nullptr;
vector<CGameObject*> CEditorManager::m_vecSceneObjects;
_bool CEditorManager::m_bOrthoGizmo = { false };
using Gizmo = CGizmo;
Gizmo::Operation GizmoOp{ CGizmo::Operation::TRANSLATE };

CEditorManager::CEditorManager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pDevice);
}


HRESULT CEditorManager::Initialize()
{
	CEditorManager::m_pSelectedObject = nullptr;
	CEditorManager::m_vecSceneObjects.clear();
	m_vecPannels.push_back(CHierarchy::Create(m_pDevice, m_pContext));
	m_vecPannels.push_back(CInspectorPanel::Create(m_pDevice, m_pContext));
	m_vecPannels.push_back(CToolbar::Create(m_pDevice, m_pContext));

	_uint windowWidth = g_iWinSizeX;
	_uint windowHeight = g_iWinSizeY;

	return S_OK;
}

void CEditorManager::Update(_float fTimeDelta)
{
	ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse)
	{
		_bool rightClick = m_pGameInstance->IsMousePressed(1);
		if (m_pGameInstance->IsMousePressed(0)|| rightClick)
		{
			vector<pair<_float, CGameObject*>> hitList;
			XMMATRIX viewMatrix = XMLoadFloat4x4(m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::VIEW));

			for (auto* obj : m_vecSceneObjects)
			{
				if (!obj || !obj->IsActive())
					continue;

				auto pModel = dynamic_cast<CModel*>(obj->Get_Component(TEXT("Com_Model")));
				if (!pModel)
				{
					auto pTerrain = dynamic_cast<CTerrain*>(obj);
					if (!pTerrain)
						continue;

					auto pToolbar = dynamic_cast<CToolbar*>(m_vecPannels[2]);
					if (pToolbar)
					{
						if (rightClick)
						{
							pToolbar->DeletePoints(pTerrain->GetPickedPosition());
						}
						else
						{
							pToolbar->CreatePoints(pTerrain->GetPickedPosition());
						}

					}
					break;
				}

				_bool hit = false;
				_float hitDepth = 0.f;

				// 메시별 픽킹 검사
				for (auto* rawMesh : pModel->Get_Meshes())
				{
					auto* pBuffer = static_cast<CVIBuffer*>(rawMesh);
					if (!pBuffer)
						continue;

					_float3 localHit;
					_matrix invWorld = obj->GetTransform()->Get_WorldMatrix_Inverse();
					if (pBuffer->Compute_PickedPosition(invWorld, localHit))
					{
						XMVECTOR vLocal4 = XMVectorSet(localHit.x, localHit.y, localHit.z, 1.f);
						XMVECTOR vWorldPos = XMVector4Transform(vLocal4, XMLoadFloat4x4(&obj->GetTransform()->Get_WorldMatrix()));
						XMVECTOR vViewPos = XMVector3TransformCoord(vWorldPos, viewMatrix);
						hitDepth = XMVectorGetZ(vViewPos);
					
						auto pToolbar = dynamic_cast<CToolbar*>(m_vecPannels[2]);
						if (pToolbar)
						{
							_float3 vWorldHit;

							XMStoreFloat3(&vWorldHit, vWorldPos);
							if (rightClick)
							{
								pToolbar->DeletePoints(vWorldHit);
							}
							else
							{
								pToolbar->CreatePoints(vWorldHit);
							}
						
						}
						hit = true;
						break;  
					}
				}

				if (hit&&!rightClick)
					hitList.emplace_back(hitDepth, obj);
			}

			if (!hitList.empty())
			{
				// 깊이순으로 정렬 후 가장 앞에 있는 객체 선택
				sort(hitList.begin(), hitList.end(),
					[](auto& A, auto& B) { return A.first < B.first; });
				m_pSelectedObject = hitList.front().second;
			}
		}
	}

	if (m_pSelectedObject)
	{
		if (m_pGameInstance->IsKeyPressed('W'))
			GizmoOp = CGizmo::Operation::TRANSLATE;
		if (m_pGameInstance->IsKeyPressed('R'))
			GizmoOp = CGizmo::Operation::ROTATE;
		if (m_pGameInstance->IsKeyPressed('E'))
			GizmoOp = CGizmo::Operation::SCALE;
	}

	// 4) 패널 업데이트
	for (auto& pannel : m_vecPannels)
		if (pannel)
			pannel->Update(fTimeDelta);
}

HRESULT CEditorManager::Render()
{

	for (auto& pannel : m_vecPannels)
	{
		if (pannel)
			pannel->Render();
	}

	static _float snapTranslate[3] = { 1.f, 1.f, 1.f };    // 1-unit 단위로 이동 스냅
	static _float snapRotate[3] = { 15.f,15.f,15.f };     // 15° 단위 회전 스냅
	static _float snapScale[3] = { 0.1f,0.1f,0.1f };     // 0.1 단위 스케일 스냅

	if (m_pSelectedObject)
	{

		CGizmo::Manipulate(
			m_pSelectedObject->GetTransform(),
			GizmoOp,   // TRANSLATE, ROTATE, SCALE 중 선택
			m_bOrthoGizmo,                  // 원근(proj)모드
			snapTranslate,
			snapRotate,
			snapScale
		);
	}

//	TestNodes();
	return S_OK;
}
static int node_id_a = 1;
static int node_id_b = 2;
static int link_id = 100;
void CEditorManager::TestNodes()
{
	ImGui::Begin("node editor");
	ImNodes::BeginNodeEditor();

	// ┌─────────── Node A ───────────┐
	ImNodes::BeginNode(node_id_a);
	{
		ImNodes::BeginNodeTitleBar();
		ImGui::TextUnformatted("State: Idle");
		ImNodes::EndNodeTitleBar();

		// 입력 핀 (pin id = 10)
		ImNodes::BeginInputAttribute(10);
		ImGui::Text("in");
		ImNodes::EndInputAttribute();

		// 내용 영역
		ImGui::Text("Animation Clip: Idle.fbx");

		// 출력 핀 (pin id = 11)
		ImNodes::BeginOutputAttribute(11);
		ImGui::Indent(40);
		ImGui::Text("out");
		ImNodes::EndOutputAttribute();
	}
	ImNodes::EndNode();
	// └──────────────────────────────┘

	// ┌─────────── Node B ───────────┐
	ImNodes::BeginNode(node_id_b);
	{
		ImNodes::BeginNodeTitleBar();
		ImGui::TextUnformatted("State: Run");
		ImNodes::EndNodeTitleBar();

		ImNodes::BeginInputAttribute(20);
		ImGui::Text("in");
		ImNodes::EndInputAttribute();

		ImGui::Text("Animation Clip: Run.fbx");

		ImNodes::BeginOutputAttribute(21);
		ImGui::Indent(40);
		ImGui::Text("out");
		ImNodes::EndOutputAttribute();
	}
	ImNodes::EndNode();
	// └──────────────────────────────┘

	// 2) 노드 간 연결 (link_id=100, from pin 11 → to pin 20)
	ImNodes::Link(link_id, 11, 20);

	// 3) 노드 에디터 종료
	ImNodes::EndNodeEditor();

	ImGui::End();
}

CEditorManager* CEditorManager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEditorManager* pInstance = new CEditorManager(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CEditorManager");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CEditorManager::Free()
{
	__super::Free();
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);

	for (auto* obj : m_vecSceneObjects)
		Safe_Release(obj);

	for (auto& pannel : m_vecPannels)
		Safe_Release(pannel);
	m_vecPannels.clear();
}
