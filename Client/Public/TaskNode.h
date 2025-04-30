#pragma once
#include "Node.h"
class TaskNode : public INode
{
public:
    TaskNode(const _wstring& name, function<NodeStatus()> func) : taskName(name), taskFunc(func) {}
    NodeStatus Execute() override
    {
        if (taskFunc)
        {
            return taskFunc();
        }
        return NodeStatus::FAIL;
    }
private:
    _wstring taskName;
    function<NodeStatus()> taskFunc;
};
