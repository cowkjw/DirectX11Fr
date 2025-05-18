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
	CEditorManager::m_pSelectedObject = nullptr;
    CEditorManager::m_vecSceneObjects.clear();
	m_vecPannels.push_back(CHierarchy::Create(m_pDevice, m_pContext));
	m_vecPannels.push_back(CInspectorPannel::Create(m_pDevice, m_pContext));
	m_vecPannels.push_back(CToolbar::Create(m_pDevice, m_pContext));

	_uint windowWidth = g_iWinSizeX;
	_uint windowHeight = g_iWinSizeY;
    if (FAILED(m_pGameInstance->CreateRenderTarget(
       windowWidth, windowWidth,
        &m_pGameTex, &m_pGameRTV, &m_pGameSRV, DXGI_FORMAT_B8G8R8A8_UNORM)))
        return E_FAIL;
    return S_OK;
}

void CEditorManager::Update(_float fTimeDelta)
{

    if (m_pSelectedObject)
    {
        if (m_pGameInstance->IsKeyPressed('W')) 
            GizmoOp = CGizmo::Operation::TRANSLATE;
        if (m_pGameInstance->IsKeyPressed('R')) 
            GizmoOp = CGizmo::Operation::ROTATE;
        if (m_pGameInstance->IsKeyPressed('E')) 
            GizmoOp = CGizmo::Operation::SCALE;
    }

    for (auto& pannel : m_vecPannels)
    {
        if (pannel)
            pannel->Update(fTimeDelta);
    }
}

HRESULT CEditorManager::Render()
{

    ID3D11RenderTargetView* pOldRTV = nullptr;
    ID3D11DepthStencilView* pDSV = nullptr;
    m_pContext->OMGetRenderTargets(1, &pOldRTV, &pDSV);

   /* m_pContext->OMSetRenderTargets(1, &m_pGameRTV, pDSV);
    _float4 vColor = _float4(0.f, 0.f, 1.f, 1.f);
    m_pContext->ClearRenderTargetView(m_pGameRTV, (_float*)&vColor);*/

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

    //m_pContext->OMSetRenderTargets(1, &pOldRTV, pDSV);
    //m_pContext->ClearRenderTargetView(pOldRTV, reinterpret_cast<const float*>(&vColor));


    _uint windowWidth = g_iWinSizeX;
    _uint windowHeight = g_iWinSizeY;
    ImGui::Begin("Scene View");
    ImGui::Image(
        (ImTextureID) m_pGameSRV,
        ImVec2((float)windowWidth, (float)windowHeight)
    );
    ImGui::End();

    // 6) Cleanup
    Safe_Release(pOldRTV);
    Safe_Release(pDSV);

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
	Safe_Release(m_pGameTex);
	Safe_Release(m_pGameRTV);
	Safe_Release(m_pGameSRV);
}
