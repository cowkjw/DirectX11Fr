#include "Panel.h"
#include "GameInstance.h"

CPanel::CPanel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CPanel::Initialize()
{
    return S_OK;
}

void CPanel::Update(_float fTimeDelta)
{
}

HRESULT CPanel::Render()
{
    return S_OK;
}

CPanel* CPanel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPanel* pInstance = new CPanel(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CPanel");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CPanel::Free()
{
	__super::Free();
	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);
	Safe_Release(m_pGameInstance);
}
