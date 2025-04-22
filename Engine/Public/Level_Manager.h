#pragma once

#include "Base.h"

/* 기존 레벨을 파괴하고 새롭게 할당된 레벨의 주소를 들고 있는다. */
/* 위의 작업을 수행 시, 기존 레벨용 자원을 모두 파괴한다. */
/* 현재 할당된 레벨을 갱신하고 보여준다. */

BEGIN(Engine)

class CLevel_Manager final : public CBase
{
private:
	CLevel_Manager();
	virtual ~CLevel_Manager() = default;

public:
	HRESULT Apply_Pending_Change_Level();
	void Request_Change_Level(_uint iLevelIndex, class CLevel* pNewLevel);
	void Update(_float fTimeDelta);
	HRESULT Render();

private:
	HRESULT Change_Level(_uint iLevelIndex, class CLevel* pNewLevel);

private:
	_uint					m_iCurrentLevelIndex = {};
	class CLevel*			m_pCurrentLevel = { nullptr };
	class CGameInstance*	m_pGameInstance = { nullptr };
	_bool					m_bLevelChangeRequested = false;
	_uint					m_iRequestedLevelIndex = 0;
	class CLevel* m_pRequestedLevel = nullptr;

public:
	static CLevel_Manager* Create();
	virtual void Free() override;
};

END