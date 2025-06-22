#pragma once
#include "GameObject.h"
#include "Client_Defines.h"

BEGIN_NAMESPACE(Engine)
class CModel;
class CAnimator;
class CBone;
class CShader;
END_NAMESPACE


BEGIN_NAMESPACE(Client)
class CEnmuTentacle :public CGameObject,public ICollisionListener
{
private:
	CEnmuTentacle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnmuTentacle(const CEnmuTentacle& rhs);
	virtual ~CEnmuTentacle() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	CAnimator* Get_Animator() const { return m_pAnimatorCom; }
	void PlayAniamtion();

	virtual void OnEnable() override;
	virtual void OnDisable() override;
	void SetTarget(CGameObject* pTarget) { m_pTarget = pTarget; }
private:
	HRESULT Ready_Components();
	HRESULT Bind_Shaders();
	void Ready_Animations();
private:
	CModel* m_pModelCom = nullptr;
	CAnimator* m_pAnimatorCom = nullptr;
	CShader* m_pShaderCom = nullptr;
	class CBodyColliderParts* m_pBodyCollider = nullptr;
	CGameObject* m_pTarget{ nullptr };
	_float m_fTimeElapsed = 0.f;
	_bool m_bAnimPlaying = false;


public:
	static CEnmuTentacle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free()override;

	// ICollisionListener을(를) 통해 상속됨
	void OnCollisionEnter(CCollider* other) override;
	void OnCollisionStay(CCollider* other, float fTimeDelta) override;
	void OnCollisionExit(CCollider* other) override;
};
END_NAMESPACE

