#pragma once
#include "GameObject.h"
#include "Client_Defines.h"
#include "KyojuroEnkEffect.h"
#include "KyojuroEnkSpiralEffect.h"

BEGIN_NAMESPACE(Engine)
class CBoxCollider;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class CKyojuroEnk : public CGameObject, public ICollisionListener
{
private:
	CKyojuroEnk(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKyojuroEnk(const CKyojuroEnk& Prototype);
	virtual ~CKyojuroEnk() = default;


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
	vector<CKyojuroEnkSpiralEffect*> m_SpiralEffects; // 나선형 이펙트들
	vector<CKyojuroEnkEffect*> m_Effects; // 파이어 이펙트들
	CKyojuroEnkEffect* m_pEnkEffect = { nullptr }; // 파이어 본체
	_float m_fElpasedTime = 0.f; // 경과 시간
	_float m_fDuration = 0.f; // 지속 시간
	CBoxCollider* m_pColliderCom = { nullptr };
public:
	virtual void Free() override;
	virtual CGameObject* Clone(void* pArg) override;
	static CKyojuroEnk* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);


	// ICollisionListener을(를) 통해 상속됨
	void OnCollisionEnter(CCollider* other) override;
	void OnCollisionEnter(CCollider* other, const _float3& hitPos) override;

	void OnCollisionStay(CCollider* other, float fTimeDelta) override;

	void OnCollisionExit(CCollider* other) override;


};
END_NAMESPACE

