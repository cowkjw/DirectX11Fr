#include "StateHurtDown.h"
#include "StateIdle.h"
#include "StateMove.h"

void StateHurtDown::Enter(CBaseCharacter* pChar)
{
	auto pAnim = pChar->Get_Animator();
	pAnim->SetBool("Hurted", true);
	pAnim->SetTrigger("HurtDown");
	pAnim->SetBool("Move", false);
	pChar->SetState(CBaseCharacter::CSTATE::DOWN);

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
}

void StateHurtDown::Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta)
{
	auto pAnim = pChar->Get_Animator();
	auto animCtrl = pAnim->GetAnimController();
	const string& stateName = animCtrl->GetCurrentState()->stateName;
	if (pAnim->CheckBool("Hurted") == false)
	{
		_bool bMoving = !XMVector3Equal(input.moveDir, XMVectorZero());
		if (bMoving)
		{
			pChar->ChangeState(new StateMove(TEXT("Move")));
		}
		else
		{
			pChar->ChangeState(new StateIdle(TEXT("Idle")));
		}
		return;
	}
}

void StateHurtDown::Exit(CBaseCharacter* pChar)
{
}
