#pragma once

#include "Component.h"

BEGIN_NAMESPACE(Engine)

class ENGINE_DLL CModel final : public CComponent
{
protected:
	CModel(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CModel(const CModel& Prototype);
	virtual ~CModel() = default;

public:
	_uint Get_NumMeshes() const {
		return m_iNumMeshes;
	}

	const char* GetCurrentAnimName() const;


	void Set_Animation(_uint iIndex, _float fadeDuration = 0.2f, _bool isLoop = false);
	const vector<class CMesh*>& Get_Meshes() const {
		return m_Meshes;
	}


	HRESULT Bind_Material(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex, aiTextureType eType, _uint iTextureIndex = 0);
	HRESULT Bind_Bone_Matrices(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex);

public:
	virtual HRESULT Initialize_Prototype(MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix);
	virtual HRESULT Initialize_PrototypeByBinary(MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix);
	virtual HRESULT Initialize(void* pArg);
	virtual HRESULT Render(_uint iMeshIndex);

public:
	HRESULT ExportBinary(const _char* pFilePath,MODEL eType);

public:
	HRESULT Play_Animation(_float fTimeDelta);

private:
	Assimp::Importer		m_Importer;	

	/* 모델에 대한 모든 정보르,ㄹ 담고 있는 구조체. */
	const aiScene*			m_pAIScene = { nullptr };

	MODEL					m_eType = {};
	_float4x4				m_PreTransformMatrix = {};
	_uint					m_iNumMeshes = {};
	vector<class CMesh*>	m_Meshes;

	_uint						m_iNumMaterials = {};
	vector<class CMaterial*>	m_Materials;

	vector<class CBone*>			m_Bones; // 전체 본의 개수


	_bool						m_isLoop{false};
	_uint						m_iCurrentAnimIndex = { };
	_uint						m_iPrevAnimIndex = { };
	_uint						m_iNumAnimations = {};
	vector<class CAnimation*>	m_Animations;
	unordered_map<string, _uint> m_AnimationMap;
	class CAnimator* m_pAnimator = { nullptr };
	_bool m_bChangeAnim = { false };

public:
	HRESULT Ready_Bones(const aiNode* pAINode, _int iParentBoneIndex);
	HRESULT Ready_Meshes();
	HRESULT Ready_Materials(const _char* pModelFilePath);
	HRESULT Ready_Animations();
public:
	static CModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix = XMMatrixIdentity());
	static CModel* CreateByBinary(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix = XMMatrixIdentity());
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END_NAMESPACE