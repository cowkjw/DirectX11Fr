#pragma once
#include "ParticleEffect.h"

BEGIN_NAMESPACE(Client)
class CHitSlashCrossParticle : public CParticleEffect
{
private:
	CHitSlashCrossParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CHitSlashCrossParticle(const CHitSlashCrossParticle& Prototype);
	virtual ~CHitSlashCrossParticle() = default;
public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	virtual HRESULT Ready_Components() override;
public:
	static CHitSlashCrossParticle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

