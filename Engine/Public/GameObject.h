#pragma once

#include "Transform.h"

BEGIN(Engine)

class ENGINE_DLL CGameObject abstract : public CBase
{ public:
	typedef struct tagGameObjectDesc : public CTransform::TRANSFORM_DESC
	{

	}GAMEOBJECT_DESC;

protected:
	CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject(const CGameObject& Prototype);
	virtual ~CGameObject() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

public:
	_bool IsActive() const { return m_bIsActive; }
	void SetActive(_bool bActive) {
		if (m_bIsActive == bActive) 
			return;
		m_bIsActive = bActive;
		if (bActive) OnEnable();
		else         OnDisable();
	}
	virtual void OnEnable() {};
	virtual void OnDisable() {};

	CComponent* Get_Component(const _wstring& strComponentTag);

protected:
	HRESULT Add_Component(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strComponentTag, CComponent** ppOut, void* pArg = nullptr);

protected:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };

	CTransform*			m_pTransformCom = { nullptr };
	map<const _wstring, class CComponent*>		m_Components;
	_bool m_bIsActive{ true };
	_bool m_bIsPooled{ false };
	_bool m_isCloned = { false };
public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};
END