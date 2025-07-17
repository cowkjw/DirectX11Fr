#pragma once
#include "MeshEffect.h"

BEGIN_NAMESPACE(Engine)
class CSphereCollider;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class CAkaKuBulletEffect : public CMeshEffect,public ICollisionListener
{
private:
	CAkaKuBulletEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAkaKuBulletEffect(const CAkaKuBulletEffect& Prototype);
	virtual ~CAkaKuBulletEffect() = default;

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
	_float m_fDir = 1.f; // 방향
	_float4 m_vColor = { 1.f, 1.f, 1.f, 1.f };
	_float m_fRotationSpeed = 2.f;
	_float m_fScale = 8.f; // 크기
	_float m_fDestScale = 9.f; // 크기
	_float m_fDuration = 1.0f; // 지속 시간

	class CSphereCollider* m_pColliderCom = { nullptr }; // 충돌체
	class CGameObject* m_pRingWindEffect = { nullptr }; // 링 바람 효과
	static _int m_iHitCount;

public:
	static CAkaKuBulletEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

public:


	// ICollisionListener을(를) 통해 상속됨
	void OnCollisionEnter(CCollider* other) override;
	void OnCollisionEnter(CCollider* other, const _float3& hitPos) override;

	void OnCollisionStay(CCollider* other, float fTimeDelta) override;

	void OnCollisionExit(CCollider* other) override;

};
END_NAMESPACE

