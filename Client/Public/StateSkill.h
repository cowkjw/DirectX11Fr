#pragma once
#include "State.h"

BEGIN_NAMESPACE(Client)
class StateSkill : public IState
{
public:
	enum class ESkillType { Skill0, Skill1, Skill2, None };
public:
	StateSkill() = default;
	StateSkill(const _wstring& stateName)
		: IState(stateName) {
	}
	StateSkill(const _wstring& stateName, ESkillType eSkillType)
		: IState(stateName), m_eSkillType(eSkillType)
	{
	}
	~StateSkill() override = default;
	void Enter(CBaseCharacter* pChar) override;
	void Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta) override;
	void Exit(CBaseCharacter* pChar) override;
	_float GetAnimProgress(CBaseCharacter* pChar) const { return pChar->Get_Animator()->GetCurrentAnimProgress(); }
	ESkillType GetSkillType() const { return m_eSkillType; }
	_float AdvanceUntil() {
		return (m_eSkillType == ESkillType::Skill1) ? 0.7f : -1.f;
	}
	_bool IsMove(const InputData& input) const { return !XMVector3Equal(input.moveDir, XMVectorZero()); }
	string TriggerName() const
	{
		switch (m_eSkillType)
		{
		case ESkillType::Skill0:
			return "Skill0";
		case ESkillType::Skill1:
			return "Skill1";
		case ESkillType::Skill2:
			return "Skill2";
		default:
			return "Skill0";
		}
	}
private:
	ESkillType m_eSkillType{ ESkillType::None };
};
END_NAMESPACE