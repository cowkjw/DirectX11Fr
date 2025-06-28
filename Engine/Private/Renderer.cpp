#include "Renderer.h"
#include "GameObject.h"
#include "GameInstance.h"


CRenderer::CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
	
}

HRESULT CRenderer::Initialize()
{
	//D3D11_DEPTH_STENCIL_DESC dsDesc{};
	//dsDesc.DepthEnable = FALSE;                         // ±íÀÌ Å×½ºÆ® ²û
	//dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;    // ±íÀÌ ¹öÆÛ¿¡ ¾²±â ²û
	//dsDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;        // Ç×»ó Å×½ºÆ® Åë°ú
	//dsDesc.StencilEnable = FALSE;                         // ½ºÅÙ½Ç ²¨µµ ¹«¹æ

	//HRESULT hr = m_pDevice->CreateDepthStencilState(&dsDesc, &m_pNoDepthState);
	//if (FAILED(hr))
	//	return hr;

	_uint				iNumViewports = { 1 };
	D3D11_VIEWPORT		ViewportDesc{};

	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);

	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Diffuse"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_B8G8R8A8_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_Diffuse"))))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Add_RenderGroup(RENDERGROUP eRenderGroup, CGameObject* pRenderObject)
{
	if (eRenderGroup >=	RENDERGROUP::END ||
		nullptr == pRenderObject)
		return E_FAIL;

	m_RenderObjects[ToIndex(eRenderGroup)].push_back(pRenderObject);

	return S_OK;
}

HRESULT CRenderer::Draw()
{
	if (FAILED(Render_Priority()))
		return E_FAIL;
	if (FAILED(Render_NonBlend()))
		return E_FAIL;
	if (FAILED(Render_Blend()))
		return E_FAIL;
	if (FAILED(Render_UI()))
		return E_FAIL;
	

	return S_OK;
}

void CRenderer::Clear()
{
	for (auto& ObjectList : m_RenderObjects)
	{
		ObjectList.clear();
	}
}

HRESULT CRenderer::Render_Priority()
{
	for (auto& pGameObject : m_RenderObjects[ToIndex(RENDERGROUP::PRIORITY)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();
	}
	m_RenderObjects[ToIndex(RENDERGROUP::PRIORITY)].clear();

	return S_OK;
}

HRESULT CRenderer::Render_NonBlend()
{
	//m_pGameInstance->Begin_MRT(TEXT("MRT_GameObjects"));
	for (auto& pGameObject : m_RenderObjects[ToIndex(RENDERGROUP::NONBLEND)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();

	}
	m_RenderObjects[ToIndex(RENDERGROUP::NONBLEND)].clear();

	//m_pGameInstance->End_MRT();
	return S_OK;
}

HRESULT CRenderer::Render_Blend()
{

	for (auto& pGameObject : m_RenderObjects[ToIndex(RENDERGROUP::BLEND)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();
	}
	m_RenderObjects[ToIndex(RENDERGROUP::BLEND)].clear();

	return S_OK;
}

HRESULT CRenderer::Render_UI()
{
	//m_pContext->OMSetDepthStencilState(m_pNoDepthState, 0);
	for (auto& pGameObject : m_RenderObjects[ToIndex(RENDERGROUP::UI)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();
	}
	//m_pContext->OMSetDepthStencilState(nullptr, 0);
	m_RenderObjects[ToIndex(RENDERGROUP::UI)].clear();

	return S_OK;
}

CRenderer* CRenderer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CRenderer* pInstance = new CRenderer(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created CRenderer");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CRenderer::Free()
{
	__super::Free();

	Safe_Release(m_pGameInstance);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	//m_pContext->OMSetDepthStencilState(nullptr, 0);
	//Safe_Release(m_pNoDepthState);

	for (auto& ObjectList : m_RenderObjects)
	{
		for (auto& pGameObject : ObjectList)
			Safe_Release(pGameObject);
		ObjectList.clear();
	}		
}
