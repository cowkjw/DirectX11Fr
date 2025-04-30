#pragma once
#include <Client_Defines.h>
#include "Node.h"
// 추상
class CCompositeNode : public INode // Select , Sequence 에 사용
{
public:
    virtual ~CCompositeNode() { Release(); }
public:
    void AddChild(INode* child) { m_vecChild.push_back(child); }
protected:
    void Release()
    {
        for (auto& node : m_vecChild)
        {
            Safe_Delete<INode*>(node);
        }
        m_vecChild.clear();
    }
protected:
    vector<INode*> m_vecChild;
};

class CSequenceNode : public CCompositeNode // 계속 진행하는 하나라도 FAIL이면 진행 안함
{
public:
    NodeStatus Execute() override
    {
        for (auto& child : m_vecChild)
        {
            NodeStatus status = child->Execute();
            if (status == NodeStatus::FAIL) // 하나라도 실패면 즉시 종료
            {
                return NodeStatus::FAIL;
            }
            if (status == NodeStatus::RUNNING) // 실행 중인 노드가 있으면 RUNNING 반환
            {
                return NodeStatus::RUNNING;
            }
        }
        return NodeStatus::SUCCESS; // 모든 노드가 성공하면 SUCCESS 반환
    }
};

class CSelectorNode : public CCompositeNode  // 성공한 노드가 있으면 반환해서 실행 시킴
{
public:
    NodeStatus Execute() override
    {
        for (auto& child : m_vecChild)
        {
            NodeStatus status = child->Execute();
            if (status == NodeStatus::SUCCESS) // 하나라도 성공하면 즉시 성공 반환
            {
                return NodeStatus::SUCCESS;
            }
            if (status == NodeStatus::RUNNING) // 실행 중이면 RUNNING 반환
            {
                return NodeStatus::RUNNING;
            }
        }
        return NodeStatus::FAIL; // 모든 노드가 실패하면 FAIL 반환
    }
};

class CParallelNode : public CCompositeNode
{
public:

    NodeStatus Execute() override
    {
        _bool bAnyRunning = false;
        for (auto& child : m_vecChild)
        {
            NodeStatus status = child->Execute();
            if (status == NodeStatus::RUNNING)
            {
                bAnyRunning = true;
            }
        }
        return bAnyRunning ? NodeStatus::RUNNING : NodeStatus::SUCCESS;
    }
};