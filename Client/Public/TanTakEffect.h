#pragma once
#include "MeshEffect.h"
BEGIN_NAMESPACE(Engine)
class CSphereCollider;
END_NAMESPACE
BEGIN_NAMESPACE(Client)
class CTanTakEffect : public CMeshEffect, public ICollisionListener
{
private:
	CTanTakEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTanTakEffect(const CTanTakEffect& Prototype);
	virtual ~CTanTakEffect() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void SetColor(const _float4& vColor)
	{
		m_vColor = vColor;
	}

	void SetDirection(_float fDir)
	{
		m_fDir = fDir;
	}

private:
	HRESULT Ready_Components();
	virtual HRESULT Bind_Shader() override;
	virtual void OnDisable() override;
	virtual void OnEnable() override;

private:
	_bool m_bSpread = false; // 스프레드 여부
	_float m_fDir = 1.f; // 방향
	_float4 m_vColor = { 1.f, 1.f, 1.f, 1.f };
	_float m_fRotationSpeed = 0.7f;
	_float m_fScale = 15.f; // 크기
	_float m_fDestScale =20.f; // 크기
	_float m_fDuration = 1.0f; // 지속 시간
	CSphereCollider* m_pColliderCom = { nullptr };

public:
	static CTanTakEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;


	// ICollisionListener을(를) 통해 상속됨
	void OnCollisionEnter(CCollider* other) override;
	void OnCollisionEnter(CCollider* other, const _float3& hitPos) override;

	void OnCollisionStay(CCollider* other, float fTimeDelta) override;

	void OnCollisionExit(CCollider* other) override;
};
END_NAMESPACE

