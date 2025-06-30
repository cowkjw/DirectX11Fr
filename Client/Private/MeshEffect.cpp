#include "ParticleEffect.h"
#include "GameInstance.h"
#include "MeshEffect.h"
#include "Model.h"
CMeshEffect::CMeshEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect{ pDevice, pContext }
{
}
CMeshEffect::CMeshEffect(const CMeshEffect& Prototype)
    : CEffect(Prototype),
	m_pModelCom{ Prototype.m_pModelCom },
	m_bUseOffset{ Prototype.m_bUseOffset }
{
	Safe_AddRef(m_pModelCom);
}

HRESULT CMeshEffect::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMeshEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
    return S_OK;
}

void CMeshEffect::Priority_Update(_float fTimeDelta)
{
}

void CMeshEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CMeshEffect::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CMeshEffect::Render()
{
	Bind_Shader();

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0);
		m_pModelCom->Bind_Material(m_pShaderCom, "g_SpecularTexture", i, aiTextureType_OPACITY, 0);
		m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);

		m_pModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Begin(m_iShaderPass)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

void CMeshEffect::SetMesh(CMesh* pMesh)
{
}

void CMeshEffect::SetModel(CModel* pModel)
{
}

void CMeshEffect::SetBone(CBone* pBone)
{
}

void CMeshEffect::SetRenderMesh(_bool bRenderMesh)
{
	m_bRenderMesh = bRenderMesh;
	if (m_bRenderMesh)
	{
		m_fUVOffset = _float2(0.5f, 0.f); // UV 오프셋 초기화
		m_bUseOffset = true; // UV 애니메이션 활성화
		for (const auto& particles : m_ParticleEffects)
		{
			particles.second->SpwanParticle();
		}
	}
}

HRESULT CMeshEffect::Ready_Components()
{
    return S_OK;
}

CMeshEffect* CMeshEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMeshEffect* pInstance = new CMeshEffect(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMeshEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CMeshEffect::Clone(void* pArg)
{
	CMeshEffect* pClone = new CMeshEffect(*this);
	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMeshEffect");
		Safe_Release(pClone);
	}
	return pClone;
}

void CMeshEffect::Free()
{
	__super::Free();
	for (auto& particles : m_ParticleEffects)
	{
		Safe_Release(particles.second);
	}
	m_ParticleEffects.clear();
	Safe_Release(m_pModelCom);
}
