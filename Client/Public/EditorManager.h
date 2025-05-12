#pragma once
#include "Base.h"
#include "Client_Defines.h"

BEGIN_NAMESPACE(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
class CGameObject;
class CGameInstance;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class CEditorManager final : public CBase
{
private:
	CEditorManager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CEditorManager() = default;

public:
	HRESULT Initialize();
	void Update(_float fTimeDelta);
	HRESULT Render();

private:
	void DrawHierarchy();      // 씬 그래프
	void DrawChildHierarchy(CGameObject* parent);
	void DrawInspector();      // 선택된 오브젝트 속성 편집
	void DrawToolbar();

	void EraseFromVector(CGameObject* pObj);


	void RegisterDefaultPrototypes();
	CGameObject* ClonePrototype(const string& name);


	void SetLevelEnumToString();

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	CGameInstance* m_pGameInstance = { nullptr };
	vector<CGameObject*> m_vecSceneObjects; // 씬에 있는 오브젝트들
	CGameObject* m_pSelectedObject = { nullptr }; // 선택된 오브젝트
	_bool m_bChangedObject{ false };

	unordered_map<string, CGameObject*> m_PrototypeMap;
	string                           m_CurrentPrototype;

	

	vector<_wstring> m_ShaderKeys;
	vector<_wstring> m_TextureKeys;
	_uint m_iShaderKeyIndex{ 0UL };
	_uint m_iTextureKeyIndex{ 0UL };

	unordered_map<string,_uint> m_LevelStringMap;

	_bool m_bOrthoGizmo = { false };

public:
	static CEditorManager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;

};
END_NAMESPACE

