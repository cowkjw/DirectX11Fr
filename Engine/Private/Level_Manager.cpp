#include "Level_Manager.h"
#include "Level.h"

#include "GameInstance.h"

CLevel_Manager::CLevel_Manager()
    : m_pGameInstance { CGameInstance::Get_Instance() }
{
    Safe_AddRef(m_pGameInstance);
}

HRESULT CLevel_Manager::Prepanding_Change_Level(_uint iLevelIndex, CLevel* pNewLevel)
{
	if (pNewLevel == nullptr)
		return E_FAIL;

    m_pNewLevel = pNewLevel;

    m_iNewLevelIndex = iLevelIndex;

    return S_OK;
}

HRESULT CLevel_Manager::Change_Level()
{
	if (nullptr == m_pNewLevel)
		return E_FAIL;
    /* 기존 레벨용 자원을 파괴해준다.*/
    if (nullptr != m_pCurrentLevel)
    {
        m_pGameInstance->Clear(m_iCurrentLevelIndex);
        Safe_Release(m_pCurrentLevel);
    }   

    m_pCurrentLevel = m_pNewLevel;

    m_iCurrentLevelIndex = m_iNewLevelIndex;
	m_pNewLevel = nullptr;

    return S_OK;
}

void CLevel_Manager::Update(_float fTimeDelta)
{
    if (nullptr != m_pCurrentLevel)
        m_pCurrentLevel->Update(fTimeDelta);
}

HRESULT CLevel_Manager::Render()
{
    if(nullptr != m_pCurrentLevel)
        m_pCurrentLevel->Render();

    return S_OK;
}

CLevel_Manager* CLevel_Manager::Create()
{
	return new CLevel_Manager();
}


void CLevel_Manager::Free()
{
    __super::Free();

    Safe_Release(m_pCurrentLevel); 
    Safe_Release(m_pGameInstance);
}