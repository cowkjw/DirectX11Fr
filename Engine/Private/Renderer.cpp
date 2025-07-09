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
	_uint				iNumViewports = { 1 };
	D3D11_VIEWPORT		ViewportDesc{};

	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);

#pragma region Create Render Targets
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Diffuse"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_B8G8R8A8_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Normal"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.5f, 0.5f, 1.0f, 1.0f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Shade"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Depth"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
#pragma endregion

#pragma region Create MRTs
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_Diffuse"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_Normal"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_Depth"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Lights"), TEXT("Target_Shade"))))
		return E_FAIL;
#pragma endregion
	m_pVIBuffer = CVIBuffer_Rect::Create(m_pDevice, m_pContext);
	if (nullptr == m_pVIBuffer)
		return E_FAIL;

	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Asset/ShaderFiles/Shader_Deferred.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;

	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixScaling(ViewportDesc.Width, ViewportDesc.Height, 1.f));
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(ViewportDesc.Width, ViewportDesc.Height, 0.0f, 1.f));


	//if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Diffuse"), 100.0f, 100.0f, 200.0f, 200.0f)))
	//	return E_FAIL;
	//if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Normal"), 100.0f, 300.0f, 200.0f, 200.0f)))
	//	return E_FAIL;
	//if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Depth"), 100.0f, 500.0f, 200.0f, 200.0f)))
	//	return E_FAIL;
	//if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Shade"), 300.f, 100.0f, 200.0f, 200.0f)))
	//	return E_FAIL;
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
	if (FAILED(Render_Lights()))
		return E_FAIL;
	//if (FAILED(Render_BackBuffer()))
	//	return E_FAIL;
	if (FAILED(Render_ToonBackBuffer()))
		return E_FAIL;
	if (FAILED(Render_NonLight()))
		return E_FAIL;
	if (FAILED(Render_Blend()))
		return E_FAIL;
	if (FAILED(Render_UI()))
		return E_FAIL;
	
#ifdef _DEBUG
	if (FAILED(Render_Debug()))
		return E_FAIL;

#endif
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
	// 디퓨즈랑 노말 같이 계산해서 텍스쳐 만듦
	m_pGameInstance->Begin_MRT(TEXT("MRT_GameObjects"));
	for (auto& pGameObject : m_RenderObjects[ToIndex(RENDERGROUP::NONBLEND)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();

	}
	m_RenderObjects[ToIndex(RENDERGROUP::NONBLEND)].clear();

	m_pGameInstance->End_MRT();
	return S_OK;
}

HRESULT CRenderer::Render_Lights()
{
	// 쉐이드 처리
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Lights"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Normal"), m_pShader, "g_NormalTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Depth"), m_pShader, "g_DepthTexture")))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInv", m_pGameInstance->Get_Transform_Float4x4_Inv(TRANSFORM::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInv", m_pGameInstance->Get_Transform_Float4x4_Inv(TRANSFORM::PROJECTION))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_RawValue("g_vCamPosition", m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;
	_float fCameraFar = m_pGameInstance->Get_CameraFar();
	if (FAILED(m_pShader->Bind_RawValue("g_fCameraFar", &fCameraFar, sizeof(_float))))
		return E_FAIL;

	m_pGameInstance->Render_Lights(m_pShader, m_pVIBuffer);

	// 백버퍼로 복구
	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_BackBuffer()
{
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Diffuse"), m_pShader, "g_DiffuseTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Shade"), m_pShader, "g_ShadeTexture")))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	m_pShader->Begin(3);

	m_pVIBuffer->Bind_Buffers();
	m_pVIBuffer->Render();
	return S_OK;
}

HRESULT CRenderer::Render_ToonBackBuffer()
{
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Diffuse"), m_pShader, "g_DiffuseTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Shade"), m_pShader, "g_ShadeTexture")))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;
	//if (FAILED(m_pShader->Bind_SRV("g_DepthTexture", m_pGameInstance->GetSceneViewSRV())))
	//	return E_FAIL;

	m_pShader->Begin(4);

	m_pVIBuffer->Bind_Buffers();
	m_pVIBuffer->Render();
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

HRESULT CRenderer::Render_NonLight()
{
	for (auto& pGameObject : m_RenderObjects[ToIndex(RENDERGROUP::NONLIGHT)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();
	}
	m_RenderObjects[ToIndex(RENDERGROUP::NONLIGHT)].clear();

	return S_OK;
}


HRESULT CRenderer::Render_UI()
{
	for (auto& pGameObject : m_RenderObjects[ToIndex(RENDERGROUP::UI)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();
	}
	m_RenderObjects[ToIndex(RENDERGROUP::UI)].clear();

	return S_OK;
}

HRESULT CRenderer::Render_Debug()
{
	m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix);
	m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix);

	m_pGameInstance->Render_MRT_Debug(TEXT("MRT_GameObjects"), m_pShader, m_pVIBuffer);
	m_pGameInstance->Render_MRT_Debug(TEXT("MRT_Lights"), m_pShader, m_pVIBuffer);
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
	Safe_Release(m_pVIBuffer);
	Safe_Release(m_pShader);
	Safe_Release(m_pBackBuffer);
	Safe_Release(m_pOriginalDSV);

	for (auto& ObjectList : m_RenderObjects)
	{
		for (auto& pGameObject : ObjectList)
			Safe_Release(pGameObject);
		ObjectList.clear();
	}		
}
