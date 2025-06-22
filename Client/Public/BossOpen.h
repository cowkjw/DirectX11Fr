#pragma once
#include "BossState.h"

BEGIN_NAMESPACE(Client)
class BossOpen : public BossState
{
public:
	BossOpen() = default;
	BossOpen(const _wstring& stateName)
		: BossState(stateName) {
	}
	~BossOpen() override = default;
	void Enter(CEnmuMeat* pChar) override;
	void Update(CEnmuMeat* pChar, _float fTimeDelta) override;
	void Exit(CEnmuMeat* pChar) override;
private:
	_float m_fOpenTime{ 0.f }; // ¿ÀÇÂ ½Ã°£
	_bool m_bIsOpen{ false }; // ¿ÀÇÂ ¿©ºÎ
	_float m_fOpenDuration{ 6.f }; // ¿ÀÇÂ Áö¼Ó ½Ã°£
};
END_NAMESPACE

