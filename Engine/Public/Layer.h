#pragma once

#include "Base.h"

/* 객체들을 모아놓는다. */

BEGIN_NAMESPACE(Engine)

class CLayer final : public CBase
{
private:
	CLayer();
	virtual ~CLayer() = default;

public:
	class CComponent* Get_Component(const _wstring& strComponentTag, _uint iIndex);

public:
	HRESULT Add_GameObject(class CGameObject* pGameObject);
	void Priority_Update(_float fTimeDelta);
	void Update(_float fTimeDelta);
	void Late_Update(_float fTimeDelta);
	void Remove_GameObject(class CGameObject* pGameObject);
	void Remove_GameObjectByName(const _wstring& strName);
private:
	list<class CGameObject*>			m_GameObjects;

public:
	static CLayer* Create();
	virtual void Free();
};

END_NAMESPACE