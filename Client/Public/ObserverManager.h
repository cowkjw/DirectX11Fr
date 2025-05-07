#pragma once
#include "Base.h"	
#include "Client_Defines.h"	
#include "Observer.h"

BEGIN_NAMESPACE(Client)
template<typename MsgT>
class CObserverManager final : public CBase
{
private:
	CObserverManager() = default;
	virtual ~CObserverManager() = default;

    CObserverManager(const CObserverManager&) = delete;
    CObserverManager& operator=(const CObserverManager&) = delete;
public:
    static CObserverManager& Get_Instance()
    {
        static CObserverManager instance;
        return instance;
    }

    void Subscribe(const string& key, IObserver<MsgT>* obs)
    {
        m_mapObservers[key].push_back(obs);
    }

    void Unsubscribe(const string& key, IObserver<MsgT>* obs)
    {
        auto& vec = m_mapObservers[key];
        vec.erase(remove(vec.begin(), vec.end(), obs), vec.end());
    }

    void Notify(const string& key, MsgT const& msg)
    {
        for (auto* obs : m_mapObservers[key])
            obs->Notify(msg);
    }

private:
   unordered_map<string,vector<IObserver<MsgT>*>> m_mapObservers;
public:
    virtual void Free() override
    {
		for (auto& pair : m_mapObservers)
		{
			pair.second.clear();
		}
		m_mapObservers.clear();
    }
};
END_NAMESPACE