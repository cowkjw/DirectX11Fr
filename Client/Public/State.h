#pragma once
#include "Client_Defines.h"
#include "Animator.h"
#include "GameInstance.h"
#include "BaseCharacter.h"
#include "InputBuffer.h"
#include "AnimController.h"

BEGIN_NAMESPACE(Engine)
class CAnimator;
class CGameObject;
class CShader;
class CModel;
class CRigidBody;
class CPhysXCollider;
class CBoxCollider;
class CCapsuleCollider;
class CSphereCollider;
class CGameInstance;
class CTransform;
class CCollider;
class CAnimController;
class CAnimation;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class IState 
{
public:
	IState() = default;
	IState(const _wstring& stateName)
		: m_stateName(stateName) {
	}
    virtual ~IState() = default;
    // 상태 진입 시: 애니메이션 세팅, 초기화
    virtual void Enter(CBaseCharacter* pChar) = 0;
    // 매 프레임: 입력 체크 · 이동/애니 처리 · 전이 판단
    virtual void Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta) = 0;
    // 상태 종료 시: 후처리
    virtual void Exit(CBaseCharacter* pChar) = 0;
//	void SetStateName(const _wstring& stateName) { m_stateName = stateName; }
	const _wstring& GetStateName() const { return m_stateName; }
protected:
	_wstring m_stateName; // 상태 이름
};
END_NAMESPACE
