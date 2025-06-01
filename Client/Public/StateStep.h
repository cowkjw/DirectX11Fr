#pragma once
#include "State.h"



BEGIN_NAMESPACE(Client)
class StateStep : public IState
{
public:
	StateStep() = default;
	StateStep(const _wstring& stateName)
		: IState(stateName) {
	}

	StateStep(EDirection eDirection)
		: m_eDirection(eDirection) {
	}
	StateStep(const _wstring& stateName, EDirection eDirection)
		: IState(stateName), m_eDirection(eDirection)
	{
	}


	~StateStep() override = default;
	void Enter(CBaseCharacter* pChar) override;
	void Update(CBaseCharacter* pChar, _float fTimeDelta) override;
	void Exit(CBaseCharacter* pChar) override;

private:
	EDirection m_eDirection{ EDirection::Forward }; // 현재 이동 방향
	_float m_fStepDistance{ 15.f }; // 스텝 이동 거리
	_float m_fStepDuration{ 0.5f }; // 스텝 지속 시간
	_float m_fElapsedTime{ 0.f }; // 경과 시간
	_vector m_vStartPosition{}; // 시작 위치
	_vector m_vTargetPosition{};
	_vector m_vStepDir{ }; // 스텝 방향 벡터

};
END_NAMESPACE