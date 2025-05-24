#pragma once
#include "Base.h"
#include "Command.h"
#include "Client_Defines.h"

BEGIN_NAMESPACE(Engine)
class CGameInstance;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class CInputBuffer final : public CBase
{
private:
	CInputBuffer();
	virtual ~CInputBuffer() = default;

public:
	void Update(_float fTime);
	void AddCommand(const Command& cmd);
	_bool CheckCommand(ECommand eCmd);
	_bool CheckCombo(const vector<ECommand>& seq);
	void PopFront(size_t n) {
		while (n-- && !m_Commands.empty())
			m_Commands.pop_front();
	}
	void ClearBuffer() {
		m_Commands.clear();}
	void PopCommand(ECommand type);

private:
	deque<Command> m_Commands;
	_float m_fTime = 0.2f; // 유효 시간
	CGameInstance* m_pGameInstance = { nullptr };

public:
	static CInputBuffer* Create();
	virtual void Free() override;
};
END_NAMESPACE

