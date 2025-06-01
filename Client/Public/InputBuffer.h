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
	Command* PopFront() {
		if (m_Commands.empty())
			return nullptr;
		Command* cmd = &m_Commands.front();
		m_Commands.pop_front();
		return cmd;
	}
	void ClearBuffer() {
		m_Commands.clear();}
	void PopCommand(ECommand type,_int iCount =-1);
	deque<Command> GetCommands() { return m_Commands; }

	_bool CheckCombo(const deque<Command>& commands, const vector<ECommand>& seq, _float fValidTime)
	{
		//if (commands.size() < seq.size()) return false;

		//// 뒤에서부터 “연속된 구간”으로 길이 seq 크기만큼 검사
		//for (size_t start = commands.size() - seq.size(); /* >=0 */; /*--*/)
		//{
		//	bool orderMatch = true;
		//	for (size_t i = 0; i < seq.size(); ++i)
		//	{
		//		if (commands[start + i].type != seq[i])
		//		{
		//			orderMatch = false;
		//			break;
		//		}
		//	}
		//	if (orderMatch)
		//	{
		//		float dt = commands[start + seq.size() - 1].timestamp
		//			- commands[start].timestamp;
		//		if (dt <= fValidTime)
		//			return true;
		//	}
		//	if (start == 0) break;
		//	--start;
		//}
		//return false;

			// 명령 수가 시퀀스 길이보다 작으면 바로 false
		if (commands.size() < seq.size())
			return false;

		// “앞에서부터” 연속된 구간을 길이 seq.size()만큼 검사
		// start가 0부터 commands.size() - seq.size() 까지 순방향으로 증가
		size_t nCmd = commands.size();
		size_t nSeq = seq.size();

		if (nCmd < nSeq)
			return false;

		// 1) commands 중에서 seq[0]를 찾는 모든 가능한 위치(startIdx)를 순회
		for (size_t startIdx = 0; startIdx < nCmd; ++startIdx)
		{
			if (commands[startIdx].type != seq[0])
				continue;

			// 2) seq[0] 위치(startIdx)를 확보했으면, 이후 seq[1..]를 순서대로 찾아본다
			size_t cmdIdx = startIdx;
			size_t matched = 1; // seq[0]는 이미 매칭됨

			for (size_t i = 1; i < nSeq; ++i)
			{
				// seq[i]를 찾기 위해 commands[cmdIdx+1..]를 순차 탐색
				bool found = false;
				for (size_t j = cmdIdx + 1; j < nCmd; ++j)
				{
					if (commands[j].type == seq[i])
					{
						found = true;
						cmdIdx = j; // seq[i] 매칭된 위치 갱신
						break;
					}
				}

				if (!found)
				{
					// seq[i]를 찾지 못하면, 이 startIdx 조합은 실패
					matched = i; // 몇 개까지 매칭됐는지 기록 (디버깅용)
					break;
				}
				++matched;
			}

			// 3) 만약 seq 전체(nSeq)만큼 매칭됐다면 시간 차 검사
			if (matched == nSeq)
			{
				// "맨 앞 명령"은 commands[startIdx], “맨 마지막(seq[nSeq-1])”은 commands[cmdIdx]
				float dt = commands[cmdIdx].timestamp - commands[startIdx].timestamp;
				if (dt <= fValidTime)
					return true;
				// 시간 초과라면, 다른 startIdx 시도
			}
			// seq가 전부 매칭되지 않았다면 다음 startIdx로 넘어간다
		}
		// 앞→뒤로 전부 검사해도 없으면 false
		return false;
	}

private:
	deque<Command> m_Commands;
	_float m_fTime = 1.f; // 유효 시간
	CGameInstance* m_pGameInstance = { nullptr };

public:
	static CInputBuffer* Create();
	virtual void Free() override;
};
END_NAMESPACE

