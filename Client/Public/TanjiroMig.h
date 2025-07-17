#pragma once
#include "GameObject.h"
#include "Client_Defines.h"
#include "TanMigEffect.h"
#include "TanMigRingEffect.h"


BEGIN_NAMESPACE(Client)
class CTanjiroMig : public CGameObject, public ICollisionListener
{
private:
	CTanjiroMig(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTanjiroMig(const CTanjiroMig& Prototype);
	virtual ~CTanjiroMig() = default;


public:
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override;
	void Update(_float fTimeDelta) override;
	void Late_Update(_float fTimeDelta) override;

public:
	void RotationDirection(_fvector vDir);
	void SetPosition(_fvector vPos);
	virtual void OnEnable() override;
	virtual void OnDisable() override;
private:
	HRESULT Ready_Effects();

private:
	CTanMigEffect* m_pMigEffect = { nullptr }; 
	CTanMigRingEffect* m_pMigRingEffect = { nullptr }; 
	_float m_fElpasedTime = 0.f; // 경과 시간
	_float m_fDuration = 0.f; // 지속 시간


	static constexpr _int PARTICLE_COUNT = 2;
	_bool  m_bSpawnedParticle[PARTICLE_COUNT] = { false, false, };
	const _float m_fParticleTriggers[PARTICLE_COUNT] = { 0.1f, 0.3f };
public:
	virtual void Free() override;
	virtual CGameObject* Clone(void* pArg) override;
	static CTanjiroMig* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);


	// ICollisionListener을(를) 통해 상속됨
	void OnCollisionEnter(CCollider* other) override;
	void OnCollisionEnter(CCollider* other, const _float3& hitPos) override;

	void OnCollisionStay(CCollider* other, float fTimeDelta) override;

	void OnCollisionExit(CCollider* other) override;


};
END_NAMESPACE

