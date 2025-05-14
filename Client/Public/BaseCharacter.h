#pragma once
#include "GameObject.h"
#include "Client_Defines.h"
BEGIN_NAMESPACE(Engine)
class CShader;
class CModel;
class CRigidBody;
class CPhysXCollider;
class CBoxCollider;
class CCapsuleCollider;
class CSphereCollider;
END_NAMESPACE

BEGIN_NAMESPACE(Client)	
class CBaseCharacter : public CGameObject
{
public:
	enum class CSTATE { IDLE, MOVE, ATTACK, SKILL, HURT, DIE };
protected:
	CBaseCharacter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBaseCharacter(const CBaseCharacter& Prototype);
	virtual ~CBaseCharacter() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

protected:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CCapsuleCollider* m_pColliderCom = { nullptr };

protected:
	virtual HRESULT Ready_Components();

protected:
	_float m_fMaxHP{ 0.f };           // 최대 체력
	_float m_fCurrentHP{ 0.f };       // 현재 체력
	_float m_fStamina{ 0.f };         // 스태미나(호흡력)

public:
	static CBaseCharacter* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};
END_NAMESPACE

