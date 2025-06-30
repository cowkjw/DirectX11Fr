#pragma once
#include "BaseCharacter.h"

BEGIN_NAMESPACE(Client)
class CKyojuro :public CBaseCharacter
{
private:
	CKyojuro(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKyojuro(const CKyojuro& Prototype);
	virtual ~CKyojuro() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual void TakeDamage(_float fDamage) override;
	virtual void OnAttackHit(CGameObject* pTarget) override;
	virtual void OnCollisionEnter(CCollider* other) override;
	virtual void OnCollisionEnter(CCollider* other, const XMFLOAT3& hitPos) override;

private:
	virtual HRESULT Ready_Components() override;
	void ReadyAnimEvents();
	virtual void Ready_Animation() override;
	void ActiveCollider();
	void DeactiveCollider();

public:
	static CKyojuro* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

