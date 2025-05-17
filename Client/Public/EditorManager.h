#pragma once
#include "Base.h"
#include "Client_Defines.h"
#include "InspectorPannel.h"
#include "Hierarchy.h"
#include "Toolbar.h"

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
	friend class CInspectorPannel;
	friend class CHierarchy;
	friend class CToolbar;
	friend class CJsonLoader;
private:
	CEditorManager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CEditorManager() = default;

public:
	HRESULT Initialize();
	void Update(_float fTimeDelta);
	HRESULT Render();
private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	CGameInstance* m_pGameInstance = { nullptr };
	static vector<CGameObject*> m_vecSceneObjects; // 씬에 있는 오브젝트들
	static CGameObject* m_pSelectedObject;
	static _bool m_bOrthoGizmo;
	vector<CPannel*> m_vecPannels;

public:
	static CEditorManager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;

};
END_NAMESPACE

