#include "Pannel.h"
#include "GameInstance.h"

CPannel::CPannel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CPannel::Initialize()
{
    return S_OK;
}

void CPannel::Update(_float fTimeDelta)
{
}

HRESULT CPannel::Render()
{
    return S_OK;
}

CPannel* CPannel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPannel* pInstance = new CPannel(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CPannel");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CPannel::Free()
{
	__super::Free();
	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);
	Safe_Release(m_pGameInstance);
}
