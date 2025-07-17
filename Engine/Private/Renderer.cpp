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

	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Normal"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_UNORM, _float4(1.0f, 1.0f, 1.0f, 1.0f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Shade"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Depth"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Specular"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	// 디스토션은 중립값으로 클리어 시키기 0.5 0.5 그래야지 다른 애들한테도 영향이 안받음
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Distortion"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.5f, 0.5f, 0.f, 0.f))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_DistortionObj"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	// 블러용 이펙트만 찍기
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_BlurEffect"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f,0.f,0.f,0.f))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_BloomEffect"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_BloomBlurX"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_BloomBlurY"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Final"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.0f, 0.0f, 0.0f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_BlurX"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_BlurY"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	// 블룸용으로 찍을 애들만 꺼내찍기
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Bright"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0, 0, 0, 0))))
		return E_FAIL;

	// 블룸 그리기
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Bloom"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Effect"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f,0.f,0.f,1.f))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_RimLight"),ViewportDesc.Width, ViewportDesc.Height,DXGI_FORMAT_R16G16B16A16_FLOAT,_float4(0, 0, 0, 0))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Fog"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R8_UNORM, _float4(0, 0, 0, 0))))
		return E_FAIL;


	// 그림자

	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Shadow"), g_iMaxWidth, g_iMaxHeight, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(1.0f, 1.0f, 1.0f, 1.0f))))
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
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Lights"), TEXT("Target_Specular"))))
		return E_FAIL;

	//if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Effects"), TEXT("Target_Distortion"))))
	//	return E_FAIL;
	// 이펙트만 먼저 찍어두고
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Effects"), TEXT("Target_Effect"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Distortion"), TEXT("Target_DistortionObj"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Distortion"), TEXT("Target_Distortion"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_BlurEffect"), TEXT("Target_BlurEffect"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Bright"), TEXT("Target_Bright"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Bloom"), TEXT("Target_Bloom"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_BloomEffect"), TEXT("Target_BloomEffect"))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Final"), TEXT("Target_Final"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_BlurX"), TEXT("Target_BlurX"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_BlurY"), TEXT("Target_BlurY"))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_BloomBlurX"), TEXT("Target_BloomBlurX"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_BloomBlurY"), TEXT("Target_BloomBlurY"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_RimLight"), TEXT("Target_RimLight"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_ShadowObjects"), TEXT("Target_Shadow"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_FogObjects"), TEXT("Target_Fog"))))
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


	m_iOriginalViewportWidth = ViewportDesc.Width;
	m_iOriginalViewportHeight = ViewportDesc.Height;

	if (FAILED(Ready_DepthStencilView(g_iMaxWidth, g_iMaxHeight)))
		return E_FAIL;
	//if(FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Diffuse"), 100.0f, 100.0f, 200.0f, 200.0f)))
	//	return E_FAIL;
	//if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Normal"), 300.0f, 100.0f, 200.0f, 200.0f)))
	//	return E_FAIL;
	//if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Depth"), 500.0f, 100.0f, 200.0f, 200.0f)))
	//	return E_FAIL;
	//if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_BlurEffect"), 700.0f, 100.0f, 200.0f, 200.0f)))
	//	return E_FAIL;
	//if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Bloom"), 900.0f, 100.0f, 200.0f, 200.0f)))
	//	return E_FAIL;
	//if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Shade"), 1100.f, 100.0f, 200.0f, 200.0f)))
	//	return E_FAIL;

	//if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Shadow"), 100.f, 400.0f, 300.0f, 300.0f)))
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
	/*if (FAILED(Render_Shadow()))
		return E_FAIL;*/

	if (FAILED(Render_NonBlend())) // 맵, 캐릭터 그리고
		return E_FAIL;
	if (FAILED(Render_Lights())) // 조명 계산함
		return E_FAIL;
	if (FAILED(Render_Distortion()))
		return E_FAIL;
	//if (FAILED(Render_BackBuffer()))
	//	return E_FAIL;
	if (FAILED(Render_ToonBackBuffer())) // 조명 계산하고 잠깐 그려두고
		return E_FAIL;

	//if (FAILED(Render_RimLight())) // 림라이트 따로 그림
	//	return E_FAIL;

	if (FAILED(Render_RawEffect())) // 이펙트들 따로 그리고
		return E_FAIL;
	if (FAILED(Render_BloomEffect())) // 블룸 이펙트들 따로 그리고
		return E_FAIL;
	if (FAILED(Render_BlurEffect())) // 블러 이펙트 따로 그림
		return E_FAIL;


	if (FAILED(Render_Bloom()))
		return E_FAIL;
	if (FAILED(Render_Blur()))
		return E_FAIL;
	if (FAILED(Render_Final())) // 블룸이랑 블러랑 합쳐서 최종 화면에 그리기
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
	m_pGameInstance->Begin_MRT(TEXT("MRT_Final"));
	for (auto& pGameObject : m_RenderObjects[ToIndex(RENDERGROUP::PRIORITY)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();
	}
	m_RenderObjects[ToIndex(RENDERGROUP::PRIORITY)].clear();
	m_pGameInstance->End_MRT();
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

HRESULT CRenderer::Render_Fog()
{
	if (FAILED(m_pShader->Bind_RawValue("g_vCamPosition", m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;
	_float fCameraFar = m_pGameInstance->Get_CameraFar();
	if (FAILED(m_pShader->Bind_RawValue("g_fCameraFar", &fCameraFar, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_FogColor", &m_vColor, sizeof(_float3))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_FogStart", &m_fFogStart, sizeof(_float))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_RawValue("g_FogEnd", &m_fFogEnd, sizeof(_float))))
		return E_FAIL;

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

HRESULT CRenderer::Render_Shadow()
{
	m_pGameInstance->Begin_MRT(TEXT("MRT_ShadowObjects"), m_pShadowDSV, true,false);

	if (FAILED(Change_ViewportDesc(g_iMaxWidth, g_iMaxHeight)))
		return E_FAIL;

	for (auto& pGameObject : m_RenderObjects[ToIndex(RENDERGROUP::SHADOW)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render_Shadow();
	}
	m_RenderObjects[ToIndex(RENDERGROUP::SHADOW)].clear();

	m_pGameInstance->End_MRT();

	if (FAILED(Change_ViewportDesc(m_iOriginalViewportWidth, m_iOriginalViewportHeight)))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_Distortion()
{
	m_pGameInstance->Begin_MRT(TEXT("MRT_Distortion"));
	for (auto& pGameObject : m_RenderObjects[ToIndex(RENDERGROUP::DISTORTION)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();

	}
	m_RenderObjects[ToIndex(RENDERGROUP::DISTORTION)].clear();

	m_pGameInstance->End_MRT();

	return S_OK;
}

HRESULT CRenderer::Render_Bloom()
{
	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;
	if (FAILED(Render_Bright()))  // 블룸용으로 밝은 부분만 따로 뽑아냄
		return E_FAIL;

	m_pGameInstance->Begin_MRT(TEXT("MRT_BloomBlurX"));

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_BloomEffect"), m_pShader, "g_BloomEffectTexture")))
		return E_FAIL;

	m_pShader->Begin(10);
	m_pVIBuffer->Bind_Buffers();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();


	m_pGameInstance->Begin_MRT(TEXT("MRT_BloomBlurY"));

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_BloomBlurX"), m_pShader, "g_BloomBlurXTexture")))
		return E_FAIL;

	m_pShader->Begin(11);
	m_pVIBuffer->Bind_Buffers();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();


	m_pGameInstance->Begin_MRT(TEXT("MRT_Bloom"));

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_BloomBlurY"), m_pShader, "g_BloomBlurYTexture")))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Bright"), m_pShader, "g_BrightTexture")))
		return E_FAIL;


	m_pShader->Begin(9);
	m_pVIBuffer->Bind_Buffers();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();

	return S_OK;
}

HRESULT CRenderer::Render_ToonBackBuffer()
{
	m_pGameInstance->Begin_MRT(TEXT("MRT_Final"), nullptr, false);

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Diffuse"), m_pShader, "g_DiffuseTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Shade"), m_pShader, "g_ShadeTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Depth"), m_pShader, "g_DepthTexture")))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Shadow"), m_pShader, "g_ShadowTexture")))
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

	if (FAILED(m_pShader->Bind_Matrix("g_LightViewMatrix", m_pGameInstance->Get_Light_ViewMatrix())))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_LightProjMatrix", m_pGameInstance->Get_Light_ProjMatrix())))
		return E_FAIL;

	_float fCamFar = m_pGameInstance->Get_CameraFar();

	if (FAILED(m_pShader->Bind_RawValue("g_fCameraFar", &fCamFar, sizeof _float)))
		return E_FAIL;

	m_pShader->Begin(4);

	m_pVIBuffer->Bind_Buffers();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();
	return S_OK;
}

HRESULT CRenderer::Render_RimLight()
{
	m_pGameInstance->Begin_MRT(TEXT("MRT_RimLight"));

	for (auto& pGameObject : m_RenderObjects[ToIndex(RENDERGROUP::RIMLIGHT)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();

	}
	m_RenderObjects[ToIndex(RENDERGROUP::RIMLIGHT)].clear();
	m_pGameInstance->End_MRT();

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInv", m_pGameInstance->Get_Transform_Float4x4_Inv(TRANSFORM::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInv", m_pGameInstance->Get_Transform_Float4x4_Inv(TRANSFORM::PROJECTION))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_RawValue("g_vCamPosition", m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;
	_float fCameraFar = m_pGameInstance->Get_CameraFar();
	if (FAILED(m_pShader->Bind_RawValue("g_fCameraFar", &fCameraFar, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_vCamPosition", m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;
	m_pShader->Begin(12);

	// 풀스크린 쿼드
	m_pVIBuffer->Bind_Buffers();
	m_pVIBuffer->Render();


	return S_OK;
}

HRESULT CRenderer::Render_Bright()
{
	m_pGameInstance->Begin_MRT(TEXT("MRT_Bright"));

	// 블룸용으로 찍어놨던 이펙트를 블룸 이펙트 텍스처로 넘김
	m_pGameInstance->Bind_RT_ShaderResource(
		TEXT("Target_BloomEffect"), m_pShader, "g_BloomEffectTexture");
	m_pShader->Begin(8);


	m_pVIBuffer->Bind_Buffers();
	m_pVIBuffer->Render();


	m_pGameInstance->End_MRT();

	return S_OK;
}

HRESULT CRenderer::Render_Blur()
{

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;
	
	m_pGameInstance->Begin_MRT(TEXT("MRT_BlurX"));

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_BlurEffect"), m_pShader, "g_BlurEffectTexture")))
		return E_FAIL;

	m_pShader->Begin(5);
	m_pVIBuffer->Bind_Buffers();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();


	m_pGameInstance->Begin_MRT(TEXT("MRT_BlurY"));

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_BlurX"), m_pShader, "g_BlurXTexture")))
		return E_FAIL;

	m_pShader->Begin(6);
	m_pVIBuffer->Bind_Buffers();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();


	return S_OK;
}

HRESULT CRenderer::Render_Final()
{
	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_BlurY"), m_pShader, "g_BlurYTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Final"), m_pShader, "g_FinalTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Bloom"), m_pShader, "g_BloomTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Effect"), m_pShader, "g_EffectTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_RimLight"), m_pShader, "g_RimLightTexture")))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Distortion"), m_pShader, "g_DistortionTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_DistortionObj"), m_pShader, "g_DistortionObjTexture")))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_RawValue("g_bFogEnabled", &m_bRenderFog, sizeof(_bool))))
		return E_FAIL;
	if (m_bRenderFog)
	{
		if (FAILED(m_pShader->Bind_RawValue("g_FogColor", &m_vColor, sizeof(_float4))))
			return E_FAIL;
		if (FAILED(m_pShader->Bind_RawValue("g_FogStart", &m_fFogStart, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShader->Bind_RawValue("g_FogEnd", &m_fFogEnd, sizeof(_float))))
			return E_FAIL;

		_float fCameraFar = m_pGameInstance->Get_CameraFar();
		if (FAILED(m_pShader->Bind_RawValue("g_fCameraFar", &fCameraFar, sizeof(_float))))
			return E_FAIL;
	}

	m_pShader->Begin(7);
	m_pVIBuffer->Bind_Buffers();
	m_pVIBuffer->Render();

	return S_OK;
}

HRESULT CRenderer::Render_BlurEffect()
{
	m_pGameInstance->Begin_MRT(TEXT("MRT_BlurEffect"));
	for (auto& pGameObject : m_RenderObjects[ToIndex(RENDERGROUP::BLUR_EFFECT)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();

	}
	m_RenderObjects[ToIndex(RENDERGROUP::BLUR_EFFECT)].clear();

	m_pGameInstance->End_MRT();


	
	return S_OK;
}

HRESULT CRenderer::Render_BloomEffect()
{
	m_pGameInstance->Begin_MRT(TEXT("MRT_BloomEffect"));

	for (auto& pGameObject : m_RenderObjects[ToIndex(RENDERGROUP::BLOOM_EFFECT)])
	{
		if (pGameObject)
			pGameObject->Render();
	}
	m_RenderObjects[ToIndex(RENDERGROUP::BLOOM_EFFECT)].clear();
	m_pGameInstance->End_MRT();
	return S_OK;
}

HRESULT CRenderer::Render_RawEffect()
{
	// 그냥 이펙트에 찍어두기
	m_pGameInstance->Begin_MRT(TEXT("MRT_Effects"));

	for (auto& pGameObject : m_RenderObjects[ToIndex(RENDERGROUP::EFFECT)])
	{
		if (pGameObject)
			pGameObject->Render();
	}
	m_RenderObjects[ToIndex(RENDERGROUP::EFFECT)].clear();
	m_pGameInstance->End_MRT();
	return S_OK;
}

HRESULT CRenderer::Render_Blend()
{
	m_RenderObjects[ToIndex(RENDERGROUP::BLEND)].sort([this](CGameObject* pSour, CGameObject* pDest)->_bool
{
		_float fDepth1{}, fDepth2{};
		if(pSour)
		{
			fDepth1 =  XMVectorGetX(XMVector3Length(XMLoadFloat4(m_pGameInstance->Get_CamPosition()) - pSour->GetTransform()->Get_State(STATE::POSITION)));
		
		}
		if (pDest)
		{
			fDepth1 = XMVectorGetX(XMVector3Length(XMLoadFloat4(m_pGameInstance->Get_CamPosition()) - pDest->GetTransform()->Get_State(STATE::POSITION)));
		}

		return fDepth1 < fDepth2;
});
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
	m_pGameInstance->Render_MRT_Debug(TEXT("MRT_Bloom"), m_pShader, m_pVIBuffer);
	m_pGameInstance->Render_MRT_Debug(TEXT("MRT_BlurEffect"), m_pShader, m_pVIBuffer);
	m_pGameInstance->Render_MRT_Debug(TEXT("MRT_ShadowObjects"), m_pShader, m_pVIBuffer);

	return S_OK;
}
//
//HRESULT CRenderer::Ready_RenderTargets()
//{
//	return S_OK;
//}

HRESULT CRenderer::Ready_DepthStencilView(_uint iWidth, _uint iHeight)
{
	ID3D11Texture2D* pDepthStencilTexture = nullptr;

	D3D11_TEXTURE2D_DESC	TextureDesc;
	ZeroMemory(&TextureDesc, sizeof(D3D11_TEXTURE2D_DESC));


	TextureDesc.Width = iWidth;
	TextureDesc.Height = iHeight;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &pDepthStencilTexture)))
		return E_FAIL;


	if (FAILED(m_pDevice->CreateDepthStencilView(pDepthStencilTexture, nullptr, &m_pShadowDSV)))
		return E_FAIL;

	Safe_Release(pDepthStencilTexture);

	return S_OK;
}

HRESULT CRenderer::Change_ViewportDesc(_uint iWidth, _uint iHeight)
{
	D3D11_VIEWPORT			ViewportDesc{};
	_uint					iNumViewports = { 1 };

	ViewportDesc.TopLeftX = 0;
	ViewportDesc.TopLeftY = 0;
	ViewportDesc.Width = iWidth;
	ViewportDesc.Height = iHeight;
	ViewportDesc.MinDepth = 0.f;
	ViewportDesc.MaxDepth = 1.f;

	m_pContext->RSSetViewports(iNumViewports, &ViewportDesc);

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
	Safe_Release(m_pShadowDSV);
	Safe_Release(m_pBackBuffer);
	Safe_Release(m_pOriginalDSV);

	for (auto& ObjectList : m_RenderObjects)
	{
		for (auto& pGameObject : ObjectList)
			Safe_Release(pGameObject);
		ObjectList.clear();
	}		
}
