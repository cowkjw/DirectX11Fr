#pragma once
#include "Effect.h"

BEGIN_NAMESPACE(Client)

class CParticleEffect : public CEffect
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
	void EnableUVAnim(const _wstring& tag,_float cols, _float rows, _float frameSec);
	void ClearParticleSystems();

	virtual void SpwanParticle();
	virtual void DespwanParticle();

	virtual void OnEnable() override;
	virtual void SetTexture(CTexture* pTexture, _uint iTextureIndex = 0,const _wstring& tag = L"");
	virtual void SetShader(CShader* pShader, _uint iShaderPass = 0, const _wstring& tag = L"");

	void SetUseParentTexture(_bool bUseParentTexture) { m_bUseParentTexture = bUseParentTexture; }


	// 툴용 
	void AddParticleSystem_ForEditor(const _wstring& particleName, CParticleSystem* pParticleSystem,CTexture* pTexture,PARTICLE_UV particleUV, _uint iTextureIndex = 0, _uint iShaderPass = 0);
	unordered_map<_wstring, CParticleSystem*>& GetParticleSystems() { return m_ParticleSystems; }

protected:
	virtual HRESULT Ready_Components();

protected:
	unordered_map<_wstring, CParticleSystem*> m_ParticleSystems;
	unordered_map<_wstring, CTexture*> m_ParticleTextures;
	unordered_map<_wstring, CShader*> m_ParticleShaders;
	unordered_map<_wstring, _uint> m_ParticleShaderPasses; // 파티클 셰이더 패스
	unordered_map<_wstring, _uint> m_ParticleTextureIndices; // 파티클 텍스쳐 인덱스
	unordered_map<_wstring, PARTICLE_UV> m_ParticleUVs; // 파티클 UV 오프셋
	_bool m_bUseParentTexture = false; // 부모 텍스쳐 사용 여부


public:
	static CParticleEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END_NAMESPACE