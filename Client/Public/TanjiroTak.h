#pragma once
#include "GameObject.h"
#include "Client_Defines.h"
#include "TanTakEffect.h"


BEGIN_NAMESPACE(Client)
class CTanjiroTak : public CGameObject, public ICollisionListener
{
private:
	CTanjiroTak(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTanjiroTak(const CTanjiroTak& Prototype);
	virtual ~CTanjiroTak() = default;


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
	CTanTakEffect* m_pTakEffect = { nullptr }; 
	_float m_fElpasedTime = 0.f; // 경과 시간
	_float m_fDuration = 0.f; // 지속 시간
public:
	virtual void Free() override;
	virtual CGameObject* Clone(void* pArg) override;
	static CTanjiroTak* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);


	// ICollisionListener을(를) 통해 상속됨
	void OnCollisionEnter(CCollider* other) override;
	void OnCollisionEnter(CCollider* other, const _float3& hitPos) override;

	void OnCollisionStay(CCollider* other, float fTimeDelta) override;

	void OnCollisionExit(CCollider* other) override;


};
END_NAMESPACE

