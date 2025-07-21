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

	void Set_Color(const _float4& vColor)
	{
		m_vColor = vColor;
	}
	void SetFogDistance(_float fStart,_float fEnd)
	{
		m_fFogStart = fStart;
		m_fFogEnd = fEnd;
	}
	void Active_Fog(_bool bActive)
	{
		m_bRenderFog = bActive;
	}

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


	ID3D11DepthStencilView* m_pShadowDSV = { nullptr };
	_uint					m_iOriginalViewportWidth{}, m_iOriginalViewportHeight{};

private:
	HRESULT Render_Priority();
	HRESULT Render_NonBlend();
	HRESULT Render_Fog();
	HRESULT Render_Blend();
	HRESULT Render_Lights();
	HRESULT Render_Shadow();
	HRESULT Render_Distortion();
	HRESULT Render_Bloom();
	HRESULT Render_BlurEffect();
	HRESULT Render_BloomEffect();
	HRESULT Render_RawEffect();
	HRESULT Render_ToonBackBuffer();
	HRESULT Render_Bright();
	HRESULT Render_Blur();
	HRESULT Render_Final();
	HRESULT Render_NonLight();
	HRESULT Render_UI();
#ifdef _DEBUG
	HRESULT Render_Debug();
#endif

	HRESULT Ready_RenderTargets(const D3D11_VIEWPORT& viewPort);
	HRESULT Ready_DepthStencilView(_uint iWidth, _uint iHeight);
	HRESULT Change_ViewportDesc(_uint iWidth, _uint iHeight);

private:
	_float m_fFogStart{}; // 안개 시작 거리
	_float m_fFogEnd{};
	_float4 m_vColor;
	_bool m_bRenderFog = false;
public:
	static CRenderer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END_NAMESPACE