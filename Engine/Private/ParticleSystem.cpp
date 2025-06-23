#include "ParticleSystem.h"
#include "GameInstance.h"

CParticleSystem::CParticleSystem(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer_Instance(pDevice, pContext)
{
}

CParticleSystem::CParticleSystem(const CParticleSystem& Prototype)
	: CVIBuffer_Instance(Prototype)
	, m_pVertexInstances{ Prototype.m_pVertexInstances }
	, m_vecSpeeds{ Prototype.m_vecSpeeds }
	, m_vecVelocities{Prototype.m_vecVelocities}
	, m_bIsLoop{ Prototype.m_bIsLoop }
	, m_ParticleDesc{ Prototype.m_ParticleDesc }
{
}

HRESULT CParticleSystem::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CParticleSystem::Initialize_Prototype(const PARTICLE_DESC& desc)
{
	m_ParticleDesc = desc;
	SetVertexInfo(desc);
	if (FAILED(CreateVertexBuffer()))
		return E_FAIL;
	if (FAILED(CreateIndexBuffer()))
		return E_FAIL;
	if (FAILED(CreateVertexInstances(desc)))
		return E_FAIL;
    return S_OK;
}

HRESULT CParticleSystem::Initialize(void* pArg)
{
	if (FAILED(m_pDevice->CreateBuffer(&m_VBInstanceDesc, &m_VBInstanceSubresourceData, &m_pVBInstance)))
		return E_FAIL;
    return S_OK;
}

void CParticleSystem::Drop(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE	SubResource{};

	if (FAILED(m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource)))
		return;

	if (m_ParticleDesc.eParticleType == PARTICLE_TYPE::POINT)
	{
		VTXPOINT_PARTICLE_INSTANCE* pVertices = static_cast<VTXPOINT_PARTICLE_INSTANCE*>(SubResource.pData);
		for (size_t i = 0; i < m_iNumInstance; i++)
		{
			pVertices[i].vLifeTime.y += fTimeDelta;
			pVertices[i].vTranslation.y -= m_vecSpeeds[i] * fTimeDelta;
			if (true == m_bIsLoop &&
				pVertices[i].vLifeTime.y >= pVertices[i].vLifeTime.x)
			{
				pVertices[i].vLifeTime.y = 0.f;
				pVertices[i].vTranslation = static_cast<VTXPOINT_PARTICLE_INSTANCE*>(m_pVertexInstances)[i].vTranslation;
			}
		}
	}
	else
	{
		VTXRECT_PARTICLE_INSTANCE* pVertices = static_cast<VTXRECT_PARTICLE_INSTANCE*>(SubResource.pData);

		for (size_t i = 0; i < m_iNumInstance; i++)
		{
			pVertices[i].vLifeTime.y += fTimeDelta;

			pVertices[i].vTranslation.y -= m_vecSpeeds[i] * fTimeDelta;

			if (true == m_bIsLoop &&
				pVertices[i].vLifeTime.y >= pVertices[i].vLifeTime.x)
			{
				pVertices[i].vLifeTime.y = 0.f;
				pVertices[i].vTranslation = static_cast<VTXRECT_PARTICLE_INSTANCE*>(m_pVertexInstances)[i].vTranslation;
			}
		}
	}
	
	m_pContext->Unmap(m_pVBInstance, 0);
}

void CParticleSystem::Spread(_float fTimeDelta)
{
}

HRESULT CParticleSystem::UpdateVertexInstances(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE	SubResource{};

	if (FAILED(m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource)))
		return E_FAIL;

	if (m_ParticleDesc.eParticleType == PARTICLE_TYPE::POINT)
	{
		VTXPOINT_PARTICLE_INSTANCE* pVertices = static_cast<VTXPOINT_PARTICLE_INSTANCE*>(SubResource.pData);
		for (size_t i = 0; i < m_iNumInstance; i++)
		{
			pVertices[i].vLifeTime.y += fTimeDelta;
			pVertices[i].vTranslation.y += m_vecSpeeds[i] * fTimeDelta;
			if (true == m_bIsLoop &&
				pVertices[i].vLifeTime.y >= pVertices[i].vLifeTime.x)
			{
				pVertices[i].vLifeTime.y = 0.f;
				pVertices[i].vTranslation = static_cast<VTXPOINT_PARTICLE_INSTANCE*>(m_pVertexInstances)[i].vTranslation;
			}
		}
	}
	else
	{
		VTXRECT_PARTICLE_INSTANCE* pVertices = static_cast<VTXRECT_PARTICLE_INSTANCE*>(SubResource.pData);

		for (size_t i = 0; i < m_iNumInstance; i++)
		{
			pVertices[i].vLifeTime.y += fTimeDelta;

			if (m_ParticleDesc.fGravity != 0.f)
			{
				m_vecVelocities[i].y -= m_ParticleDesc.fGravity * fTimeDelta;
			}
			pVertices[i].vTranslation.x += m_vecVelocities[i].x * fTimeDelta;
			pVertices[i].vTranslation.y += m_vecVelocities[i].y * fTimeDelta;
			pVertices[i].vTranslation.z += m_vecVelocities[i].z * fTimeDelta;

			if (true == m_bIsLoop &&
				pVertices[i].vLifeTime.y >= pVertices[i].vLifeTime.x)
			{
				pVertices[i].vLifeTime.y = 0.f;
				pVertices[i].vTranslation = static_cast<VTXRECT_PARTICLE_INSTANCE*>(m_pVertexInstances)[i].vTranslation;
			}
		}
	}

	m_pContext->Unmap(m_pVBInstance, 0);
	return S_OK;
}

json CParticleSystem::Serialize()
{
	json j;
	j["ParticleType"] = static_cast<int>(m_ParticleDesc.eParticleType);
	j["NumInstance"] = m_ParticleDesc.iNumInstance;
	j["Range"] = { m_ParticleDesc.vRange.x, m_ParticleDesc.vRange.y, m_ParticleDesc.vRange.z };
	j["Size"] = { m_ParticleDesc.vSize.x, m_ParticleDesc.vSize.y };
	j["Center"] = { m_ParticleDesc.vCenter.x, m_ParticleDesc.vCenter.y, m_ParticleDesc.vCenter.z };
	j["IsLoop"] = m_bIsLoop;
	j["LifeTime"] = { m_ParticleDesc.vLifeTime.x, m_ParticleDesc.vLifeTime.y };
	j["Speed"] = { m_ParticleDesc.vSpeed.x, m_ParticleDesc.vSpeed.y };
	j["StartColor"] = { m_ParticleDesc.vStartColor.x, m_ParticleDesc.vStartColor.y, m_ParticleDesc.vStartColor.z };
	j["EndColor"] = { m_ParticleDesc.vEndColor.x, m_ParticleDesc.vEndColor.y, m_ParticleDesc.vEndColor.z };
	j["Velocity"] = { m_ParticleDesc.vVelocity.x, m_ParticleDesc.vVelocity.y, m_ParticleDesc.vVelocity.z };
	j["Gravity"] = m_ParticleDesc.fGravity;
	j["SpreadAngle"] = m_ParticleDesc.fSpreadAngle;
	j["AlphaVariation"] = m_ParticleDesc.fAlphaVariation;
	return json();
}

void CParticleSystem::SetVertexInfo(const PARTICLE_DESC& desc)
{
	if (desc.eParticleType == PARTICLE_TYPE::RECT)
	{
		m_iNumInstance = desc.iNumInstance;
		m_iVertexInstanceStride = sizeof(VTXRECT_PARTICLE_INSTANCE);
		m_iNumIndexPerInstance = 6; 
		
		m_iNumVertices = 4;
		m_iVertexStride = sizeof(VTXPOSTEX);
		m_ePrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		m_iNumIndices = m_iNumIndexPerInstance;
	}
	else if (desc.eParticleType == PARTICLE_TYPE::POINT)
	{
		m_iNumInstance = desc.iNumInstance;
		m_iVertexInstanceStride = sizeof(VTXPOINT_PARTICLE_INSTANCE);
		m_iNumIndexPerInstance = 1;

		m_iNumVertices = 1; // 포인트 파티클은 단일 버텍스
		m_iVertexStride = sizeof(VTXPOINT);
		m_ePrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
	}
	m_iNumVertexBuffers = 2; // 정적 버텍스와 인스턴스용 버퍼
	m_iIndexStride = sizeof(_ushort);
	m_eIndexFormat = DXGI_FORMAT_R16_UINT;
	m_iNumIndices = m_iNumIndexPerInstance;
}

HRESULT CParticleSystem::CreateVertexBuffer()
{
	D3D11_BUFFER_DESC			VBBufferDesc{};
	VBBufferDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
	VBBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VBBufferDesc.CPUAccessFlags = /*D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE*/0;
	VBBufferDesc.StructureByteStride = m_iVertexStride;
	VBBufferDesc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA		VBInitialData{};
	if (m_ParticleDesc.eParticleType == PARTICLE_TYPE::RECT)
	{
		VTXPOSTEX* pVertices = new VTXPOSTEX[m_iNumVertices];
		ZeroMemory(pVertices, sizeof(VTXPOSTEX) * m_iNumVertices);

		m_pVertexPositions = new _float3[m_iNumVertices];
		ZeroMemory(m_pVertexPositions, sizeof(_float3) * m_iNumVertices);

		pVertices[0].vPosition = _float3(-0.5f, 0.5f, 0.f);
		pVertices[0].vTexcoord = _float2(0.f, 0.f);

		pVertices[1].vPosition = _float3(0.5f, 0.5f, 0.f);
		pVertices[1].vTexcoord = _float2(1.f, 0.f);

		pVertices[2].vPosition = _float3(0.5f, -0.5f, 0.f);
		pVertices[2].vTexcoord = _float2(1.f, 1.f);

		pVertices[3].vPosition = _float3(-0.5f, -0.5f, 0.f);
		pVertices[3].vTexcoord = _float2(0.f, 1.f);

		for (_uint i = 0; i < m_iNumVertices; ++i)
			m_pVertexPositions[i] = pVertices[i].vPosition;
		VBInitialData.pSysMem = pVertices;

		if (FAILED(m_pDevice->CreateBuffer(&VBBufferDesc, &VBInitialData, &m_pVB)))
			return E_FAIL;

		Safe_Delete_Array(pVertices);
	}
	else if (m_ParticleDesc.eParticleType == PARTICLE_TYPE::POINT)
	{
		VTXPOINT* pVertices = new VTXPOINT[m_iNumVertices];
		ZeroMemory(pVertices, sizeof(VTXPOINT) * m_iNumVertices);

		m_pVertexPositions = new _float3[m_iNumVertices];
		ZeroMemory(m_pVertexPositions, sizeof(_float3) * m_iNumVertices);

		pVertices[0].vPosition = _float3(-0.5f, 0.5f, 0.f);

		_float	fSize = m_pGameInstance->Compute_Random(m_ParticleDesc.vSize.x, m_ParticleDesc.vSize.y);

		pVertices[0].vPSize = _float2(fSize, fSize);

		for (_uint i = 0; i < m_iNumVertices; ++i)
			m_pVertexPositions[i] = pVertices[i].vPosition;
		VBInitialData.pSysMem = pVertices;

		if (FAILED(m_pDevice->CreateBuffer(&VBBufferDesc, &VBInitialData, &m_pVB)))
			return E_FAIL;

		Safe_Delete_Array(pVertices);
	}

	return S_OK;
}

HRESULT CParticleSystem::CreateIndexBuffer()
{
	if (m_ParticleDesc.eParticleType == PARTICLE_TYPE::RECT)
	{
	D3D11_BUFFER_DESC			IBBufferDesc{};
	IBBufferDesc.ByteWidth = m_iNumIndices * m_iIndexStride;
	IBBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IBBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	IBBufferDesc.CPUAccessFlags = /*D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE*/0;
	IBBufferDesc.StructureByteStride = m_iIndexStride;
	IBBufferDesc.MiscFlags = 0;

	_ushort* pIndices = new _ushort[m_iNumIndices];
	ZeroMemory(pIndices, sizeof(_ushort) * m_iNumIndices);
	D3D11_SUBRESOURCE_DATA		IBInitialData{};
	
		_uint		iNumIndices = {};

		pIndices[iNumIndices++] = 0;
		pIndices[iNumIndices++] = 1;
		pIndices[iNumIndices++] = 2;

		pIndices[iNumIndices++] = 0;
		pIndices[iNumIndices++] = 2;
		pIndices[iNumIndices++] = 3;

		IBInitialData.pSysMem = pIndices;
		if (FAILED(m_pDevice->CreateBuffer(&IBBufferDesc, &IBInitialData, &m_pIB)))
			return E_FAIL;
		Safe_Delete_Array(pIndices);
	}
	return S_OK;
}

HRESULT CParticleSystem::CreateVertexInstances(const PARTICLE_DESC& desc)
{
	m_VBInstanceDesc.ByteWidth = m_iNumInstance * m_iVertexInstanceStride;
	m_VBInstanceDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_VBInstanceDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_VBInstanceDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_VBInstanceDesc.StructureByteStride = m_iVertexInstanceStride;
	m_VBInstanceDesc.MiscFlags = 0;

	m_vecSpeeds.resize(m_iNumInstance);
	m_vecVelocities.resize(m_iNumInstance);

	if (desc.eParticleType == PARTICLE_TYPE::RECT)
	{
		VTXRECT_PARTICLE_INSTANCE* pBuffer = new VTXRECT_PARTICLE_INSTANCE[m_iNumInstance];
		for (size_t i = 0; i < m_iNumInstance; i++)
		{
			m_vecSpeeds[i] = m_pGameInstance->Compute_Random(desc.vSpeed.x, desc.vSpeed.y);
			_float	fSize = m_pGameInstance->Compute_Random(desc.vSize.x, desc.vSize.y);

			_float angleX = XMConvertToRadians(m_pGameInstance->Compute_Random(-m_ParticleDesc.fSpreadAngle * 0.5f, m_ParticleDesc.fSpreadAngle * 0.5f));
			_float angleY = XMConvertToRadians(m_pGameInstance->Compute_Random(-m_ParticleDesc.fSpreadAngle * 0.5f, m_ParticleDesc.fSpreadAngle * 0.5f));
			_float angleZ = XMConvertToRadians(m_pGameInstance->Compute_Random(-m_ParticleDesc.fSpreadAngle * 0.5f, m_ParticleDesc.fSpreadAngle * 0.5f));
			
			_vector baseDir = XMLoadFloat3(&m_ParticleDesc.vVelocity);
			if (XMVector3Equal(baseDir, XMVectorZero()))
				baseDir = XMVectorSet(0.f, 1.f, 0.f, 0.f); // 기본 위 방향

			_matrix rot = XMMatrixRotationRollPitchYaw(angleX, angleY, angleZ);
			_vector spreadDir = XMVector3TransformNormal(baseDir, rot);
			spreadDir = XMVector3Normalize(spreadDir);

			// 최종 속도 벡터 = 방향 * 속도
			_vector finalVelocity = spreadDir * m_vecSpeeds[i];

			// 저장
			XMStoreFloat3(&m_vecVelocities[i], finalVelocity);

			pBuffer[i].vRight = _float4(fSize, 0.f, 0.f, 0.f);
			pBuffer[i].vUp = _float4(0.f, fSize, 0.f, 0.f);
			pBuffer[i].vLook = _float4(0.f, 0.f, fSize, 0.f);


			pBuffer[i].vTranslation = _float4(
				m_pGameInstance->Compute_Random(desc.vCenter.x - desc.vRange.x * 0.5f, desc.vCenter.x + desc.vRange.x * 0.5f),
				m_pGameInstance->Compute_Random(desc.vCenter.y - desc.vRange.y * 0.5f, desc.vCenter.y + desc.vRange.y * 0.5f),
				m_pGameInstance->Compute_Random(desc.vCenter.z - desc.vRange.z * 0.5f, desc.vCenter.z + desc.vRange.z * 0.5f),
				1.f
			);

			pBuffer[i].vLifeTime = _float2(
				m_pGameInstance->Compute_Random(desc.vLifeTime.x, desc.vLifeTime.y),
				0.f
			);
			pBuffer[i].vStartColor = desc.vStartColor;
			pBuffer[i].vEndColor = desc.vEndColor;
			pBuffer[i].fAlphaVariation = desc.fAlphaVariation;
		}
		m_pVertexInstances = pBuffer;
	}
	else if (desc.eParticleType == PARTICLE_TYPE::POINT)
	{
		VTXPOINT_PARTICLE_INSTANCE* pBuffer = new VTXPOINT_PARTICLE_INSTANCE[m_iNumInstance];
		for (size_t i = 0; i < m_iNumInstance; i++)
		{
			m_vecSpeeds[i] = m_pGameInstance->Compute_Random(desc.vSpeed.x, desc.vSpeed.y);
			_float	fSize = m_pGameInstance->Compute_Random(desc.vSize.x, desc.vSize.y);



			pBuffer[i].vRight = _float4(fSize, 0.f, 0.f, 0.f);
			pBuffer[i].vUp = _float4(0.f, fSize, 0.f, 0.f);
			pBuffer[i].vLook = _float4(0.f, 0.f, fSize, 0.f);


			pBuffer[i].vTranslation = _float4(
				m_pGameInstance->Compute_Random(desc.vCenter.x - desc.vRange.x * 0.5f, desc.vCenter.x + desc.vRange.x * 0.5f),
				m_pGameInstance->Compute_Random(desc.vCenter.y - desc.vRange.y * 0.5f, desc.vCenter.y + desc.vRange.y * 0.5f),
				m_pGameInstance->Compute_Random(desc.vCenter.z - desc.vRange.z * 0.5f, desc.vCenter.z + desc.vRange.z * 0.5f),
				1.f
			);

			pBuffer[i].vLifeTime = _float2(
				m_pGameInstance->Compute_Random(desc.vLifeTime.x, desc.vLifeTime.y),
				0.f
			);

		}
		m_pVertexInstances = pBuffer;
	}
	m_VBInstanceSubresourceData.pSysMem = m_pVertexInstances;

	return S_OK;
}

CParticleSystem* CParticleSystem::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const PARTICLE_DESC& desc)
{
	CParticleSystem* pInstance = new CParticleSystem(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype(desc)))
	{
		MSG_BOX("Failed to create CParticleSystem");
		Safe_Release(pInstance);
		return nullptr;
	}
	return pInstance;
}

CComponent* CParticleSystem::Clone(void* pArg)
{
	CParticleSystem* pClone = new CParticleSystem(*this);
	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to clone CParticleSystem");
		Safe_Release(pClone);
		return nullptr;
	}
	return pClone;
}

void CParticleSystem::Free()
{
	__super::Free();

	if (!m_isCloned)
	{
		if (m_ParticleDesc.eParticleType == PARTICLE_TYPE::RECT)
		{
			delete[] static_cast<VTXRECT_PARTICLE_INSTANCE*>(m_pVertexInstances);
		}
		else if (m_ParticleDesc.eParticleType == PARTICLE_TYPE::POINT)
		{
			delete[] static_cast<VTXPOINT_PARTICLE_INSTANCE*>(m_pVertexInstances);
		}
		m_pVertexInstances = nullptr;
	}
	m_vecSpeeds.clear();
}
