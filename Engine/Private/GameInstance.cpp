#include "GameInstance.h"

#include "Picking.h"
#include "Renderer.h"
#include "Level_Manager.h"
#include "Light_Manager.h"
#include "Timer_Manager.h"
#include "Graphic_Device.h"
#include "FrustumCull.h"
#include "UIManager.h"
#include "GameObject.h"
#include "Object_Manager.h"
#include "Input_Device.h"
#include "TransformPipeline.h"
#include "ResourceMag.h"
#include "Prototype_Manager.h"
#include "FontMag.h"
#include "CollisionMag.h"

IMPLEMENT_SINGLETON(CGameInstance);

CGameInstance::CGameInstance()
{
}

HRESULT CGameInstance::Initialize_Engine(const ENGINE_DESC& EngineDesc, _Out_ ID3D11Device** ppDeviceOut, ID3D11DeviceContext** ppContextOut)
{
	m_pGraphic_Device = CGraphic_Device::Create(EngineDesc.hWnd, EngineDesc.isWindowed, EngineDesc.iWinSizeX, EngineDesc.iWinSizeY, ppDeviceOut, ppContextOut);
	if (nullptr == m_pGraphic_Device)
		return E_FAIL;

	m_pTimer_Manager = CTimer_Manager::Create();
	if (nullptr == m_pTimer_Manager)
		return E_FAIL;

	m_pLevel_Manager = CLevel_Manager::Create();
	if (nullptr == m_pLevel_Manager)
		return E_FAIL;

	m_pPrototype_Manager = CPrototype_Manager::Create(EngineDesc.iNumLevels);
	if (nullptr == m_pPrototype_Manager)
		return E_FAIL;

	m_pObject_Manager = CObject_Manager::Create(EngineDesc.iNumLevels);
	if (nullptr == m_pObject_Manager)
		return E_FAIL;

	m_pRenderer = CRenderer::Create(*ppDeviceOut, *ppContextOut);
	if (nullptr == m_pRenderer)
		return E_FAIL;

	m_pPicking = CPicking::Create(*ppDeviceOut, *ppContextOut, EngineDesc.hWnd, EngineDesc.iWinSizeX, EngineDesc.iWinSizeY);
	if (nullptr == m_pPicking)
		return E_FAIL;


	m_pFrustumCull = CFrustumCull::Create();
	if (nullptr == m_pFrustumCull)
		return E_FAIL;

	m_pInput_Device = CInput_Device::Create(EngineDesc.hWnd);
	if (nullptr == m_pInput_Device)
		return E_FAIL;

	m_pTransformPipeline = CTransformPipeline::Create();
	if (nullptr == m_pTransformPipeline)
		return E_FAIL;

	m_pUIManager = CUIManager::Create(*ppDeviceOut, *ppContextOut);
	if (nullptr == m_pUIManager)
		return E_FAIL;

	m_pResourceMag = CResourceMag::Create(*ppDeviceOut, *ppContextOut);
	if (nullptr == m_pResourceMag)
		return E_FAIL;

	m_pCollisionMag = CCollisionMag::Create();
	if (nullptr == m_pCollisionMag)
		return E_FAIL;

	m_pLight_Manager = CLight_Manager::Create();
	if (nullptr == m_pLight_Manager)
		return E_FAIL;

	m_pFont_Manager = CFontMag::Create(*ppDeviceOut, *ppContextOut);
	if (nullptr == m_pFont_Manager)
		return E_FAIL;



	
	return S_OK;
}

void CGameInstance::Fixed_Update(_float fTimeDelta)
{
	m_pCollisionMag->Update(fTimeDelta);
}

void CGameInstance::Update_Engine(_float fTimeDelta)
{
	m_pObject_Manager->Priority_Update(fTimeDelta);
	m_pTransformPipeline->Update();

	m_pPicking->Update(*m_pTransformPipeline->Get_Transform_Float4x4(TRANSFORM::VIEW), *m_pTransformPipeline->Get_Transform_Float4x4(TRANSFORM::PROJECTION));

	m_pFrustumCull->Update(*m_pTransformPipeline->Get_Transform_Float4x4(TRANSFORM::VIEW), *m_pTransformPipeline->Get_Transform_Float4x4(TRANSFORM::PROJECTION));
	m_pObject_Manager->Update(fTimeDelta);	
	m_pObject_Manager->Late_Update(fTimeDelta);


	m_pLevel_Manager->Update(fTimeDelta);
	m_pUIManager->Update_UI(fTimeDelta);
}

HRESULT CGameInstance::Begin_Draw()
{
	if (nullptr == m_pGraphic_Device)
		return E_FAIL;

	m_pGraphic_Device->Clear_BackBuffer_View(_float4(0.f, 0.f, 1.f, 1.f));
	m_pGraphic_Device->Clear_DepthStencil_View();

	return S_OK;
}

HRESULT CGameInstance::Draw()
{
	if (nullptr == m_pGraphic_Device)
		return E_FAIL;

	m_pRenderer->Draw();
	m_pLevel_Manager->Render();
	m_pCollisionMag->DebugDraw();

	return S_OK;
}

HRESULT CGameInstance::End_Draw()
{
	m_pGraphic_Device->Present();
	m_pLevel_Manager->Change_Level();
	return S_OK;
}

void CGameInstance::Clear(_uint iLevelIndex)
{
	/* 특정 레벨의 자원을 삭제한다. */
	
	/* 특정 레벨의 객체을 삭제한다. */
	m_pObject_Manager->Clear(iLevelIndex);

	/* 특정 레벨의 원형객을 삭제한다. */
	m_pPrototype_Manager->Clear(iLevelIndex);

	m_pResourceMag->Clear();
}

void CGameInstance::ClearObejcts(_uint iLevelIndex)
{/* 특정 레벨의 객체을 삭제한다. */
	m_pObject_Manager->Clear(iLevelIndex);
}


_float CGameInstance::Compute_Random_Normal()
{
	return rand() / static_cast<_float>(RAND_MAX);	
}

_float CGameInstance::Compute_Random(_float fMin, _float fMax)
{
	return fMin + (fMax - fMin) * Compute_Random_Normal();	
}

#pragma region LEVEL_MANAGER

HRESULT CGameInstance::Change_Level(_uint iLevelIndex, CLevel* pNewLevel)
{
 	return m_pLevel_Manager->Prepanding_Change_Level(iLevelIndex, pNewLevel);
}
#pragma endregion

#pragma region PROTOTYPE_MANAGER

HRESULT CGameInstance::Add_Prototype(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, CBase* pPrototype)
{
	return m_pPrototype_Manager->Add_Prototype(iPrototypeLevelIndex, strPrototypeTag, pPrototype);
}

CBase* CGameInstance::Clone_Prototype(PROTOTYPE ePrototypeType, _uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, void* pArg)
{
	return m_pPrototype_Manager->Clone_Prototype(ePrototypeType, iPrototypeLevelIndex, strPrototypeTag, pArg);
}
map<const _wstring, class CBase*>* CGameInstance::Get_Prototypes(_uint iLevelIndex)
{
	return m_pPrototype_Manager->Get_Prototypes(iLevelIndex);
}
#pragma endregion

#pragma region OBJECT_MANAGER
CGameObject* CGameInstance::Add_GameObject(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLevelIndex, const _wstring& strLayerTag, void* pArg )
{
	return m_pObject_Manager->Add_GameObject(iPrototypeLevelIndex, strPrototypeTag, iLevelIndex, strLayerTag, pArg);
}

CComponent* CGameInstance::Get_Component(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strComponentTag, _uint iIndex)
{
	return m_pObject_Manager->Get_Component(iLevelIndex, strLayerTag, strComponentTag, iIndex);
	
}

HRESULT CGameInstance::Delete_GameObject(_uint iLevelIndex, CGameObject* pGameObject)
{
	if (nullptr == pGameObject)
		return E_FAIL;
	m_pObject_Manager->Delete_GameObject(iLevelIndex, pGameObject);
	return S_OK;
}

HRESULT CGameInstance::Delete_GameObjectByName(_uint iLevelIndex, const _wstring& strName)
{
	m_pObject_Manager->Delete_GameObjectByName(iLevelIndex, strName);
	return S_OK;
}

CGameObject* CGameInstance::Find_GameObjectByName(_uint iLevelIndex, const _wstring& strName)
{
	return m_pObject_Manager->Find_GameObjectByName(iLevelIndex, strName);
}

#pragma endregion

#pragma region RENDERER

HRESULT CGameInstance::Add_RenderGroup(RENDERGROUP eRenderGroup, CGameObject* pRenderObject)
{
	return m_pRenderer->Add_RenderGroup(eRenderGroup, pRenderObject);
}

#pragma endregion

#pragma region TIMER_MANAGER

_float CGameInstance::Get_TimeDelta(const _wstring& strTimerTag)
{
	return m_pTimer_Manager->Get_TimeDelta(strTimerTag);
}

HRESULT CGameInstance::Add_Timer(const _wstring& strTimerTag)
{
	return m_pTimer_Manager->Add_Timer(strTimerTag);
}

void CGameInstance::Update_Timer(const _wstring& strTimerTag)
{
	return m_pTimer_Manager->Update(strTimerTag);
}

void CGameInstance::Transform_Picking_ToLocalSpace(const _matrix& WorldMatrixInverse)
{
	m_pPicking->Transform_ToLocalSpace(WorldMatrixInverse);
}

_bool CGameInstance::Picking_InWorld(_float3& vPickedPos, const _float3& vPointA, const _float3& vPointB, const _float3& vPointC)
{
	return m_pPicking->Picking_InWorld(vPickedPos, vPointA, vPointB, vPointC);
}

_bool CGameInstance::Picking_InLocal(_float3& vPickedPos, const _float3& vPointA, const _float3& vPointB, const _float3& vPointC)
{
	return m_pPicking->Picking_InLocal(vPickedPos, vPointA, vPointB, vPointC);
}

#pragma endregion

//#pragma region PICKING
//void CGameInstance::Transform_Picking_ToLocalSpace(const _float4x4& WorldMatrixInverse)
//{
//	m_pPicking->Transform_ToLocalSpace(WorldMatrixInverse);
//}
//_bool CGameInstance::Picking_InWorld(_float3& vPickedPos, const _float3& vPointA, const _float3& vPointB, const _float3& vPointC)
//{
//	return m_pPicking->Picking_InWorld(vPickedPos, vPointA, vPointB, vPointC);
//}
//_bool CGameInstance::Picking_InLocal(_float3& vPickedPos, const _float3& vPointA, const _float3& vPointB, const _float3& vPointC)
//{
//	return m_pPicking->Picking_InLocal(vPickedPos, vPointA, vPointB, vPointC);
//}
//
//#pragma endregion


#pragma region FRUSTUM
_bool CGameInstance::IsPointInFrustum(const _float3& point)
{
	return m_pFrustumCull->IsPointInFrustum(point);
}

_bool CGameInstance::IsSphereInFrustum(const _float3& point, _float fRadius)
{
	return m_pFrustumCull->IsSphereInFrustum(point, fRadius);
}

_bool CGameInstance::IsAABBInFrustum(const _float3& point, const _float3& scale)
{
	return m_pFrustumCull->IsAABBInFrustum(point, scale);
}
#pragma endregion

#pragma region INPUT_DEVICE
_bool CGameInstance::IsKeyDown(_ushort vkey) const
{
	return m_pInput_Device->IsKeyDown(vkey);
}
_bool CGameInstance::IsKeyPressed(_ushort vkey) const
{
	return m_pInput_Device->IsKeyPressed(vkey);
}
_bool CGameInstance::IsMouseDown(_ulonglong btn) const
{
	return m_pInput_Device->IsMouseDown(btn);
}
_bool CGameInstance::IsMousePressed(_ulonglong btn) const
{
	return m_pInput_Device->IsMousePressed(btn);
}
POINT CGameInstance::GetMousePos() const
{
	return m_pInput_Device->GetMousePos();
}
LONG CGameInstance::GetMouseWheel() const
{
	return m_pInput_Device->GetMouseWheel();
}
POINT CGameInstance::GetMouseDelta() const
{
	return m_pInput_Device->GetMouseDelta();
}
void CGameInstance::ProcessRawInput(LPARAM lParam)
{
	m_pInput_Device->ProcessRawInput(lParam);
}
void CGameInstance::Update_Input()
{
	m_pInput_Device->Update();
}
#pragma endregion


#pragma region TRANSFORM_PIPELINE
void CGameInstance::Set_Transform(TRANSFORM eState, _fmatrix TransformMatrix)
{
	m_pTransformPipeline->Set_Transform(eState, TransformMatrix);
}
const _float4x4* CGameInstance::Get_Transform_Float4x4(TRANSFORM eState) const
{
	return m_pTransformPipeline->Get_Transform_Float4x4(eState);
}
const _matrix CGameInstance::Get_Transform_Matrix(TRANSFORM eState) const
{
	return m_pTransformPipeline->Get_Transform_Matrix(eState);
}
const _float4* CGameInstance::Get_CamPosition() const
{
	return m_pTransformPipeline->Get_CamPosition();
}
#pragma endregion

#pragma region UI
void CGameInstance::AddCanvasUI(CUICanvas* pCanvas)
{
	if (nullptr == m_pUIManager)
		return;
	m_pUIManager->AddCanvasUI(pCanvas);
}
void CGameInstance::RemoveCanvasUI(const _wstring& canvasName)
{
	if (nullptr == m_pUIManager)
		return;
	m_pUIManager->RemoveCanvasUI(canvasName);
}
void CGameInstance::RemoveUI(const _wstring& canvasName, const _wstring& uiName)
{
	if (nullptr == m_pUIManager)
		return;
	m_pUIManager->RemoveUI(canvasName, uiName);
}
CUIObject* CGameInstance::Get_UI(const _wstring& canvasName, const _wstring& uiName)
{
	if (nullptr == m_pUIManager)
		return nullptr;
	return m_pUIManager->GetUI(canvasName, uiName);
}
CGameObject* CGameInstance::CreateUI(void* pDesc, UI_TYPE eUIType)
{
	return m_pUIManager->CreateUI(static_cast<CUIObject::UIOBJECT_DESC*>(pDesc), eUIType);
}
void CGameInstance::ClearUI()
{
	if (nullptr == m_pUIManager)
		return;
	m_pUIManager->ClearCanvas();
}
#pragma endregion

#pragma region RESOURCE_MANAGER
CShader* CGameInstance::GetShader(const _wstring& key, _bool bIsStatic)
{
	if (bIsStatic)
	{
		return m_pResourceMag->GetShader(key);
	}
	else
	{
		return m_pResourceMag->GetDynamicShader(key);
	}
}

CTexture* CGameInstance::GetTexture(const _wstring& key, _bool bIsStatic)
{
	if (bIsStatic)
	{
		return m_pResourceMag->GetTexture(key);
	}
	else
	{
		return m_pResourceMag->GetDynamicTexture(key);
	}
}

CVIBuffer* CGameInstance::GetBuffer(const _wstring& key, _bool bIsStatic)
{
	if (bIsStatic)
	{
		return m_pResourceMag->GetBuffer(key);
	}
	else
	{
		return m_pResourceMag->GetDynamicBuffer(key);
	}
}

CModel* CGameInstance::GetModel(const _wstring& key, _bool bIsStatic)
{
	if (bIsStatic)
	{
		return m_pResourceMag->GetModel(key);
	}
	else
	{
		return m_pResourceMag->GetDynamicModel(key);
	}
}


CShader* CGameInstance::LoadShader(const _wstring& key, const _wstring& vsPath, const D3D11_INPUT_ELEMENT_DESC* pElements, _uint iNumElements, _bool bIsStatic)
{
	if (bIsStatic)
	{
		return m_pResourceMag->LoadShader(key, vsPath, pElements, iNumElements);
	}
	else
	{
		return m_pResourceMag->LoadDynamicShader(key, vsPath, pElements, iNumElements);
	}
}
CTexture* CGameInstance::LoadTexture(const _wstring& key, const _wstring& filePath, _bool bIsStatic, _uint iNumTextrues)
{
	if (bIsStatic)
	{
		return m_pResourceMag->LoadTexture(key, filePath, iNumTextrues);
	}
	else
	{
		return m_pResourceMag->LoadDynamicTexture(key, filePath, iNumTextrues);
	}
}
CVIBuffer* CGameInstance::LoadBuffer(const _wstring& key, BUFFER_TYPE eType, _bool bIsStatic)
{
	if (bIsStatic)
	{
		return m_pResourceMag->LoadBuffer(key, eType);
	}
	else
	{
		return m_pResourceMag->LoadDynamicBuffer(key, eType);
	}
}
CModel* CGameInstance::LoadModel(const _wstring& key, const _wstring& filePath, MODEL eType, _matrix preMatrix, _bool bIsStatic)
{
	if (bIsStatic)
	{
		return m_pResourceMag->LoadModel(key, filePath, eType, preMatrix);
	}
	else
	{
		return m_pResourceMag->LoadDynamicModel(key, filePath, eType, preMatrix);
	}
}
const vector<_wstring>& CGameInstance::GetShaderKeys(_bool bIsStatic) const
{
	return m_pResourceMag->GetShaderKeys(bIsStatic);
}
const vector<_wstring>& CGameInstance::GetTextureKeys(_bool bIsStatic) const
{
	return m_pResourceMag->GetTextureKeys(bIsStatic);
}

const vector<_wstring>& CGameInstance::GetModelKeys(_bool bIsStatic) const
{
	return m_pResourceMag->GetModelKeys(bIsStatic);
}


#pragma endregion

#pragma region COLLIDER
void CGameInstance::Register_Collider(CCollider* pCollider)
{
	if (nullptr == m_pCollisionMag)
		return;
	m_pCollisionMag->Register(pCollider);
}

void CGameInstance::Unregister_Collider(CCollider* pCollider)
{
	if (nullptr == m_pCollisionMag)
		return;
	m_pCollisionMag->Unregister(pCollider);
}
#pragma endregion

#pragma region LIGHT_MANAGER
const LIGHT_DESC* CGameInstance::Get_Light(_uint iIndex)
{
	return m_pLight_Manager->Get_Light(iIndex);
}

HRESULT CGameInstance::Add_Light(const LIGHT_DESC& LightDesc)
{
	return m_pLight_Manager->Add_Light(LightDesc);
}
#pragma endregion

#pragma region FONT_MANAGER
HRESULT CGameInstance::Add_Font(const _wstring& strFontTag, const _tchar* pFontFilePath)
{
	return m_pFont_Manager->Add_Font(strFontTag, pFontFilePath);
}
void CGameInstance::Draw_Font(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition, _fvector vColor, _float fRotation, const _float2& vOrigin, _float fScale)
{
	m_pFont_Manager->Draw(strFontTag, pText, vPosition, vColor, fRotation, vOrigin, fScale);
}
#pragma endregion


void CGameInstance::Release_Engine()
{
	Safe_Release(m_pFont_Manager);

	Safe_Release(m_pLight_Manager);

	Safe_Release(m_pCollisionMag);

	Safe_Release(m_pPicking);
	
	Safe_Release(m_pTransformPipeline);

	Safe_Release(m_pUIManager);

	Safe_Release(m_pResourceMag);

	Safe_Release(m_pInput_Device);

	Safe_Release(m_pFrustumCull);

	Safe_Release(m_pTimer_Manager);

	Safe_Release(m_pRenderer);

	Safe_Release(m_pObject_Manager);

	Safe_Release(m_pPrototype_Manager);

	Safe_Release(m_pLevel_Manager);

	Safe_Release(m_pGraphic_Device);

	Destroy_Instance();
}

void CGameInstance::Free()
{
	__super::Free();

}
