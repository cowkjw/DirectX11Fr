#pragma once
#include "Client_Defines.h"
template<typename MsgT>
class IObserver
{
public:
    
    virtual ~IObserver() = default;
    virtual void Notify(MsgT const& msg) = 0;
};

