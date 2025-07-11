#pragma once
#include "ParticleEffect.h"

BEGIN_NAMESPACE(Client)
class CEnkFireSpreadParticle : public CParticleEffect
{
private:
	CEnkFireSpreadParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnkFireSpreadParticle(const CEnkFireSpreadParticle& Prototype);
	virtual ~CEnkFireSpreadParticle() = default;
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
	static CEnkFireSpreadParticle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

