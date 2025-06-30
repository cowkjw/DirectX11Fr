#pragma once
#include "Client_Defines.h"
#include "Base.h"
BEGIN_NAMESPACE(Engine)
class CGameInstance;
class CGameObject;
class CUIObject;
class CAnimation;
class CParticleSystem;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class CJsonLoader final : public CBase
{
public:
	CJsonLoader();
	CJsonLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CJsonLoader() = default;
public:
    _int CountPrototypes(const string& filePath) const;
    HRESULT Load_Prototypes(  const string& filePath, function<void()> onEntryLoaded);
	HRESULT Load_Shaders(const string& filePath, function<void()> onEntryLoaded);
	HRESULT Load_Textures(const string& filePath, function<void()> onEntryLoaded);
	HRESULT Load_Models(const string& filePath,function<void()> onEntryLoaded);
	HRESULT Load_Objects(const string& filePath, function<void()> onEntryLoaded);
	HRESULT Load_Particle(const string& filePath, CParticleSystem** ppParticle);
	HRESULT Load_CutSceneCamera(const string& filePath, CGameObject* pCamera);
	json	Load_CutScene_PropertyAsJson(const string& filePath);

	HRESULT Save_Objects(const string& filePath, function<void()> onEntryLoaded);
	HRESULT Save_Particle(const string& filePath,CParticleSystem* pParticle);
	HRESULT Save_CutSceneCamera(const string& filePath, CGameObject* pCamera);


	HRESULT LoadAnimEvent(const string& filePath,vector<CAnimation*>& animations);

private:
	void CollectAllObjects(CGameObject* root,vector<CGameObject*>& out);
	void FactoryComponent(CGameObject* pObj, const json& j);
	void FactoryUI(CUIObject** pObjOut, const json& j);
private:
	CGameInstance* m_pGameInstance = { nullptr };   
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };

public:
    virtual void Free() override;
};
END_NAMESPACE
