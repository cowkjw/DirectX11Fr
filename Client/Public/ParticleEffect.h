#pragma once

#include "Effect.h"

BEGIN_NAMESPACE(Client)

class CParticleEffect  : public CEffect
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

public:
	void AddParticleSystem(const _wstring& particleName, CParticleSystem* pParticleSystem);
	void RemoveParticleSystem(CParticleSystem* pParticleSystem);
	void RemoveParticleSystem(const _wstring& particleName);
	void ClearParticleSystems();

	void SpwanParticle();
	void DespwanParticle();

	virtual void OnEnable() override;

protected:
	virtual HRESULT Ready_Components();

protected:
	unordered_map<_wstring, CParticleSystem*> m_ParticleSystems;
	unordered_map<_wstring, CTexture*> m_ParticleTextures;
	unordered_map<_wstring, CShader*> m_ParticleShaders;

public:
	static CParticleEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END_NAMESPACE