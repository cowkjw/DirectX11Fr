#pragma once
#include "Client_Defines.h"
#include "Base.h"
BEGIN_NAMESPACE(Engine)
class CGameInstance;
class CGameObject;
class CUIObject;
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
	HRESULT Load_Models(const string& filePath,function<void()> onEntryLoaded);
	HRESULT Load_Objects(const string& filePath, function<void()> onEntryLoaded);
	HRESULT Save_Objects(const string& filePath, function<void()> onEntryLoaded);

private:
	void CollectAllObjects(CGameObject* root,vector<CGameObject*>& out);
	void FactoryComponent(CGameObject* pObj, const json& j);
	void FactoryUI(CUIObject** pObjOut, const json& j);
private:
	CGameInstance* m_pGameInstance = { nullptr };   

public:
    virtual void Free() override;
};
END_NAMESPACE
