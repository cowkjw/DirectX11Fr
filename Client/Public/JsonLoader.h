#pragma once
#include "Client_Defines.h"
#include "Base.h"
BEGIN(Engine)
class CGameInstance;
END

BEGIN(Client)
class CJsonLoader final : public CBase
{
public:
    _int CountPrototypes(const string& filePath) const;
    HRESULT Load_Prototypes(  const string& filePath, function<void()> onEntryLoaded);

private:
	CGameInstance* m_pGameInstance = { nullptr };   

public:
    virtual void Free() override;
};
END
