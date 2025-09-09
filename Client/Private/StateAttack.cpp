#include "StateAttack.h"
#include "StateMove.h"
#include "StateIdle.h"
#include "StateSkill.h"
#include "Akaza.h"

void StateAttack::Enter(CBaseCharacter* pChar)
{
	auto pAnim = pChar->Get_Animator();
	pAnim->SetBool("Attacking", true);
	pAnim->SetBool("Move", false);
	pChar->SetState(CBaseCharacter::CSTATE::ATTACK);
	string triggerName = TriggerName();
	pAnim->SetTrigger(triggerName);
	if (m_eAttackType == EAttackType::None)
		m_eAttackType = EAttackType::Attack1;

	if (m_eAttackType == EAttackType::Up || m_eAttackType == EAttackType::Down)
		pAnim->SetBool("Jump", false);
}

void StateAttack::Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta)
{
	CGameInstance* gi = CGameInstance::Get_Instance();
	auto pAnim = pChar->Get_Animator();
	auto animCtrl = pAnim->GetAnimController();
	const string& stateName = animCtrl->GetCurrentState()->stateName;
	auto buf = pChar->GetInputBuffer();
	auto pTrans = pChar->GetTransform();
	const _float fProgress = GetAnimProgress(pChar);

	if (fProgress <= AdvanceUntil())
	{
		pChar->GetTransform()->Go_Straight(fTimeDelta, pChar->GetNavigation());
	}

	if (input.doSkill1)
	{
		pChar->ChangeState(new StateSkill(TEXT("Skill1"),StateSkill::ESkillType::Skill1));
		return;
	}

	if (input.doSkill0)
	{
		pChar->ChangeState(new StateSkill(TEXT("Skill0"), StateSkill::ESkillType::Skill0));
		return;
	}

    switch (m_eAttackType)
    {
    case EAttackType::Attack1:
    {
        // 타겟 바라보기
        if (auto* tgt = pChar->Get_Target())
            pTrans->LookAtXZ(tgt->GetTransform()->Get_State(STATE::POSITION));

        if (input.doAttack2)
        {
            buf->PopCommand(ECommand::LightAttack, 2);
            ChangeSub(EAttackType::Attack2, m_eAttackType, pChar);
            return;
        }

    
        if (stateName == "attack0" && fProgress >= 1.f) {
            bIsCombo = false;
            if (IsMove(input)) pChar->ChangeState(new StateMove(TEXT("Move")));
            else               pChar->ChangeState(new StateIdle(TEXT("Idle")));
            return;
        }

        bIsCombo = true;
    }
    break;

    case EAttackType::Attack2:
    {
        if (input.doAttack3) {
            bIsCombo = true;
            buf->PopCommand(ECommand::LightAttack, 2);
            ChangeSub(EAttackType::Attack3, m_eAttackType, pChar);
            return;
        }

        if (fProgress >= 1.f) {
            bIsCombo = false;
            if (IsMove(input)) pChar->ChangeState(new StateMove(TEXT("Move")));
            else               pChar->ChangeState(new StateIdle(TEXT("Idle")));
            return;
        }

        bIsCombo = true; // 유지
    }
    break;

    case EAttackType::Attack3:
    {
        if (fProgress >= 0.7f)
        {
            if (input.doAttack4) {
                buf->PopCommand(ECommand::LightAttack, 2);
                bIsCombo = true;

                if (input.doAttack3Up)   ChangeSub(EAttackType::Up, m_eAttackType, pChar);
                else if (input.doAttack3Down) ChangeSub(EAttackType::Down, m_eAttackType, pChar);
                else                          ChangeSub(EAttackType::Attack4, m_eAttackType, pChar);
                return;
            }
            else 
            {
                
                bIsCombo = false;
                if (IsMove(input)) pChar->ChangeState(new StateMove(TEXT("Move")));
                else               pChar->ChangeState(new StateIdle(TEXT("Idle")));
                return;
            }
        }
    }
    break;

    case EAttackType::Attack4:
    {
        if (fProgress >= 1.f) 
        {
            bIsCombo = false;
            if (IsMove(input)) pChar->ChangeState(new StateMove(TEXT("Move")));
            else               pChar->ChangeState(new StateIdle(TEXT("Idle")));
            return;
        }
    }
    break;

    case EAttackType::Up:
    {
  
        if (fProgress >= 1.f) {
            bIsCombo = false;
            if (IsMove(input)) pChar->ChangeState(new StateMove(TEXT("Move")));
            else               pChar->ChangeState(new StateIdle(TEXT("Idle")));
            return;
        }
    }
    break;

    case EAttackType::Down:
    {
        if (fProgress >= 1.f)
        {
            bIsCombo = false;
            if (IsMove(input)) pChar->ChangeState(new StateMove(TEXT("Move")));
            else               pChar->ChangeState(new StateIdle(TEXT("Idle")));
            return;
        }
    }
    break;

    default:
        pChar->ChangeState(new StateIdle(TEXT("Idle")));
        return;
    }
}

void StateAttack::Exit(CBaseCharacter* pChar)
{
    switch (m_eAttackType)
    {
    case EAttackType::Attack1:
    case EAttackType::Attack2:
    case EAttackType::Attack3:
        pChar->Get_Animator()->SetBool("Attacking", bIsCombo);
        break;

    case EAttackType::Attack4:
        pChar->Get_Animator()->SetBool("Attacking", false);
        if (auto akaza = dynamic_cast<CAkaza*>(pChar))
            akaza->SetComState(CAkaza::COM_STATE::IDLE);
        break;

    case EAttackType::Up:
    case EAttackType::Down:
    default:
        break;
    }
}

void StateAttack::ChangeSub(EAttackType next, EAttackType& cur, CBaseCharacter* pChar)
{
    cur = next;
    auto* anim = pChar->Get_Animator();
    if (cur == EAttackType::Up || cur == EAttackType::Down)
        anim->SetBool("Jump", false);
    anim->SetTrigger(TriggerName());
    pChar->SetState(CBaseCharacter::CSTATE::ATTACK);
}

_float StateAttack::AdvanceUntil()
{
	switch (m_eAttackType)
    {
	case EAttackType::Attack1: return 0.10f;
	case EAttackType::Attack2: return 0.35f;
	case EAttackType::Attack3: return 0.20f;
	case EAttackType::Up:      return 0.40f;
	case EAttackType::Down:    return 0.50f;
	case EAttackType::Attack4:
	default:         return -1.f;
	}
}
