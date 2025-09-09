#pragma once
#include "State.h"
class StateHurt :public IState
{
public:
	enum class EHurtType
	{
		Hurt,
		HurtDown,
		HurtBlow,
		HurtAir,
		HurtBound,
		NONE
	};
public:
	StateHurt() : IState(TEXT("Hurt")) {}
	StateHurt(EHurtType eType) : IState(TEXT("Hurt")), m_eHurtType(eType) {}
	virtual ~StateHurt() = default;
	virtual void Enter(CBaseCharacter* pChar) override;
	virtual void Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta) override;
	virtual void Exit(CBaseCharacter* pChar) override;

private:
	_float GetAnimProgress(CBaseCharacter* pChar) const { return pChar->Get_Animator()->GetCurrentAnimProgress(); }
	EHurtType GetAttackType() const { return m_eHurtType; }
	string TriggerName() const
	{
		switch (m_eHurtType)
		{
		case EHurtType::Hurt:
			return "Hurt";
		case EHurtType::HurtDown:
			return "HurtDown";
		case EHurtType::HurtBlow:
			return "HurtBlow";
		case EHurtType::HurtAir:
			return "HurtAir";
		case EHurtType::HurtBound:
			return "HurtBound";
		default:
			return "Attack";
		}
	}
	_bool IsMove(const InputData& input) const { return !XMVector3Equal(input.moveDir, XMVectorZero()); }
	_float AdvanceUntil();

	void SpecficSoundEffect(CBaseCharacter* pChar);
private:
	EHurtType m_eHurtType = EHurtType::NONE;
};

