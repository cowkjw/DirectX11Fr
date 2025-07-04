#include "MySequence.h"
#include "Toolbar.h"

void CMySequence::Add(_int type)
{
    SequenceItem item;
    item.pPS = nullptr;
    item.start = 0;
    item.end = 60;
    item.type = type;
    item.color = 0xFF00CCFF;
    item.name = "NewEffect";
    m_items.push_back(item);
}

void CMySequence::Del(_int index)
{
	if (index < 0 || index >= (_int)m_items.size())
		return;
	Safe_Release(m_items[index].pPS); // 파티클 시스템 해제
	m_items.erase(m_items.begin() + index);
    if (m_pToolbar)
    {
		m_pToolbar->RemoveSequence(index); // 툴바에서 해당 시퀀스 제거
    }
}
