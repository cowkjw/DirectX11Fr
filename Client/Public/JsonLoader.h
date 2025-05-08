#pragma once
#include "Client_Defines.h"
#include "Base.h"
BEGIN_NAMESPACE(Engine)
class CGameInstance;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class CJsonLoader final : public CBase
{
public:
	CJsonLoader();
	virtual ~CJsonLoader() = default;
public:
    _int CountPrototypes(const string& filePath) const;
    HRESULT Load_Prototypes(  const string& filePath, function<void()> onEntryLoaded);
	HRESULT Load_Shaders(const string& filePath, function<void()> onEntryLoaded);
	HRESULT Load_Textures(const string& filePath, function<void()> onEntryLoaded);

private:
	CGameInstance* m_pGameInstance = { nullptr };   

public:
    virtual void Free() override;
};
END_NAMESPACE
