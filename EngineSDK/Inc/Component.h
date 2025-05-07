#pragma once

#include "Base.h"

BEGIN_NAMESPACE(Engine)

class ENGINE_DLL CComponent abstract : public CBase
{
protected:
	CComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CComponent(const CComponent& Prototype);
	virtual ~CComponent() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);	

public:
	_bool IsActive() const { return m_bIsActive; }
	void SetActive(_bool bActive) {	m_bIsActive = bActive;}

protected:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };

	_bool						m_isCloned = { false };
	_bool m_bIsActive{ true };
public:
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END_NAMESPACE