#pragma once
#include "Pannel.h"
#include <JsonLoader.h>
BEGIN_NAMESPACE(Engine)
class CGameObject;
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

	CGameObject* ClonePrototype(const string& prototypeName, const wstring& instanceName);

private:
	//unordered_map<string, CGameObject*> m_PrototypeMap;
	unordered_set<string> m_PrototypeSet;
	unordered_map<string, _uint> m_LevelStringMap;
	string           m_CurrentPrototype;
	_uint	         m_iCurrentSelectedLevel{ 0 };
	vector<_wstring> m_ShaderKeys;
	vector<_wstring> m_TextureKeys;
	vector<map<const _wstring, class CBase*>> m_pPrototypes;
	_char         m_NameBuf[128] = "";

	_char    m_FilePathBuf[260];   // 저장/로딩할 파일 경로
	CJsonLoader  m_JsonLoader;    // JsonLoader 인스턴스

public:
	static CToolbar* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};
END_NAMESPACE

