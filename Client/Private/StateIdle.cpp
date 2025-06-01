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

void StateIdle::Update(CBaseCharacter* pChar, _float fTimeDelta)
{
    auto gi = CGameInstance::Get_Instance();
    auto anim = pChar->Get_Animator();
    auto buf = pChar->GetInputBuffer();
    // 1타 공격 입력

    // 점프
    if (buf->CheckCommand(ECommand::Jump)) {
        buf->PopFront(1);
        pChar->ChangeState(new StateJump(TEXT("Jump")));
        return;
    }
    // 가드
    if (gi->IsKeyPressed('O'))
    {
        pChar->ChangeState(new StateGuard(TEXT("Guard")));
        return;
    }
    // 스킬
    if (buf->CheckCommand(ECommand::Skill0)) {
        buf->PopFront(1);
        pChar->ChangeState(new StateSkill0(TEXT("Skill0")));
        return;
    }
    // 이동
    _bool moving = gi->IsKeyDown(VK_UP) || gi->IsKeyDown(VK_DOWN) ||
        gi->IsKeyDown(VK_LEFT) || gi->IsKeyDown(VK_RIGHT);
    if (moving&&!anim->CheckBool("Attacking")&&!anim->CheckBool("Jump"))
    {
        pChar->ChangeState(new StateMove(TEXT("Move")));
        return;
    }

    if (buf->CheckCommand(ECommand::LightAttack))
    {
        buf->PopFront(1);
        pChar->ChangeState(new StateAttack1(TEXT("Skill1")));
        return;
    }
}

void StateIdle::Exit(CBaseCharacter* pChar)
{
}
