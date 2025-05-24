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
    virtual ~IState() = default;
    // 상태 진입 시: 애니메이션 세팅, 초기화
    virtual void Enter(CBaseCharacter* pChar) = 0;
    // 매 프레임: 입력 체크 · 이동/애니 처리 · 전이 판단
    virtual void Update(CBaseCharacter* pChar, _float fTimeDelta) = 0;
    // 상태 종료 시: 후처리
    virtual void Exit(CBaseCharacter* pChar) = 0;
};
END_NAMESPACE
