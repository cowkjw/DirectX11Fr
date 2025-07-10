#include "ParticleEffect.h"

#include "GameInstance.h"

CParticleEffect::CParticleEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect{ pDevice, pContext }

{

}

CParticleEffect::CParticleEffect(const CParticleEffect& Prototype)
	: CEffect(Prototype)
	, m_ParticleTextureIndices(Prototype.m_ParticleTextureIndices)
	, m_bUseParentTexture(Prototype.m_bUseParentTexture)
	, m_ParticleUVs(Prototype.m_ParticleUVs)

{
	for (const auto& Pair : Prototype.m_ParticleSystems)
	{
		if (Pair.second)
		{
			CParticleSystem* pCloned = static_cast<CParticleSystem*>(Pair.second->Clone(nullptr));
			m_ParticleSystems.insert({ Pair.first, pCloned });
		}
	}

	// 셰이더 맵 깊은 복사
	for (const auto& Pair : Prototype.m_ParticleShaders)
	{
		m_ParticleShaders.insert({ Pair.first, Pair.second });
		Safe_AddRef(Pair.second);
	}
}

HRESULT CParticleEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CParticleEffect::Initialize(void* pArg)
{
	GAMEOBJECT_DESC			Desc{};
	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 0.f;
	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CParticleEffect::Priority_Update(_float fTimeDelta)
{

}

void CParticleEffect::Update(_float fTimeDelta)
{
	//if (m_pBoneSocket)
	//{
	//	_float4x4 parentWorld = m_pParent->GetTransform()->Get_WorldMatrix();
	//	_float4x4 boneLocal = *m_pBoneSocket->Get_CombinedTransformationMatrix();
	//	_matrix matBoneLocal = XMLoadFloat4x4(&boneLocal);
	///*	for (size_t i = 0; i < 3; i++)
	//		matBoneLocal.r[i] = XMVector3Normalize(matBoneLocal.r[i]);*/

	//		_matrix world = XMMatrixMultiply(XMLoadFloat4x4(&boneLocal), XMLoadFloat4x4(&parentWorld));
	//	_matrix world = /*XMLoadFloat4x4(&m_pTransformCom->Get_WorldMatrix())**/matBoneLocal * XMLoadFloat4x4(&parentWorld);
	//	world = XMMatrixMultiply(matScale, world); 

	//	_float4x4 WorldMatrix{};
	//	XMStoreFloat4x4(&m_CombinedWorldMatrix, world);
	//	m_pTransformCom->Set_WorldMatrix(m_CombinedWorldMatrix);
	//}
	if (m_pBoneSocket)
	{
		_float4x4 parentWorld = m_pParent->GetTransform()->Get_WorldMatrix();
		_float4x4 boneLocal = *m_pBoneSocket->Get_CombinedTransformationMatrix();

		_matrix matBoneLocal = XMLoadFloat4x4(&boneLocal);
		_matrix matParentWorld = XMLoadFloat4x4(&parentWorld);

		_matrix world = XMMatrixMultiply(matBoneLocal, matParentWorld);

		// 파티클 이펙트의 로컬 오프셋이 있다면 적용
		 _matrix localOffset = XMLoadFloat4x4(&m_pTransformCom->Get_WorldMatrix());
		 world = XMMatrixMultiply(localOffset, world);


		XMStoreFloat4x4(&m_CombinedWorldMatrix, world);
	//	m_pTransformCom->Set_WorldMatrix(m_CombinedWorldMatrix);
	}
	else
	{
		// 본 소켓이 없으면 그냥 월드에 생성하도록
		m_CombinedWorldMatrix = GetTransform()->Get_WorldMatrix();
	}
	_int iActiveParticleCount = 0;
	for (const auto& Pair : m_ParticleSystems)
	{
		if (Pair.second && Pair.second->IsPlaying())
		{
			iActiveParticleCount++;
			Pair.second->UpdateVertexInstances(fTimeDelta);
		}
	}
	if (iActiveParticleCount == 0)
	{
		SetActive(false);
	}
}

void CParticleEffect::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONLIGHT, this);
}

HRESULT CParticleEffect::Render()
{

	//if (m_pTransformCom)
	//{
	//	m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
	//}

	/*for (auto& Pair : m_ParticleShaders)
	{
		if (Pair.second == nullptr)
			continue;
		if (m_pTransformCom)
		{
			m_pTransformCom->Bind_ShaderResource(Pair.second, "g_WorldMatrix");
		}


	}*/
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::PROJECTION))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;


	for (const auto& particle : m_ParticleSystems)
	{
		if (particle.second && particle.second->IsActive())
		{
			if (m_bUseParentTexture==false)
			{
				if (FAILED(m_pShaderCom->Begin(m_iShaderPass)))
					return E_FAIL;
			}
			else
			{
				if (m_iShaderPass != 0)
				{
					if (FAILED(m_pShaderCom->Begin(m_iShaderPass)))
						return E_FAIL;
				}
				else
				{
					if (FAILED(m_pShaderCom->Begin(m_ParticleShaderPasses[particle.first])))
						return E_FAIL;
				}
				if (m_ParticleTextures.find(particle.first) != m_ParticleTextures.end())
				{
					if (m_ParticleTextures[particle.first] == nullptr)
						return E_FAIL;
					if (m_ParticleTextureIndices.find(particle.first) == m_ParticleTextureIndices.end())
						return E_FAIL;
				}
				if (FAILED(m_ParticleTextures[particle.first]->Bind_ShaderResource(m_pShaderCom, "g_Texture", m_ParticleTextureIndices[particle.first])))
					return E_FAIL;
			}
			if (FAILED(particle.second->Bind_Buffers()))
				return E_FAIL;
			if (FAILED(particle.second->Render()))
				return E_FAIL;
		}
	}

	//for (const auto& Pair : m_ParticleSystems)
	//{
	//	if (Pair.second && Pair.second->IsActive())
	//	{
	//		if (FAILED(Pair.second->Bind_Buffers()))
	//			return E_FAIL;
	//		if (FAILED(Pair.second->Render()))
	//			return E_FAIL;
	//	}
	//}

	//if (FAILED(m_ParticleSystems[L"Snow"]->Bind_Buffers()))
	//	return E_FAIL;

	//if (FAILED(m_ParticleSystems[L"Snow"]->Render()))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CParticleEffect::Ready_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxPointInstance"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	///* For.Com_Particle */
	//if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Particle"),
	//	TEXT("Com_Particle"), reinterpret_cast<CComponent**>(&m_ParticleSystems[L"Snow"]))))
	//	return E_FAIL;
	///* For.Com_Texture */

	//if (FAILED(__super::Add_Component(TEXT("Com_Texture"), m_pGameInstance->GetTexture(TEXT("TitleLogo"), true), reinterpret_cast<CComponent**>(&m_Textures[TEX_DIFFUSE]))))
	//	return E_FAIL;
	return S_OK;
}

CParticleEffect* CParticleEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CParticleEffect* pInstance = new CParticleEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMainApp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CParticleEffect::AddParticleSystem(const _wstring& particleName, CParticleSystem* pParticleSystem)
{
	if (nullptr == pParticleSystem)
		return;
	auto it = m_ParticleSystems.find(particleName);
	if (it != m_ParticleSystems.end())
	{
		Safe_Release(it->second);
	}
	m_ParticleSystems[particleName] = pParticleSystem;
	Safe_AddRef(pParticleSystem);
}

void CParticleEffect::RemoveParticleSystem(CParticleSystem* pParticleSystem)
{
	if (nullptr == pParticleSystem)
		return;
	for (auto it = m_ParticleSystems.begin(); it != m_ParticleSystems.end();)
	{
		if (it->second == pParticleSystem)
		{
			Safe_Release(it->second);
			it = m_ParticleSystems.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void CParticleEffect::RemoveParticleSystem(const _wstring& particleName)
{
	auto it = m_ParticleSystems.find(particleName);
	if (it != m_ParticleSystems.end())
	{
		Safe_Release(it->second);
		m_ParticleSystems.erase(it);
	}
	if (m_ParticleTextures.find(particleName) != m_ParticleTextures.end())
	{
		Safe_Release(m_ParticleTextures[particleName]);
		m_ParticleTextures.erase(particleName);
	}
	if (m_ParticleShaders.find(particleName) != m_ParticleShaders.end())
	{
		Safe_Release(m_ParticleShaders[particleName]);
		m_ParticleShaders.erase(particleName);
	}
	if (m_ParticleShaderPasses.find(particleName) != m_ParticleShaderPasses.end())
	{
		m_ParticleShaderPasses.erase(particleName);
	}
	if (m_ParticleTextureIndices.find(particleName) != m_ParticleTextureIndices.end())
	{
		m_ParticleTextureIndices.erase(particleName);
	}
}

void CParticleEffect::EnableUVAnim(const _wstring& tag, _float cols, _float rows, _float frameSec)
{
	if (m_ParticleUVs.find(tag) != m_ParticleUVs.end())
	{
	
		PARTICLE_UV& uv = m_ParticleUVs[tag];
		uv.fCols = cols;
		uv.fRows = rows;
		uv.iTotalFrames = cols * rows; // 총 프레임 수 계산
		uv.fFrameTime = frameSec;
		uv.fCurrentTime = 0.f; // 현재 시간 초기화
		uv.vUVScale = _float2(1.f / uv.fCols, 1.f / uv.fRows); // UV 스케일 계산
		uv.vUVOffset = _float2(0.f, 0.f); // UV 오프셋 초기화
		uv.iCurrentFrame = 0; // 현재 프레임 초기화
	}
	else
	{
		PARTICLE_UV uv;
		uv.fCols = cols;
		uv.fRows = rows;
		uv.iTotalFrames = cols * rows; // 총 프레임 수 계산
		uv.fFrameTime = frameSec;
		uv.fCurrentTime = 0.f;
		uv.vUVScale = _float2(1.f / uv.fCols, 1.f / uv.fRows); // UV 스케일 계산
		uv.vUVOffset = _float2(0.f, 0.f); // UV 오프셋 초기화
		uv.iCurrentFrame = 0; // 현재 프레임 초기화
		m_ParticleUVs[tag] = uv;
	}
}

void CParticleEffect::ClearParticleSystems()
{
	for (auto& pair : m_ParticleSystems)
	{
		Safe_Release(pair.second);
	}

	m_ParticleSystems.clear();
}

void CParticleEffect::SpwanParticle()
{
	for (const auto& Pair : m_ParticleSystems)
	{
		if (Pair.second)
		{
			Pair.second->ResetParticle();
		}
	}
}

void CParticleEffect::DespwanParticle()
{
	for (const auto& Pair : m_ParticleSystems)
	{
		if (Pair.second)
		{
			Pair.second->SetActive(false);
		}
	}
}

void CParticleEffect::OnEnable()
{
	SpwanParticle();
}

void CParticleEffect::SetTexture(CTexture* pTexture, _uint iTextureIndex, const _wstring& tag)
{
	if (nullptr == pTexture)
		return;
	auto it = m_ParticleTextures.find(tag); // 찾아보고
	if (it != m_ParticleTextures.end()) // 있으면 삭제하고
	{
		Safe_Release(it->second);
	}
	if (tag.empty()) // 태그가 비어있으면 기본 텍스쳐로 설정
	{
		m_ParticleTextures[L"Default"] = pTexture; // 기본 텍스쳐 설정
		m_ParticleShaderPasses[L"Default"] = iTextureIndex; // 기본 셰이더 패스 설정
	}
	else // 태그가 있으면
	{
		m_ParticleTextures[tag] = pTexture; // 텍스쳐 저장
		m_ParticleTextureIndices[tag] = iTextureIndex; // 특정 태그에 대한 셰이더 패스 설정
	}
	
	Safe_AddRef(pTexture); // 텍스쳐 참조 카운트 증가
}

void CParticleEffect::SetShader(CShader* pShader, _uint iShaderPass, const _wstring& tag)
{
	if (nullptr == pShader)
		return;
	auto it = m_ParticleShaders.find(tag); // 찾아보고
	if (it != m_ParticleShaders.end()) // 있으면 삭제하고
	{
		Safe_Release(it->second);
	}
	if (tag.empty()) // 태그가 비어있으면 기본 셰이더로 설정
	{
		m_ParticleShaders[L"Default"] = pShader; // 기본 셰이더 설정
		m_iShaderPass = iShaderPass; // 기본 셰이더 패스 설정
		m_ParticleShaderPasses[L"Default"] = iShaderPass; // 기본 셰이더 패스 설정
	}
	// 태그가 있으면
	else
	{
		m_ParticleShaders[tag] = pShader; // 셰이더 저장
		m_ParticleShaderPasses[tag] = iShaderPass; // 특정 태그에 대한 셰이더 패스 설정
	}
	Safe_AddRef(pShader); // 셰이더 참조 카운트 증가
}

void CParticleEffect::AddParticleSystem_ForEditor(const _wstring& particleName, CParticleSystem* pParticleSystem, CTexture* pTexture, PARTICLE_UV particleUV,
	_uint iTextureIndex, _uint iShaderPass)
{
	if (nullptr == pParticleSystem || pTexture == nullptr)
		return;
	auto it = m_ParticleSystems.find(particleName);
	if (it != m_ParticleSystems.end())
	{
		Safe_Release(it->second);
	}
	m_ParticleSystems[particleName] = pParticleSystem;
	Safe_AddRef(pParticleSystem);
	Safe_AddRef(pTexture); // 텍스쳐 참조 카운트 증가
	m_ParticleTextureIndices[particleName] = iTextureIndex;
	m_ParticleShaderPasses[particleName] = iShaderPass;
	m_ParticleTextures[particleName] = pTexture; // 텍스쳐 저장
	m_ParticleUVs[particleName] = particleUV; // UV 오프셋 저장
}


CGameObject* CParticleEffect::Clone(void* pArg)
{
	CParticleEffect* pInstance = new CParticleEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CParticleEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CParticleEffect::Free()
{
	__super::Free();
	for (auto& pair : m_ParticleSystems)
	{
		Safe_Release(pair.second);
	}
	for (auto& pair : m_ParticleTextures)
	{
		Safe_Release(pair.second);
	}
	for (auto& pair : m_ParticleShaders)
	{
		Safe_Release(pair.second);
	}
	m_ParticleSystems.clear();
}
