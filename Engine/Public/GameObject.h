#pragma once

#include "Transform.h"

BEGIN_NAMESPACE(Engine)

class ENGINE_DLL CGameObject abstract : public CBase
{ public:
	typedef struct tagGameObjectDesc : public CTransform::TRANSFORM_DESC
	{
		_wstring strName;
		_wstring strTag;
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

	virtual void OnEnable() {};
	virtual void OnDisable() {};

public:
	_bool IsActive() const { return m_bIsActive; }
	void SetActive(_bool bActive) {
		if (m_bIsActive == bActive) 
			return;
		m_bIsActive = bActive;
		if (bActive) OnEnable();
		else         OnDisable();
	}

	CComponent* Get_Component(const _wstring& strComponentTag);

	const _wstring& Get_Name() const { return m_strName; }

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
	_bool m_bIsCloned = { false };
	_wstring m_strName{};
	_wstring m_strTag;
public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};
END_NAMESPACE