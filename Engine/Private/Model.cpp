#include "Component.h"

#include "Animation.h"
#include "Animator.h"
#include "Material.h"
#include "Model.h"
#include "Mesh.h"
#include "Bone.h"

CModel::CModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{

}

CModel::CModel(const CModel& Prototype)
	: CComponent(Prototype)
	, m_iNumMeshes{ Prototype.m_iNumMeshes }
	, m_Meshes{ Prototype.m_Meshes }
	, m_iNumMaterials{ Prototype.m_iNumMaterials }
	, m_Materials{ Prototype.m_Materials }
	, m_eType{ Prototype.m_eType }
	, m_PreTransformMatrix{ Prototype.m_PreTransformMatrix }
	, m_AnimationMap{ Prototype.m_AnimationMap }
	,m_AnimationNameMap{Prototype.m_AnimationNameMap}
	, m_iNumAnimations{ Prototype.m_iNumAnimations } 
{

	for (auto& pBone : Prototype.m_Bones)
		m_Bones.push_back(pBone->Clone());

	for (auto& pAnimation : Prototype.m_Animations)
		m_Animations.push_back(pAnimation->Clone(m_Bones));

	for (auto& pMaterial : m_Materials)
		Safe_AddRef(pMaterial);

	for (auto& pMesh : m_Meshes)
		Safe_AddRef(pMesh);
}




HRESULT CModel::Bind_Material(CShader* pShader, const _char* pConstantName, _uint iMeshIndex, aiTextureType eType, _uint iTextureIndex)
{
	if (iMeshIndex >= m_iNumMeshes)
		return E_FAIL;

	_uint		iMaterialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();

	if (iMaterialIndex >= m_iNumMaterials)
		return E_FAIL;

	return m_Materials[iMaterialIndex]->Bind_ShaderResource(pShader, pConstantName, eType, iTextureIndex);
}

HRESULT CModel::Bind_Bone_Matrices(CShader* pShader, const _char* pConstantName, _uint iMeshIndex)
{
	return m_Meshes[iMeshIndex]->Bind_Bone_Matrices(pShader, pConstantName, m_Bones);
}

HRESULT CModel::Initialize_Prototype(MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix)
{

	_uint		iFlag = aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast;

	//if (MODEL::NONANIM == eType)
	//	iFlag |= aiProcess_PreTransformVertices; // 정점들을 로컬 상에서 초기화해준다.

	m_pAIScene = m_Importer.ReadFile(pModelFilePath, iFlag);

	if (nullptr == m_pAIScene)
		return E_FAIL;

	XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix); // 돌아가거나 하는 모델들을 나중에 돌려주기 위해서 미리 행렬을 받아서 저장해둔다.

	m_eType = eType;

	if (FAILED(Ready_Bones(m_pAIScene->mRootNode, -1)))
		return E_FAIL;

	if (FAILED(Ready_Meshes()))
		return E_FAIL;

	if (FAILED(Ready_Materials(pModelFilePath)))
		return E_FAIL;

	if (FAILED(Ready_Animations()))
		return E_FAIL;

	//m_pAnimator = CAnimator::Create(this, m_Bones);
	//if (nullptr == m_pAnimator)
	//	return E_FAIL;
	//m_pAnimator->Set_CurrentAnim(m_Animations[0]);
	return S_OK;
}

HRESULT CModel::Initialize_PrototypeByBinary(MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix)
{
	ifstream ifs(pModelFilePath, ios::binary);
	if (!ifs.is_open())
		return E_FAIL;

	// 1) 헤더 매직 ('MBIN') :contentReference[oaicite:6]{index=6}:contentReference[oaicite:7]{index=7}
	uint32_t magic;
	ifs.read(reinterpret_cast<char*>(&magic), sizeof(magic));
	if (magic != 0x4D42494E) // 'MBIN'
		return E_FAIL;

	// 2) Materials
	uint32_t matCount;
	ifs.read(reinterpret_cast<char*>(&matCount), sizeof(matCount));
	vector<CMaterial*> materials;
	materials.reserve(matCount);
	for (uint32_t i = 0; i < matCount; ++i)
		materials.push_back(CMaterial::CreateByBinary(m_pDevice, m_pContext, ifs));

	// 3) Bones
	uint32_t boneCount;
	ifs.read(reinterpret_cast<char*>(&boneCount), sizeof(boneCount));
	vector<CBone*> bones;
	bones.reserve(boneCount);
	for (uint32_t i = 0; i < boneCount; ++i)
		bones.push_back(CBone::CreateByBinary(ifs));

	// 4) Meshes
	uint32_t meshCount;
	ifs.read(reinterpret_cast<char*>(&meshCount), sizeof(meshCount));
	vector<CMesh*> meshes;
	meshes.resize(meshCount);
	for (uint32_t i = 0; i < meshCount; ++i)
		meshes[i] = CMesh::CreateByBinary(m_pDevice, m_pContext, ifs, bones, PreTransformMatrix);

	// 5) 모델 타입
	uint32_t type;
	ifs.read(reinterpret_cast<char*>(&type), sizeof(type));

	// 6) 애니메이션
	if (type == static_cast<uint32_t>(MODEL::ANIM))
	{
		//uint32_t animCount;
		//ifs.read(reinterpret_cast<char*>(&animCount), sizeof(animCount));
		//for (uint32_t i = 0; i < animCount; ++i)
		//{
		//	CAnimation* pAnimation = CAnimation::CreateByBinary(ifs, bones);
		//	if (nullptr == pAnimation)
		//		return E_FAIL;
		//	m_Animations.push_back(pAnimation);
		//	//m_AnimationMap[m_Animations.back()->Get_Name()] = m_Animations.size() - 1;
		//}
		//this->m_iNumAnimations = (uint32_t)m_Animations.size();

		string full = pModelFilePath;
		size_t pos = full.find_last_of("\\/");
		string dir = (pos != string::npos) ? full.substr(0, pos + 1) : "./";

		// 1) 검색 패턴 (예: "C:/Models/*.anim.bin")
		string pattern = dir + "*.anim.bin";

		WIN32_FIND_DATAA findData;
		HANDLE hFind = FindFirstFileA(pattern.c_str(), &findData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				// 2) 찾은 파일 이름(build.cFileName)과 디렉터리 결합
				string animPath = dir + findData.cFileName;

				// 3) 파일 열기
				ifstream ifs(animPath, ios::binary);
				if (!ifs.is_open())
					continue;

				// 4) CAnimation 생성 (bones는 이미 채워져 있음)
				CAnimation* pAnim = CAnimation::CreateByBinary(ifs, this->m_Bones);
				if (pAnim)
				{
					this->m_Animations.push_back(pAnim);
					this->m_AnimationMap[pAnim->Get_Name()] =
						static_cast<uint32_t>(this->m_Animations.size() - 1);
					this->m_AnimationNameMap[static_cast<uint32_t>(this->m_Animations.size() - 1)] = pAnim->Get_Name();
				}

			} while (FindNextFileA(hFind, &findData));  // 다음 파일 검색

			FindClose(hFind);
		}

		// 5) 총 개수 업데이트
		this->m_iNumAnimations = static_cast<uint32_t>(this->m_Animations.size());
	}

	this->m_Materials = materials;
	this->m_iNumMaterials = (uint32_t)materials.size();
	this->m_Bones = bones;
	this->m_iNumMeshes = meshCount;
	this->m_Meshes = meshes;
	this->m_eType = static_cast<MODEL>(type);
	XMStoreFloat4x4(&this->m_PreTransformMatrix, PreTransformMatrix);


	//m_pAnimator = CAnimator::Create(this, this->m_Bones);

	//if (nullptr == m_pAnimator)
	//	return E_FAIL;

	//m_pAnimator->Set_CurrentAnim(m_Animations[0]);

	return S_OK;
}

HRESULT CModel::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CModel::Render(_uint iMeshIndex)
{
	m_Meshes[iMeshIndex]->Bind_Buffers();
	m_Meshes[iMeshIndex]->Render();

	return S_OK;
}

HRESULT CModel::ExportBinary(const _char* pFilePath, MODEL eType)
{


	ofstream ofs(pFilePath, ios::binary);
	if (!ofs) return E_FAIL;

	// 헤더(고유 매직 넘버)
	uint32_t magic = 0x4D42494E; // 'MBIN'
	ofs.write((char*)&magic, sizeof(magic));

	// 1) Materials
	ofs.write((char*)&m_iNumMaterials, sizeof(m_iNumMaterials));
	for (auto& mat : m_Materials)
		mat->ExportBinary(ofs);

	// 2) Bones
	uint32_t boneCount = (uint32_t)m_Bones.size();
	ofs.write((char*)&boneCount, sizeof(boneCount));
	for (auto& bone : m_Bones)
		bone->ExportBinary(ofs);

	// 3) Meshes (필요하다면)
	ofs.write((char*)&m_iNumMeshes, sizeof(m_iNumMeshes));
	for (auto& mesh : m_Meshes)
		mesh->ExportBinary(ofs);

	uint32_t type = (uint32_t)eType;
	ofs.write((char*)&type, sizeof(type));

	if (eType == MODEL::ANIM)
	{
		//// 애니메이션 관련 데이터 저장
		//ofs.write((char*)&m_iNumAnimations, sizeof(m_iNumAnimations));
		//for (auto& anim : m_Animations)
		//	anim->ExportBinary(ofs);

		string full = pFilePath;
		size_t pos = full.find_last_of("\\/");
		string dir = (pos != string::npos) ? full.substr(0, pos + 1) : "";

		for (auto& anim : m_Animations)
		{
			// animFile = "run.anim.bin"
			string animFile = anim->Get_Name();
			string animPath = dir + animFile + ".anim.bin";

			// 실제 애니 바이너리만 Export
			ofstream aos(animPath, ios::binary);
			if (aos)
				anim->ExportBinary(aos);  // CAnimation::ExportBinary
		}
	}


	return S_OK;

}

HRESULT CModel::Play_Animation(_float fTimeDelta)
{
	//if (m_pAnimator)
	//{
	//	m_pAnimator->Update(fTimeDelta);
	//}

	//m_Animations[m_iCurrentAnimIndex]->Update_Bones(fTimeDelta, m_Bones, m_isLoop);

	/* 2. 전체 뼐르 순회하면서 뼈들의 ColmbinedTransformationMatixf를 부모에서부터 자식으로 갱신해주낟. */
	for (auto& pBone : m_Bones)
	{
		pBone->Update_CombinedTransformationMatrix(m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));
	}

	return S_OK;
}


HRESULT CModel::Ready_Bones(const aiNode* pAINode, _int iParentBoneIndex)
{
	CBone* pBone = CBone::Create(pAINode, iParentBoneIndex);
	if (nullptr == pBone)
		return E_FAIL;

	m_Bones.push_back(pBone);

	_int		iParentIndex = static_cast<_uint>(m_Bones.size()) - 1;

	for (_uint i = 0; i < pAINode->mNumChildren; i++)
	{
		Ready_Bones(pAINode->mChildren[i], iParentIndex);
	}
	return S_OK;
}

HRESULT CModel::Ready_Meshes()
{
	m_iNumMeshes = m_pAIScene->mNumMeshes;

	for (size_t i = 0; i < m_iNumMeshes; i++)
	{
		CMesh* pMesh = CMesh::Create(m_pDevice, m_pContext, m_eType, m_pAIScene->mMeshes[i], m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));
		if (nullptr == pMesh)
			return E_FAIL;

		m_Meshes.push_back(pMesh);
	}

	return S_OK;
}

HRESULT CModel::Ready_Materials(const _char* pModelFilePath)
{
	m_iNumMaterials = m_pAIScene->mNumMaterials;

	for (size_t i = 0; i < m_iNumMaterials; i++)
	{
		CMaterial* pMaterial = CMaterial::Create(m_pDevice, m_pContext, pModelFilePath, m_pAIScene->mMaterials[i]);
		if (nullptr == pMaterial)
			return E_FAIL;

		m_Materials.push_back(pMaterial);
	}
	return S_OK;
}

HRESULT CModel::Ready_Animations()
{
	m_iNumAnimations = m_pAIScene->mNumAnimations;

	for (_uint i = 0; i < m_iNumAnimations; i++)
	{
		CAnimation* pAnimation = CAnimation::Create(m_pAIScene->mAnimations[i], m_Bones);
		if (nullptr == pAnimation)
			return E_FAIL;

		m_Animations.push_back(pAnimation);
		m_AnimationMap[m_pAIScene->mAnimations[i]->mName.C_Str()] = i;
	}

	return S_OK;
}

CModel* CModel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix)
{
	CModel* pInstance = new CModel(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(eType, pModelFilePath, PreTransformMatrix)))
	{
		MSG_BOX("Failed to Created : CModel");
		Safe_Release(pInstance);
	}

	// Export the model to binary format
	string filePath = pModelFilePath;
	if (filePath.find(".fbx") != string::npos)
	{
		filePath.replace(filePath.find(".fbx"), 4, ".bin");
	}
	pInstance->ExportBinary(filePath.c_str(), eType);

	return pInstance;
}

CModel* CModel::CreateByBinary(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix)
{
	CModel* pInstance = new CModel(pDevice, pContext);
	if (FAILED(pInstance->Initialize_PrototypeByBinary(eType, pModelFilePath, PreTransformMatrix)))
	{
		MSG_BOX("Failed to Created : CModel");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CComponent* CModel::Clone(void* pArg)
{
	CModel* pInstance = new CModel(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CModel");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CModel::Free()
{
	__super::Free();

	for (auto& pBone : m_Bones)
		Safe_Release(pBone);

	for (auto& pMaterial : m_Materials)
		Safe_Release(pMaterial);

	for (auto& pMesh : m_Meshes)
		Safe_Release(pMesh);

	for (auto& pAnimation : m_Animations)
		Safe_Release(pAnimation);

	Safe_Release(m_pAnimator);

	m_Meshes.clear();

	m_Importer.FreeScene();
}
