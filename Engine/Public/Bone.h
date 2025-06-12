#pragma once

#include "Base.h"

/* 뼈. (aiNode, aiBone, aiNodeAnim) */
/* aiNode를 이용해서 셋팅한 뼈 정보. */

BEGIN_NAMESPACE(Engine)

class CBone final : public CBase
{
private:
	CBone();
	virtual ~CBone() = default;

public:
	const _float4x4* Get_CombinedTransformationMatrix() const {
		return &m_CombinedTransformationMatrix;
	}


	void Set_TransformationMatrix(_fmatrix TransformationMatrix) {
		XMStoreFloat4x4(&m_TransformationMatrix, TransformationMatrix);
	}


public:
	HRESULT Initialize(const aiNode* pAINode, _int iParentBoneIndex);
	void Update_CombinedTransformationMatrix(const vector<CBone*>& Bones, _fmatrix PreTransformMatrix);
	_bool Compare_Name(const _char* pName) {
		return !strcmp(m_szName, pName);
	}

	const _float4x4& Get_LocalBindPose() const { return m_LocalBindPoseMatrix; }
	const _float4x4& Get_TransformationMatrix() const {
		return m_TransformationMatrix;
	}

	void Set_ExtraScale(const _float3& vExtraScale) {
		m_vExtraScale = vExtraScale;
	}
	const _float3& Get_ExtraScale() const {
		return m_vExtraScale;
	}
	void Set_ExtraMatrix(const _matrix& vExtraMatrix) {
		m_vExtraMatix = vExtraMatrix;
	}
	void Reset_ExtraMatrix() {
		m_vExtraMatix = XMMatrixIdentity();
	}
	void ResetBones();

public:
	HRESULT ExportBinary(ofstream& ofs);

private:
	_char					m_szName[MAX_PATH] = {};

	/* (A : 이 뼈 자체의 원점기준 변환정보를 표현한 행렬) */
	_float4x4				m_TransformationMatrix = {};

	/* (A : 이 뼈 자체의 원점기준 변환정보를 표현한 행렬) * 부모행렬. = A: 부모를 기준으로 회전한다. */
	_float4x4				m_CombinedTransformationMatrix = {};	
	_matrix                 m_vExtraMatix{ XMMatrixIdentity() };

	_int					m_iParentBoneIndex = { -1 };

	_float4x4  m_LocalBindPoseMatrix = {};
	_float3    m_vExtraScale{1.f,1.f,1.f};

public:
	static CBone* Create(const aiNode* pAINode, _int iParentBoneIndex);
	static CBone* CreateByBinary(ifstream& ifs);
	CBone* Clone();
	virtual void Free() override;

};

END_NAMESPACE