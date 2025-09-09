#include "StateHurt.h"
#include "StateIdle.h"
#include "StateMove.h"

void StateHurt::Enter(CBaseCharacter* pChar)
{
	auto pAnim = pChar->Get_Animator();

	pAnim->SetBool("Hurted", true);
	pAnim->SetTrigger(TriggerName());
	pAnim->SetBool("Move", false);

	switch (m_eHurtType)
	{
	case EHurtType::Hurt:
		pChar->SetState(CBaseCharacter::CSTATE::HURT);
		break;
	case EHurtType::HurtDown:
		pChar->SetState(CBaseCharacter::CSTATE::DOWN);
		break;
	case EHurtType::HurtBlow:
	case EHurtType::HurtAir:
		pChar->SetState(CBaseCharacter::CSTATE::AIR);
		break;
	case EHurtType::HurtBound:
		pChar->SetState(CBaseCharacter::CSTATE::BOUND);
		break;
	default:
		pChar->SetState(CBaseCharacter::CSTATE::HURT);
		pChar->DeactiveCollider();
		break;
	}
	SpecficSoundEffect(pChar);
}

void StateHurt::Update(CBaseCharacter* pChar, const InputData& input, _float fTimeDelta)
{
	auto pAnim = pChar->Get_Animator();

	if (pAnim->CheckBool("Hurted") == false)
	{
		if (IsMove(input)) pChar->ChangeState(new StateMove(TEXT("Move")));
		else               pChar->ChangeState(new StateIdle(TEXT("Idle")));
		return;
	}
}

void StateHurt::Exit(CBaseCharacter* pChar)
{
	auto pAnim = pChar->Get_Animator();
	pAnim->SetBool("Hurted", false);
}

void StateHurt::SpecficSoundEffect(CBaseCharacter* pChar)
{
	switch (m_eHurtType)
	{
	case EHurtType::Hurt:
		if (pChar->Get_Name() == TEXT("Kyojuro"))
		{
			CSoundMag::Get_Instance()->StopEffect("event:/Kyojuro/Nob");
			CSoundMag::Get_Instance()->StopEffect("event:/Kyojuro/Enk");
			CSoundMag::Get_Instance()->StopEffect("event:/Kyojuro/Kie");
			CSoundMag::Get_Instance()->PlayOneShot("event:/Kyojuro/Hited");
		}
		else if (pChar->Get_Name() == TEXT("Akaza"))
		{
			CSoundMag::Get_Instance()->PlayOneShot("event:/Akaza/Hited");
		}
		else
		{
			CSoundMag::Get_Instance()->PlayOneShot("event:/Common/BodyAttack");
			CSoundMag::Get_Instance()->PlayOneShot("event:/Tanjiro/Hited");
		}
		break;
	case EHurtType::HurtDown:
		if (pChar->Get_Name() == TEXT("Kyojuro"))
		{
			CSoundMag::Get_Instance()->StopEffect("event:/Kyojuro/Nob");
			CSoundMag::Get_Instance()->StopEffect("event:/Kyojuro/Enk");
			CSoundMag::Get_Instance()->StopEffect("event:/Kyojuro/Kie");
			CSoundMag::Get_Instance()->PlayOneShot("event:/Kyojuro/Hited");
		}
		else if (pChar->Get_Name() == TEXT("Akaza"))
		{
			CSoundMag::Get_Instance()->PlayOneShot("event:/Akaza/Hited");
		}
		else
		{
			CSoundMag::Get_Instance()->PlayOneShot("event:/Common/BodyAttack");
			CSoundMag::Get_Instance()->StopEffect("event:/Tanjiro/Tak");
			CSoundMag::Get_Instance()->StopEffect("event:/Tanjiro/Nej");
			CSoundMag::Get_Instance()->StopEffect("event:/Tanjiro/Mig");
			CSoundMag::Get_Instance()->PlayOneShot("event:/Tanjiro/Hited");
		}
		break;
	case EHurtType::HurtBlow:
	case EHurtType::HurtAir:
	case EHurtType::HurtBound:
		if (pChar->Get_Name() == TEXT("Kyojuro"))
		{
			CSoundMag::Get_Instance()->PlayOneShot("event:/Kyojuro/Blow");
		}
		else if (pChar->Get_Name() == TEXT("Akaza"))
		{
			CSoundMag::Get_Instance()->PlayOneShot("event:/Akaza/Blow");
		}
		else
		{
			CSoundMag::Get_Instance()->PlayOneShot("event:/Common/BodyAttack");
			CSoundMag::Get_Instance()->PlayOneShot("event:/Tanjiro/Blow");
		}
		break;
	}
}
