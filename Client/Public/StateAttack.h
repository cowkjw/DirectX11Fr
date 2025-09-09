#pragma once
#include "State.h"

BEGIN_NAMESPACE(Client)
class StateAttack : public IState
{
public:
	enum class EAttackType{Attack1,Attack2,Attack3, Attack4,Up,Down,None};
public:
	StateAttack() = default;
	StateAttack(const _wstring& stateName)
		: IState(stateName) {
	}
	StateAttack(const _wstring& stateName,EAttackType eAttackType)
		: IState(stateName), m_eAttackType(eAttackType)
	{
	}
	~StateAttack() override = default;
	void Enter(CBaseCharacter* pChar) override;
	void Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta) override;
	void Exit(CBaseCharacter* pChar) override;

	_float GetAnimProgress(CBaseCharacter* pChar) const { return pChar->Get_Animator()->GetCurrentAnimProgress(); }
	EAttackType GetAttackType() const { return m_eAttackType; }
	string TriggerName() const
	{
		switch (m_eAttackType)
		{
		case EAttackType::Attack1:
		case EAttackType::Attack2:
		case EAttackType::Attack3:
		case EAttackType::Attack4:
			return "Attack";
		case EAttackType::Up:
			return "AttackUp";
		case EAttackType::Down:
			return "AttackDown";
		default:
			return "Attack";
		}
	}
	void ChangeSub(EAttackType next,EAttackType& cur, CBaseCharacter* pChar);
	_bool IsMove(const InputData& input) const{ return !XMVector3Equal(input.moveDir, XMVectorZero()); }
	_float AdvanceUntil();
private:
	_bool bIsCombo{ false }; // ÄÞº¸ ¿©ºÎ
	EAttackType m_eAttackType{ EAttackType::None };

};
END_NAMESPACE