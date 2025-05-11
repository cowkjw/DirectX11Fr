#pragma once
#include "Base.h"
#include "Client_Defines.h"

BEGIN_NAMESPACE(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
class CUIObject;
class CGameInstance;
class CUICanvas;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class CUIController final : public CBase
{
private:
	CUIController(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CUIController() = default;

public:
	HRESULT Initialize();
	void Update(_float fTimeDelta);
	HRESULT Render();

private:
	void DrawHierarchy();      // 씬 그래프
	void DrawInspector();      // 선택된 오브젝트 속성 편집
	void DrawToolbar();

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	CGameInstance* m_pGameInstance = { nullptr };
	vector<CUIObject*> m_vecSceneObjects; // 씬에 있는 오브젝트들
	CUIObject* m_pSelectedObject = { nullptr }; // 선택된 오브젝트
	_bool m_bChangedObject{ false };
	vector<CUICanvas*> m_vCanvases;
	CUICanvas* m_pSelectedCanvas{ nullptr };

public:
	static CUIController* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;

};
END_NAMESPACE

