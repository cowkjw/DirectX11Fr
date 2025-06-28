#include "Mesh.h"

#include "Bone.h"
#include "Shader.h"

CMesh::CMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer{ pDevice, pContext }
{
}

CMesh::CMesh(const CMesh& Prototype)
	: CVIBuffer(Prototype)
{
}

HRESULT CMesh::Initialize_Prototype(MODEL eType, const aiMesh* pAIMesh, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix)
{
	strcpy_s(m_szName, pAIMesh->mName.data);
	m_iMaterialIndex = pAIMesh->mMaterialIndex;
	m_iNumVertexBuffers = 1;
	m_iNumVertices = pAIMesh->mNumVertices;
	m_iNumIndices = pAIMesh->mNumFaces * 3;
	m_iIndexStride = sizeof(_uint);
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_iNumPritimive = m_iNumIndices / 3;

	HRESULT hr = eType == MODEL::NONANIM ? Ready_NonAnim_Mesh(pAIMesh, PreTransformMatrix) : Ready_Anim_Mesh(pAIMesh, Bones);

	if (FAILED(hr))
		return E_FAIL;


	D3D11_BUFFER_DESC			IBBufferDesc{};
	IBBufferDesc.ByteWidth = m_iNumIndices * m_iIndexStride;
	IBBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IBBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	IBBufferDesc.CPUAccessFlags = /*D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE*/0;
	IBBufferDesc.StructureByteStride = m_iIndexStride;
	IBBufferDesc.MiscFlags = 0;

	//_uint* pIndices = new _uint[m_iNumIndices];
	//ZeroMemory(pIndices, sizeof(_uint) * m_iNumIndices);

	m_pIndices = new _uint[m_iNumIndices];
	ZeroMemory(m_pIndices, sizeof(_uint) * m_iNumIndices);

	_uint* pIndices = static_cast<_uint*>(m_pIndices);

	_uint	iNumIndices = { 0 };

	for (size_t i = 0; i < pAIMesh->mNumFaces; i++)
	{
		pIndices[iNumIndices++] = pAIMesh->mFaces[i].mIndices[0];
		pIndices[iNumIndices++] = pAIMesh->mFaces[i].mIndices[1];
		pIndices[iNumIndices++] = pAIMesh->mFaces[i].mIndices[2];
	}

	D3D11_SUBRESOURCE_DATA		IBInitialData{};
	IBInitialData.pSysMem = m_pIndices;

	if (FAILED(m_pDevice->CreateBuffer(&IBBufferDesc, &IBInitialData, &m_pIB)))
		return E_FAIL;

	size_t ibBytes = m_iNumIndices * m_iIndexStride;
	m_RawIB.resize(ibBytes);
	memcpy(m_RawIB.data(), m_pIndices, ibBytes);

	//Safe_Delete_Array(pIndices);




	return S_OK;
}

HRESULT CMesh::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CMesh::Initialize_FromData(void* pVertexData, UINT vertexCount, UINT vertexStride, void* pIndexData, UINT indexCount, UINT indexStride, _bool isAnim, const vector<class CBone*>& bones, const _fmatrix& PreTransformMatrix)
{
	m_iNumVertices = vertexCount;
	m_iVertexStride = vertexStride;
	m_iNumIndices = indexCount;
	m_iIndexStride = indexStride;
	m_iNumVertexBuffers = 1;
	m_ePrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_iNumPritimive = m_iNumIndices / 3;
	m_pIndices = new _uint[m_iNumIndices];
	memcpy(
		m_pIndices,                     // dest
		pIndexData,                     // src
		m_iNumIndices * m_iIndexStride  // byte count
	);
	// VB 생성
	D3D11_BUFFER_DESC vbDesc{};
	vbDesc.ByteWidth = vertexCount * vertexStride;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.StructureByteStride = vertexStride;

	D3D11_SUBRESOURCE_DATA vbInit{};
	vbInit.pSysMem = pVertexData;
	if (FAILED(m_pDevice->CreateBuffer(&vbDesc, &vbInit, &m_pVB)))
		return E_FAIL;

	// IB 생성
	D3D11_BUFFER_DESC ibDesc{};
	ibDesc.ByteWidth = indexCount * indexStride;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.StructureByteStride = indexStride;

	D3D11_SUBRESOURCE_DATA ibInit{};
	ibInit.pSysMem = pIndexData;
	if (FAILED(m_pDevice->CreateBuffer(&ibDesc, &ibInit, &m_pIB)))
		return E_FAIL;


	// m_pVertexPositions 채우기 (PreTransformMatrix 적용)
	m_pVertexPositions = new _float3[vertexCount];
	if (isAnim) {
		auto verts = reinterpret_cast<const VTXANIMMESH*>(pVertexData);
		for (UINT i = 0; i < vertexCount; ++i)
			m_pVertexPositions[i] = verts[i].vPosition;
	}
	else {
		auto verts = reinterpret_cast<const VTXMESH*>(pVertexData);
		for (UINT i = 0; i < vertexCount; ++i) {
			XMVECTOR pos = XMLoadFloat3(&verts[i].vPosition);
			//pos = XMVector3TransformCoord(pos, XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&PreTransformMatrix)));
			XMStoreFloat3(&m_pVertexPositions[i], pos);
		}
	}

	return S_OK;
}

HRESULT CMesh::Ready_NonAnim_Mesh(const aiMesh* pAIMesh, _fmatrix PreTransformMatrix)
{
	m_iVertexStride = sizeof(VTXMESH);
	D3D11_BUFFER_DESC			VBBufferDesc{};
	VBBufferDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
	VBBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VBBufferDesc.CPUAccessFlags = /*D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE*/0;
	VBBufferDesc.StructureByteStride = m_iVertexStride;
	VBBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA		VBInitialData{};

	VTXMESH* pVertices = new VTXMESH[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXMESH) * m_iNumVertices);

	m_pVertexPositions = new _float3[m_iNumVertices];
	ZeroMemory(m_pVertexPositions, sizeof(_float3) * m_iNumVertices);

	for (size_t i = 0; i < m_iNumVertices; i++)
	{
		memcpy(&pVertices[i].vPosition, &pAIMesh->mVertices[i], sizeof(_float3));
		XMStoreFloat3(&pVertices[i].vPosition, XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vPosition), PreTransformMatrix));

		memcpy(&pVertices[i].vNormal, &pAIMesh->mNormals[i], sizeof(_float3));
		XMStoreFloat3(&pVertices[i].vNormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vNormal), PreTransformMatrix));

		memcpy(&pVertices[i].vTangent, &pAIMesh->mTangents[i], sizeof(_float3));
		memcpy(&pVertices[i].vTexcoord, &pAIMesh->mTextureCoords[0][i], sizeof(_float2));
	}

	for (_uint i = 0; i < m_iNumVertices; ++i)
		m_pVertexPositions[i] = pVertices[i].vPosition;

	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBBufferDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;


	size_t vbBytes = m_iNumVertices * m_iVertexStride;
	m_RawVB.resize(vbBytes);
	memcpy(m_RawVB.data(), pVertices, vbBytes);

	Safe_Delete_Array(pVertices);

	return S_OK;
}

HRESULT CMesh::Ready_Anim_Mesh(const aiMesh* pAIMesh, const vector<class CBone*>& Bones)
{
	m_iVertexStride = sizeof(VTXANIMMESH);
	D3D11_BUFFER_DESC			VBBufferDesc{};
	VBBufferDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
	VBBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VBBufferDesc.CPUAccessFlags = /*D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE*/0;
	VBBufferDesc.StructureByteStride = m_iVertexStride;
	VBBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA		VBInitialData{};

	VTXANIMMESH* pVertices = new VTXANIMMESH[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXANIMMESH) * m_iNumVertices);

	m_pVertexPositions = new _float3[m_iNumVertices];
	ZeroMemory(m_pVertexPositions, sizeof(_float3) * m_iNumVertices);

	for (size_t i = 0; i < m_iNumVertices; i++)
	{
		memcpy(&pVertices[i].vPosition, &pAIMesh->mVertices[i], sizeof(_float3));
		memcpy(&pVertices[i].vNormal, &pAIMesh->mNormals[i], sizeof(_float3));
		memcpy(&pVertices[i].vTangent, &pAIMesh->mTangents[i], sizeof(_float3));
		memcpy(&pVertices[i].vTexcoord, &pAIMesh->mTextureCoords[0][i], sizeof(_float2));
	}

	/* 이 정점이 영향을 받는 뼈의 정보를 저장할 때? */
	/* 뭔말인지 알지? */



	m_iNumBones = pAIMesh->mNumBones;

	for (uint32_t i = 0; i < m_iNumBones; i++)
	{
		/* 부모ㅗ 자식등의 뼈의 관계성을 표현(x) -> aiNode */
		/* 이 메시에 어떤 정점들게 영향을 줍니다. and 얼마나 영향을 줍니다. */
		aiBone* pAIBone = pAIMesh->mBones[i];

		_float4x4		OffsetMatrix;

		memcpy(&OffsetMatrix, &pAIBone->mOffsetMatrix, sizeof(_float4x4));

		XMStoreFloat4x4(&OffsetMatrix, XMMatrixTranspose(XMLoadFloat4x4(&OffsetMatrix)));


		m_OffsetMatrices.push_back(OffsetMatrix);

		_uint	iBoneIndex = {};

		// 이름으로 비교해서 전체 뼈중에 현재 메시의 뼈가 몇번째 뼈인지 찾는다.
		auto	iter = find_if(Bones.begin(), Bones.end(), [&](CBone* pBone)->_bool
			{
				if (true == pBone->Compare_Name(pAIBone->mName.data))
					return true;

				++iBoneIndex;

				return false;
			});

		m_BoneIndices.push_back(iBoneIndex);

		/* i번째 뼈가 몇개 정점에게 영향을 주는데?*/
		_uint		iNumWeights = pAIBone->mNumWeights;

		for (_uint j = 0; j < iNumWeights; j++)
		{
			/* i번째 뼈가 영향ㅇ르 주는 j번째 정점의 정보 */
			aiVertexWeight	AIWeight = pAIBone->mWeights[j];

			if (0.f == pVertices[AIWeight.mVertexId].vBlendWeights.x)
			{
				/* 이 메시에게 영향을 주는 뼈들 중 i번째 뼈가 이 저젬에게 영향을 주네. */
				pVertices[AIWeight.mVertexId].vBlendIndices.x = i;
				pVertices[AIWeight.mVertexId].vBlendWeights.x = AIWeight.mWeight;
			}

			else if (0.f == pVertices[AIWeight.mVertexId].vBlendWeights.y)
			{
				pVertices[AIWeight.mVertexId].vBlendIndices.y = i;
				pVertices[AIWeight.mVertexId].vBlendWeights.y = AIWeight.mWeight;
			}

			else if (0.f == pVertices[AIWeight.mVertexId].vBlendWeights.z)
			{
				pVertices[AIWeight.mVertexId].vBlendIndices.z = i;
				pVertices[AIWeight.mVertexId].vBlendWeights.z = AIWeight.mWeight;
			}

			else if (0.f == pVertices[AIWeight.mVertexId].vBlendWeights.w)
			{
				pVertices[AIWeight.mVertexId].vBlendIndices.w = i;
				pVertices[AIWeight.mVertexId].vBlendWeights.w = AIWeight.mWeight;
			}
		}
	}



	if (0 == m_iNumBones)
	{
		m_iNumBones = 1;

		_uint	iBoneIndex = {};

		auto	iter = find_if(Bones.begin(), Bones.end(), [&](CBone* pBone)->_bool
			{
				if (true == pBone->Compare_Name(m_szName))
					return true;

				++iBoneIndex;

				return false;
			});


		m_BoneIndices.push_back(iBoneIndex);

		_float4x4		OffsetMatrix;
		XMStoreFloat4x4(&OffsetMatrix, XMMatrixIdentity());
		m_OffsetMatrices.push_back(OffsetMatrix);
	}

	for (_uint i = 0; i < m_iNumVertices; ++i)
		m_pVertexPositions[i] = pVertices[i].vPosition;

	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBBufferDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;


	size_t vbBytes = m_iNumVertices * m_iVertexStride;
	m_RawVB.resize(vbBytes);
	memcpy(m_RawVB.data(), pVertices, vbBytes);

	Safe_Delete_Array(pVertices);

	return S_OK;
}

HRESULT CMesh::Bind_Bone_Matrices(CShader* pShader, const _char* pConstantName, const vector<class CBone*>& Bones)
{
	ZeroMemory(m_BoneMatrices, sizeof(_float4x4) * g_iMaxNumBones);

	for (size_t i = 0; i < m_iNumBones; i++)
	{
		XMStoreFloat4x4(&m_BoneMatrices[i],
			XMLoadFloat4x4(&m_OffsetMatrices[i]) *
			XMLoadFloat4x4(Bones[m_BoneIndices[i]]->Get_CombinedTransformationMatrix()));
	}

	return pShader->Bind_Matrices(pConstantName, m_BoneMatrices, m_iNumBones);
}

HRESULT CMesh::ExportBinary(ofstream& ofs)
{
	WriteUInt(ofs, 0x4D534845);  // 'MSHE'
	WriteUInt(ofs, (uint32_t)m_iNumVertices);
	WriteUInt(ofs, (uint32_t)m_iVertexStride);
	WriteUInt(ofs, (uint32_t)m_iNumIndices);
	WriteUInt(ofs, (uint32_t)m_iIndexStride);
	WriteUInt(ofs, (uint32_t)m_iNumVertexBuffers);
	uint32_t nameLen = (uint32_t)strlen(m_szName);
	WriteUInt(ofs, nameLen);
	ofs.write(m_szName, nameLen);
	WriteUInt(ofs, m_iMaterialIndex);
	WriteUInt(ofs, (_uint)m_iNumBones);
	WriteUInt(ofs, (_uint)m_BoneIndices.size());
	for (auto& idx : m_BoneIndices) 
		WriteUInt(ofs, idx);
	for (auto& mat : m_OffsetMatrices) 
	{
		ofs.write(reinterpret_cast<const char*>(&mat), sizeof(XMFLOAT4X4));
	}
	ofs.write(reinterpret_cast<const char*>(m_RawVB.data()), m_RawVB.size());
	ofs.write(reinterpret_cast<const char*>(m_RawIB.data()), m_RawIB.size());
	return S_OK;
}

CMesh* CMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, const aiMesh* pAIMesh, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix)
{
	CMesh* pInstance = new CMesh(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(eType, pAIMesh, Bones, PreTransformMatrix)))
	{
		MSG_BOX("Failed to Created : CMesh");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CMesh* CMesh::CreateByBinary(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ifstream& ifs, const vector<CBone*>& bones, const _fmatrix& PreTransformMatrix)
{
	//uint32_t magic;
	//ifs.read(reinterpret_cast<char*>(&magic), sizeof(magic));
	//if (magic != 0x4D534845) // 'MSHE'
	//	return nullptr;

	//// 메타데이터 읽기
	//uint32_t numVertices, vertexStride, numIndices, indexStride, numVBs;
	//ifs.read(reinterpret_cast<char*>(&numVertices), sizeof(numVertices));
	//ifs.read(reinterpret_cast<char*>(&vertexStride), sizeof(vertexStride));
	//ifs.read(reinterpret_cast<char*>(&numIndices), sizeof(numIndices));
	//ifs.read(reinterpret_cast<char*>(&indexStride), sizeof(indexStride));
	//ifs.read(reinterpret_cast<char*>(&numVBs), sizeof(numVBs));

	//// 이름
	//uint32_t nameLen;
	//ifs.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
	//std::string name(nameLen, '\0');
	//ifs.read(&name[0], nameLen);

	//// 재질 인덱스
	//uint32_t materialIndex;
	//ifs.read(reinterpret_cast<char*>(&materialIndex), sizeof(materialIndex));

	//// 본 인덱스 정보
	//uint32_t boneCount, boneIdxCount;
	//ifs.read(reinterpret_cast<char*>(&boneCount), sizeof(boneCount));
	//ifs.read(reinterpret_cast<char*>(&boneIdxCount), sizeof(boneIdxCount));
	//std::vector<int> boneIndices(boneIdxCount);
	//for (uint32_t i = 0; i < boneIdxCount; ++i)
	//	ifs.read(reinterpret_cast<char*>(&boneIndices[i]), sizeof(boneIndices[i]));

	//// raw 버텍스·인덱스 읽기
	//std::vector<uint8_t> vbRaw(numVertices * vertexStride);
	//ifs.read(reinterpret_cast<char*>(vbRaw.data()), vbRaw.size());
	//std::vector<uint8_t> ibRaw(numIndices * indexStride);
	//ifs.read(reinterpret_cast<char*>(ibRaw.data()), ibRaw.size());

	//// 인스턴스 생성
	//CMesh* pMesh = new CMesh(pDevice, pContext);
	//strcpy_s(pMesh->m_szName, nameLen + 1, name.c_str());
	//pMesh->m_iMaterialIndex = materialIndex;
	//if (boneCount > 1) {
	//	pMesh->m_BoneIndices = boneIndices;
	//}

	//// 공통 초기화
	//_bool isAnim = (boneCount > 1);
	//if (FAILED(pMesh->Initialize_FromData(
	//	vbRaw.data(), numVertices, vertexStride,
	//	ibRaw.data(), numIndices, indexStride,
	//	isAnim, bones, PreTransformMatrix))) {
	//	Safe_Release(pMesh);
	//}

	uint32_t magic;
	ifs.read(reinterpret_cast<char*>(&magic), sizeof(magic));
	if (magic != 0x4D534845) // 'MSHE'
		return nullptr;

	// 메타데이터 읽기
	uint32_t numVertices, vertexStride, numIndices, indexStride, numVBs;
	ifs.read(reinterpret_cast<char*>(&numVertices), sizeof(numVertices));
	ifs.read(reinterpret_cast<char*>(&vertexStride), sizeof(vertexStride));
	ifs.read(reinterpret_cast<char*>(&numIndices), sizeof(numIndices));
	ifs.read(reinterpret_cast<char*>(&indexStride), sizeof(indexStride));
	ifs.read(reinterpret_cast<char*>(&numVBs), sizeof(numVBs));

	// 이름
	uint32_t nameLen;
	ifs.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
	string name(nameLen, '\0');
	ifs.read(&name[0], nameLen);

	// 재질 인덱스
	uint32_t materialIndex;
	ifs.read(reinterpret_cast<char*>(&materialIndex), sizeof(materialIndex));

	// 본 정보 개수
	uint32_t boneCount, boneIdxCount;
	ifs.read(reinterpret_cast<char*>(&boneCount), sizeof(boneCount));
	ifs.read(reinterpret_cast<char*>(&boneIdxCount), sizeof(boneIdxCount));

	// 본 인덱스 읽기
	vector<int> boneIndices(boneIdxCount);
	for (uint32_t i = 0; i < boneIdxCount; ++i)
		ifs.read(reinterpret_cast<char*>(&boneIndices[i]), sizeof(boneIndices[i]));

	vector<XMFLOAT4X4> offsetMatrices(boneIdxCount);
	for (uint32_t i = 0; i < boneIdxCount; ++i) {
		ifs.read(reinterpret_cast<char*>(&offsetMatrices[i]), sizeof(XMFLOAT4X4));
	}

	// 버텍스·인덱스 RAW 데이터 읽기
	vector<uint8_t> vbRaw(numVertices * vertexStride);
	ifs.read(reinterpret_cast<char*>(vbRaw.data()), vbRaw.size());
	vector<uint8_t> ibRaw(numIndices * indexStride);
	ifs.read(reinterpret_cast<char*>(ibRaw.data()), ibRaw.size());

	// 인스턴스 생성 및 필드 설정
	CMesh* pMesh = new CMesh(pDevice, pContext);
	// 이름 복사
	strcpy_s(pMesh->m_szName, nameLen + 1, name.c_str());
	// 메타 설정
	pMesh->m_iMaterialIndex = materialIndex;
	pMesh->m_iNumBones = boneCount;           // 본 개수
	pMesh->m_iNumVertexBuffers = numVBs;              // VB 개수
	// 본 인덱스·오프셋 매트릭스 복사
	pMesh->m_BoneIndices = boneIndices;
	pMesh->m_OffsetMatrices = offsetMatrices;

	// 실제 버퍼 생성
	_bool isAnim = (boneCount > 1);
	if (FAILED(pMesh->Initialize_FromData(
		vbRaw.data(), numVertices, vertexStride,
		ibRaw.data(), numIndices, indexStride,
		isAnim, bones, PreTransformMatrix)))
	{
		Safe_Release(pMesh);
	}

	return pMesh;
}


CComponent* CMesh::Clone(void* pArg)
{
	CMesh* pInstance = new CMesh(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMesh");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMesh::Free()
{
	__super::Free();


}
