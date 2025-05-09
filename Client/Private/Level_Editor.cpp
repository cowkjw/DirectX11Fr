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
	SetWindowText(g_hWnd, TEXT("에디터 레벨입니다."));

	if (nullptr == m_pIMGUIMag)
		return E_FAIL;
	if (FAILED(m_pIMGUIMag->Render()))
		return E_FAIL;

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

}
