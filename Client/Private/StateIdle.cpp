#include "StateIdle.h"
#include "Animator.h"
#include "GameInstance.h"
#include "BaseCharacter.h"
#include "AnimController.h"
#include "StateAttack1.h"
#include "StateJump.h"
#include "StateMove.h"
#include "StateGuard.h"
#include "StateSkill0.h"    
#include "InputBuffer.h"
#include "Command.h"

void StateIdle::Enter(CBaseCharacter* pChar)
{
    auto anim = pChar->Get_Animator();
    anim->SetBool("Move", false);
    anim->SetBool("Jump", false);
    anim->SetBool("Attacking", false);
    anim->SetBool("Guard", false);
    auto buf = pChar->GetInputBuffer();
	buf->PopCommand(ECommand::LightAttack); // 1타 공격 입력 초기화
    pChar->SetState(CBaseCharacter::CSTATE::IDLE);
}

void StateIdle::Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta)
{
    auto gi = CGameInstance::Get_Instance();
    auto anim = pChar->Get_Animator();
    auto buf = pChar->GetInputBuffer();
    // 1타 공격 입력

    // 점프
    if (input.doJump) 
    {
        buf->PopCommand(ECommand::Jump,.1);
        pChar->ChangeState(new StateJump(TEXT("Jump")));
        return;
    }
    // 가드
    if (input.doGuard)
    {
        buf->PopCommand(ECommand::Guard,1);
        pChar->ChangeState(new StateGuard(TEXT("Guard")));
        return;
    }
    // 스킬

    if (input.doSkill0)
    {
        buf->PopCommand(ECommand::Skill0, 1);
        pChar->ChangeState(new StateSkill0(TEXT("Skill0")));
        return;
    }
    //if (buf->CheckCommand(ECommand::Skill0)) {
    //    buf->PopFront(1);
    //    pChar->ChangeState(new StateSkill0(TEXT("Skill0")));
    //    return;
    //}
    // 이동
    _bool bMoving = !XMVector3Equal(input.moveDir, XMVectorZero());
    if (bMoving &&!anim->CheckBool("Attacking")&&!anim->CheckBool("Jump"))
    {
        pChar->ChangeState(new StateMove(TEXT("Move")));
        return;
    }

    if (input.doAttack)
    {
        pChar->ChangeState(new StateAttack1(TEXT("Attack1")));
        return;
    }
}

void StateIdle::Exit(CBaseCharacter* pChar)
{
}
