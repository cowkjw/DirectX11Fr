#include "InputBuffer.h"
#include "GameInstance.h"

CInputBuffer::CInputBuffer()
	: m_Commands{}
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
}

void CInputBuffer::Update(_float fTime)
{
	if (m_Commands.size() >= 3)
	{
		int a = 0;
	}
	// m_fTime(0.2f) 보다 오래된 명령은 전부 제거
	while (!m_Commands.empty() &&
		fTime - m_Commands.front().timestamp > m_fTime)
	{
		m_Commands.pop_front();
	}
}

void CInputBuffer::AddCommand(const Command& cmd)
{
	m_Commands.push_back(cmd);
}

_bool CInputBuffer::CheckCommand(ECommand eCmd)
{
	for (auto& cmd : m_Commands)
		if (cmd.type == eCmd) return true;
	return false;
}

_bool CInputBuffer::CheckCombo(const vector<ECommand>& seq)
{
	if (m_Commands.size() < seq.size()) return false;
	// 순서 및 시간 간격 검사
	for (size_t i = 0; i < seq.size(); ++i)
		if (m_Commands[m_Commands.size() - seq.size() + i].type != seq[i])
			return false;
	float dt = m_Commands.back().timestamp
		- m_Commands[m_Commands.size() - seq.size()].timestamp;
	return dt <= m_fTime;
}

void CInputBuffer::PopCommand(ECommand type, _int iCount)
{
	for (auto it = m_Commands.begin(); it != m_Commands.end(); )
	{
		if (it->type == type)
		{
			if (iCount != -1)
			{
				iCount--;
			}
			it = m_Commands.erase(it);

		}
		else
		{
			if (iCount == 0) break; // iCount가 0이면 더 이상 제거하지 않음
			++it;
		}
	}
}

CInputBuffer* CInputBuffer::Create()
{
	return new CInputBuffer();
}

void CInputBuffer::Free()
{
	__super::Free();
	Safe_Release(m_pGameInstance);
	m_Commands.clear();
}
