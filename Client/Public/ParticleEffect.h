#pragma once

#include "Effect.h"

BEGIN_NAMESPACE(Client)

class CParticleEffect final : public CEffect
{
protected:
	CParticleEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CParticleEffect(const CParticleEffect& Prototype);
	virtual ~CParticleEffect() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

protected:
	HRESULT Ready_Components();

public:
	static CParticleEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END_NAMESPACE