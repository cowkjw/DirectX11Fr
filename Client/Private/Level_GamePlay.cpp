#include "Level_GamePlay.h"
#include "GameInstance.h"

CGamePlay::CGamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
		: CLevel { pDevice, pContext }
{

}

HRESULT CGamePlay::Initialize()
{

	return S_OK;
}

void CGamePlay::Update(_float fTimeDelta)
{
	int a = 10;
}

HRESULT CGamePlay::Render()
{
	SetWindowText(g_hWnd, TEXT("게임플레이 레벨입니다."));

	return S_OK;
}

CGamePlay* CGamePlay::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGamePlay* pInstance = new CGamePlay(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CGamePlay");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CGamePlay::Free()
{
	__super::Free();

}
