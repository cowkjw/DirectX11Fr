#include "Bone.h"

CBone::CBone()
{
}

HRESULT CBone::Initialize(const aiNode* pAINode, _int iParentBoneIndex)
{
	strcpy_s(m_szName, pAINode->mName.data); // 이름 복사

	memcpy(&m_TransformationMatrix, &pAINode->mTransformation, sizeof(_float4x4)); // 기본 변환 행렬 복사

	XMStoreFloat4x4(&m_TransformationMatrix, XMMatrixTranspose(XMLoadFloat4x4(&m_TransformationMatrix))); // 전치 행렬로 변환

	m_LocalBindPoseMatrix = m_TransformationMatrix; // 로컬 행렬 설정 나중에 블렌드에 사용하기

	XMStoreFloat4x4(&m_CombinedTransformationMatrix, XMMatrixIdentity()); // 나중에 애니메이션 정렬을 위해서 초기화

	m_iParentBoneIndex = iParentBoneIndex; // 내 부모 뼈의 인덱스

	return S_OK;
}

void CBone::Update_CombinedTransformationMatrix(const vector<CBone*>& Bones, _fmatrix PreTransformMatrix)
{
	if (-1 == m_iParentBoneIndex)// 부모가 없으면
		XMStoreFloat4x4(&m_CombinedTransformationMatrix, XMLoadFloat4x4(&m_TransformationMatrix) * PreTransformMatrix);

	else // 부모가 있으면

	// 부모의 변환 행렬과 내 변환 행렬을 곱해준다.
	{
		_matrix trasformationMatrix = XMMatrixMultiply(XMLoadFloat4x4(&m_TransformationMatrix), m_vExtraMatix);
		XMStoreFloat4x4(&m_CombinedTransformationMatrix,
			trasformationMatrix * XMLoadFloat4x4(&Bones[m_iParentBoneIndex]->m_CombinedTransformationMatrix));

		Reset_ExtraMatrix();
	}
}



CBone* CBone::Create(const aiNode* pAINode, _int iParentBoneIndex)
{
	CBone* pInstance = new CBone();

	if (FAILED(pInstance->Initialize(pAINode, iParentBoneIndex)))
	{
		MSG_BOX("Failed to Created : CBone");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBone::ResetBones()
{
	XMStoreFloat4x4(&m_CombinedTransformationMatrix, XMLoadFloat4x4(&m_TransformationMatrix));
}

HRESULT CBone::ExportBinary(ofstream& ofs)
{
	uint32_t nameLen = (uint32_t)strlen(m_szName);
	ofs.write((char*)&nameLen, sizeof(nameLen));
	ofs.write(m_szName, nameLen);
	ofs.write((char*)&m_iParentBoneIndex, sizeof(m_iParentBoneIndex));
	ofs.write((char*)&m_TransformationMatrix, sizeof(_float4x4));

	return S_OK;
}

CBone* CBone::CreateByBinary(ifstream& ifs)
{
	// 1) 이름 길이 + 이름 읽기
	uint32_t nameLen;
	ifs.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
	std::string name(nameLen, '\0');
	ifs.read(&name[0], nameLen);

	// 2) 부모 인덱스 읽기
	int32_t parentIndex;
	ifs.read(reinterpret_cast<char*>(&parentIndex), sizeof(parentIndex));

	// 3) 변환 행렬 읽기
	_float4x4 tm;
	ifs.read(reinterpret_cast<char*>(&tm), sizeof(_float4x4));

	// 4) 객체 생성 및 멤버 설정 :contentReference[oaicite:2]{index=2}:contentReference[oaicite:3]{index=3}
	CBone* pBone = new CBone();
	strcpy_s(pBone->m_szName, nameLen + 1, name.c_str());
	pBone->m_iParentBoneIndex = parentIndex;
	pBone->m_TransformationMatrix = tm;
	pBone->m_LocalBindPoseMatrix = pBone->m_TransformationMatrix; // 로컬 행렬 설정 나중에 블렌드에 사용하기
	// 초기 Combined 매트릭스는 Identity
	XMStoreFloat4x4(&pBone->m_CombinedTransformationMatrix, XMMatrixIdentity());

	return pBone;
}

CBone* CBone::Clone()
{
	return new CBone(*this);	
}

void CBone::Free()
{
	__super::Free();


}
