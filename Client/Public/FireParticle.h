#pragma once
#include "ParticleEffect.h"

BEGIN_NAMESPACE(Client)
class CFireParticle : public CParticleEffect
{
private:
	CFireParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFireParticle(const CFireParticle& Prototype);
	virtual ~CFireParticle() = default;
public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void SetStartColor(const _float3& vColor);

private:
	virtual HRESULT Ready_Components() override;
public:
	static CFireParticle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

