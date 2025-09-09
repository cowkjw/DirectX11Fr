#include "StateSkill.h"
#include "StateIdle.h"
#include "StateMove.h"

void StateSkill::Enter(CBaseCharacter* pChar)
{
	if (m_eSkillType == ESkillType::None) 
		m_eSkillType = ESkillType::Skill0; // ±âº» Skill0

	auto pAnim = pChar->Get_Animator();
	pAnim->SetBool("Attacking", true);
	pAnim->SetBool("Move", false);
	if (m_eSkillType == ESkillType::Skill0 || m_eSkillType == ESkillType::Skill1)
		pAnim->SetBool("Jump", false);
	if (m_eSkillType == ESkillType::Skill2)
		pAnim->SetBool("Guard", true);

	pAnim->SetTrigger(TriggerName());

	switch (m_eSkillType)
	{
	case ESkillType::Skill0:
		pChar->SetState(CBaseCharacter::CSTATE::SKILL);
		break;
	case ESkillType::Skill1:
		pChar->SetState(CBaseCharacter::CSTATE::SKILL1);
		break;
	case ESkillType::Skill2:
		pChar->SetState(CBaseCharacter::CSTATE::SKILL2);
		break;
	default:
		break;
	}

}

void StateSkill::Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta)
{
	CGameInstance* gi = CGameInstance::Get_Instance();
	auto anim = pChar->Get_Animator();
	auto animCtrl = anim->GetAnimController();
	const string& stateName = animCtrl->GetCurrentState()->stateName;

	switch (m_eSkillType)
	{
	case ESkillType::Skill0:
		if (pChar->Get_Target())
		{
			pChar->GetTransform()->LookAtXZ(pChar->Get_Target()->GetTransform()->Get_State(STATE::POSITION));
		}
		break;
	case ESkillType::Skill1:
		if (pChar->Get_Target())
		{
			pChar->GetTransform()->LookAtXZ(pChar->Get_Target()->GetTransform()->Get_State(STATE::POSITION));
		}
		if (GetAnimProgress(pChar) <= 0.7f)
		{
			pChar->GetTransform()->Go_Straight(fTimeDelta, pChar->GetNavigation());
		}
		break;
	case ESkillType::Skill2:
	default:
		break;
	}

	if (anim->CheckBool("Attacking") == false)
	{
		if (IsMove(input)) pChar->ChangeState(new StateMove(TEXT("Move")));
		else               pChar->ChangeState(new StateIdle(TEXT("Idle")));
		return;
	}
}

void StateSkill::Exit(CBaseCharacter* pChar)
{
	auto pAnim = pChar->Get_Animator();
	switch (m_eSkillType)
	{
	case ESkillType::Skill0:
		pAnim->SetBool("Attacking", false);
		break;
	case ESkillType::Skill1:
		break;
	case ESkillType::Skill2:
		pAnim->SetBool("Attacking", false);
		pAnim->SetBool("Guard", false);
		break;
	default: 
		break;
	}
}
