#pragma once

#include "Base.h"

BEGIN_NAMESPACE(Engine)

class CTarget_Manager final : public CBase
{
private:
	CTarget_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CTarget_Manager() = default;

public:
	HRESULT Initialize();
	HRESULT Add_RenderTarget(const _wstring& strTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT ePixelFormat, const _float4& vClearColor);
	HRESULT Add_MRT(const _wstring& strMRTTag, const _wstring& strTargetTag);
	HRESULT Remove_RenderTarget(const _wstring& strTargetTag);
	HRESULT Remove_RenderTargetGroup(const _wstring& strMRTTag);
	ID3D11ShaderResourceView* Get_RenderTargetSRV(const _wstring& strTargetTag);

	HRESULT Begin_MRT(const _wstring& strMRTTag, ID3D11DepthStencilView* pDSV, _bool isTargetClear, _bool isDepthClear);
	HRESULT End_MRT();
	HRESULT Bind_ShaderResource(const _wstring& strTargetTag, class CShader* pShader, const _char* pContantName);

#ifdef _DEBUG
public:
	HRESULT Ready_Debug(const _wstring& strTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY);
	HRESULT Render_Debug(const _wstring& strMRTTag, class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
#endif

private:
	class CRenderTarget* Find_RenderTarget(const _wstring& strTargetTag);
	vector<class CRenderTarget*>* Find_MRT(const _wstring& strMRTTag);

private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };

	ID3D11RenderTargetView* m_pBackBuffer = { nullptr };
	ID3D11DepthStencilView* m_pOriginalDSV = { nullptr };

	unordered_map<_wstring, class CRenderTarget*>		m_RenderTargets;
	unordered_map<_wstring, vector<class CRenderTarget*>>	m_MRTs;
	unordered_map<_wstring, _uint>	m_MRTsIdxMap;



public:
	static CTarget_Manager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END_NAMESPACE