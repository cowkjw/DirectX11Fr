#include "EditorManager.h"
#include "GameInstance.h"
#include "UIProgressBar.h"
#include "Transform.h"
#include "UICanvas.h"
#include "UIButton.h"
#include "UIImage.h"
#include "Gizmo.h"

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
  
	m_vecPannels.push_back(CHierarchy::Create(m_pDevice, m_pContext));
	m_vecPannels.push_back(CInspectorPannel::Create(m_pDevice, m_pContext));
	m_vecPannels.push_back(CToolbar::Create(m_pDevice, m_pContext));
    return S_OK;
}

void CEditorManager::Update(_float fTimeDelta)
{

    if (m_pGameInstance->IsKeyPressed(VK_F2) && m_pSelectedObject)
    {
        // F2 처리 로직
    }
    if (m_pSelectedObject)
    {
        if (m_pGameInstance->IsKeyPressed('W')) GizmoOp = CGizmo::Operation::TRANSLATE;
        if (m_pGameInstance->IsKeyPressed('R')) GizmoOp = CGizmo::Operation::ROTATE;
        if (m_pGameInstance->IsKeyPressed('E')) GizmoOp = CGizmo::Operation::SCALE;
    }

    for (auto& pannel : m_vecPannels)
    {
        if (pannel)
            pannel->Update(fTimeDelta);
    }
}

HRESULT CEditorManager::Render()
{
	for (auto& pannel : m_vecPannels)
	{
		if (pannel)
			pannel->Render();
	}

    if (m_pSelectedObject)
        CGizmo::Manipulate(
            m_pSelectedObject->GetTransform(),
            GizmoOp,
            m_bOrthoGizmo
        );

    return S_OK;
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
