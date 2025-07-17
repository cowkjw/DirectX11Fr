#pragma once
#include "GameObject.h"
#include "Client_Defines.h"
#include "TanNejWindEffect.h"
#include "TanNejVortexEffect.h"


BEGIN_NAMESPACE(Client)
class CTanjiroNej : public CGameObject, public ICollisionListener
{
private:
	CTanjiroNej(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTanjiroNej(const CTanjiroNej& Prototype);
	virtual ~CTanjiroNej() = default;


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
	array<CTanNejWindEffect*,2> m_pWindEffects = { nullptr };
	CTanNejVortexEffect* m_pVortexEffect = { nullptr }; 
	_float m_fElpasedTime = 0.f; // 경과 시간
	_float m_fDuration = 0.f; // 지속 시간

	static constexpr _int PARTICLE_COUNT = 2;
	_bool  m_bSpawnedParticle[PARTICLE_COUNT] = { false, false, };
	const _float m_fParticleTriggers[PARTICLE_COUNT] = { 0.1f, 0.3f };
public:
	virtual void Free() override;
	virtual CGameObject* Clone(void* pArg) override;
	static CTanjiroNej* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);


	// ICollisionListener을(를) 통해 상속됨
	void OnCollisionEnter(CCollider* other) override;
	void OnCollisionEnter(CCollider* other, const _float3& hitPos) override;

	void OnCollisionStay(CCollider* other, float fTimeDelta) override;

	void OnCollisionExit(CCollider* other) override;


};
END_NAMESPACE

