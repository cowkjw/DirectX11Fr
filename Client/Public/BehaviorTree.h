#pragma once
#include "CompositeNode.h"
#include "TaskNode.h"

class CBehaviorTree 
{
public:
	~CBehaviorTree()
	{
		Safe_Delete(m_tRoot);
	}

public:
	void Run()
	{
		if (m_tRoot)
		{
			m_tRoot->Execute();
		}
	}
	void Set_Root(INode* root) { m_tRoot = root; }
private:
	INode* m_tRoot;
};

