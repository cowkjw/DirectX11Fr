#include "Level_Editor.h"
#include "GameInstance.h"

CLevel_Editor::CLevel_Editor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
		: CLevel { pDevice, pContext }
{

}

HRESULT CLevel_Editor::Initialize()
{
	m_pIMGUIMag = CIMGUIMag::Create(m_pDevice, m_pContext);

	if (nullptr == m_pIMGUIMag)
		return E_FAIL;


	if (!m_pGameInstance->Add_GameObject(ToIndex(LEVEL::STATIC), TEXT("Prototype_GameObject_Camera_Free"),
		ToIndex(LEVEL::STATIC), TEXT("Layer_Camera")))
		return E_FAIL;

	LIGHT_DESC			LightDesc{};

	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vAmbient = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;


	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, 1.f, 1.f, 0.f);
	LightDesc.vDiffuse = _float4(0.5f, 0.7f, 0.5f, 1.f);
	LightDesc.vAmbient = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;


	return S_OK;
}

void CLevel_Editor::Update(_float fTimeDelta)
{
	if (nullptr == m_pIMGUIMag)
		return;
	m_pIMGUIMag->Update(fTimeDelta);
}

HRESULT CLevel_Editor::Render()
{
	if (nullptr == m_pIMGUIMag)
		return E_FAIL;
	if (FAILED(m_pIMGUIMag->Render()))
		return E_FAIL;

	SetWindowText(g_hWnd, TEXT("에디터 레벨입니다."));

	return S_OK;
}

CLevel_Editor* CLevel_Editor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_Editor* pInstance = new CLevel_Editor(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_Editor");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_Editor::Free()
{
	__super::Free();
	Safe_Release(m_pGameInstance);
	Safe_Release(m_pIMGUIMag);
}
