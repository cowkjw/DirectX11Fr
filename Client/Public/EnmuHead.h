#pragma once
#include "EnmuParts.h"

BEGIN_NAMESPACE(Client)
class CEnmuHead final :  public CEnmuParts
{
private:
	CEnmuHead(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnmuHead(const CEnmuHead& Prototype);
	virtual ~CEnmuHead() = default;

public:
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	virtual HRESULT Ready_Components() override;
	virtual void Ready_Animation() override;
	virtual void Ready_Collider() override;
	virtual void ActiveCollider() override;
	virtual void DeactiveCollider() override;

private:
	class CBodyColliderParts* m_pBodyCollider = nullptr; // 콜라이더
public:
	static CEnmuHead* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,void* pArg = nullptr);
	static CGameObject* Clone(void* pArg);
	virtual void Free() override;


	// ICollisionListener을(를) 통해 상속됨
	virtual void OnCollisionEnter(CCollider* other, const XMFLOAT3& hitPos) override;
	virtual void OnCollisionEnter(CCollider* other) override;
	virtual void OnCollisionStay(CCollider* other, float fTimeDelta) override;
	virtual void OnCollisionExit(CCollider* other) override;

};
END_NAMESPACE

