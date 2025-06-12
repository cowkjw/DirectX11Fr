#pragma once
#include "Pannel.h"
#include <JsonLoader.h>
BEGIN_NAMESPACE(Engine)
class CGameObject;
class CAnimation;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class CToolbar : public CPannel
{
private:
	CToolbar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	~CToolbar() = default;
public:
	HRESULT Initialize();
	void Update(_float fTimeDelta);
	HRESULT Render();
private:
	void DrawToolbar();

	void RegisterDefaultPrototypes();
	void SetLevelEnumToString();
	void UpdatePrototypeList();
	void Get_PrototypeList();
	void FBXLodaer();
	void DrawAnimEventEditor();

	CGameObject* ClonePrototype(const string& prototypeName, const wstring& instanceName,void* pArg = nullptr);

private:
	//unordered_map<string, CGameObject*> m_PrototypeMap;
	unordered_set<string> m_PrototypeSet;
	unordered_map<string, _uint> m_LevelStringMap;
	string           m_CurrentPrototype;
	_uint	         m_iCurrentSelectedLevel{ 0 };
	vector<_wstring> m_ShaderKeys;
	vector<_wstring> m_TextureKeys;
	vector<_wstring> m_ModelKeys;
	vector<map<const _wstring, class CBase*>> m_pPrototypes;
	_char         m_NameBuf[128] = "";

	_char    m_FilePathBuf[260];   // 저장/로딩할 파일 경로
	CJsonLoader*  m_JsonLoader = nullptr;    // JsonLoader 인스턴스

	mutex m_FbxLoadMutex;
	string m_ModelKey;

	vector<_wstring> m_FbxFilePaths;
public:
	static CToolbar* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};
END_NAMESPACE

