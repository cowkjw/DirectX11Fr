#pragma once
#include "ParticleEffect.h"

BEGIN_NAMESPACE(Client)
class CHitShockParticle : public CParticleEffect
{
private:
	CHitShockParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CHitShockParticle(const CHitShockParticle& Prototype);
	virtual ~CHitShockParticle() = default;
public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	virtual void SpwanParticle() override;
	virtual void DespwanParticle() override;
	void SetInitParticleUV(_int col, _int row, _float fFramTime);

private:
	virtual HRESULT Ready_Components() override;
	
private:
	PARTICLE_UV m_InitParticleUV;
	PARTICLE_UV m_UseParticleUV;
public:
	static CHitShockParticle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

