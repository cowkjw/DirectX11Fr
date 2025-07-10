#pragma once

#include "Base.h"

BEGIN_NAMESPACE(Engine)

class CRenderer final : public CBase
{
private:
	CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CRenderer() = default;
public:
	HRESULT Add_RenderGroup(RENDERGROUP eRenderGroup, class CGameObject* pRenderObject);
	HRESULT Draw();
	void Clear();

private:
	HRESULT Initialize();

private:
	list<class CGameObject*>	m_RenderObjects[ToIndex(RENDERGROUP::END)];
	_float4x4 				m_ViewMatrix{}, m_ProjMatrix{}, m_WorldMatrix{};
	
	class CVIBuffer_Rect* m_pVIBuffer = { nullptr }; // 후처리용 화면 전체에 그릴 사각형 버퍼
	class CShader* m_pShader = { nullptr }; // 후처리용 쉐이더


	ID3D11RenderTargetView* m_pBackBuffer = { nullptr };
	ID3D11DepthStencilView* m_pOriginalDSV = { nullptr };
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };

private:
	HRESULT Render_Priority();
	HRESULT Render_NonBlend();
	HRESULT Render_Blend();
	HRESULT Render_Lights();
	HRESULT Render_BackBuffer();
	HRESULT Render_Distortion();
	HRESULT Render_ToonBackBuffer();
	HRESULT Render_NonLight();
	HRESULT Render_UI();
#ifdef _DEBUG
	HRESULT Render_Debug();
#endif

public:
	static CRenderer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END_NAMESPACE