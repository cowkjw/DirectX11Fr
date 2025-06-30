#pragma once
#include "BaseCharacter.h"

BEGIN_NAMESPACE(Client)
class CTanjiro :  public CBaseCharacter
{
private:
	CTanjiro(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTanjiro(const CTanjiro& Prototype);
	virtual ~CTanjiro() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual void TakeDamage(_float fDamage) override;
	virtual void OnAttackHit(CGameObject* pTarget) override;

private:
	virtual HRESULT Ready_Components() override;
	virtual void Ready_Animation() override;
	void ReadyAnimEvents();
	void ActiveCollider();
	void DeactiveCollider();


public:
	static CTanjiro* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

