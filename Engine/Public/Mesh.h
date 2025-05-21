#pragma once

#include "VIBuffer.h"

BEGIN_NAMESPACE(Engine)

class ENGINE_DLL CMesh final : public CVIBuffer
{
private:
	CMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMesh(const CMesh& Prototype);
	virtual ~CMesh() = default;

public:
	_uint Get_MaterialIndex() const {
		return m_iMaterialIndex;
	}

public:
	virtual HRESULT Initialize_Prototype(MODEL eType, const aiMesh* pAIMesh, const vector<class CBone*>& Bones,  _fmatrix PreTransformMatrix);
	virtual HRESULT Initialize(void* pArg);

	HRESULT Initialize_FromData(
		 void* pVertexData,
		UINT vertexCount,
		UINT vertexStride,
		 void* pIndexData,
		UINT indexCount,
		UINT indexStride,
		_bool isAnim,
		const vector<class CBone*>& bones,
		const _fmatrix& PreTransformMatrix
	);


public:
	HRESULT Bind_Bone_Matrices(class CShader* pShader, const _char* pConstantName, const vector<class CBone*>& Bones);

public:
	HRESULT ExportBinary(ofstream& ofs);

private:
	_char			m_szName[MAX_PATH] = {};
	_uint			m_iMaterialIndex = {};

	/* 모델에 선언된 전체뼈(x) */
	/* 전체 뼈들 중, 이 메시에 영향을 주는 뼈들만골라서 모아놓은 컨테이너. */
	_uint			m_iNumBones = { };

	/* 전체를 기준으로 이 메시에 영향을 주는 뼈의 인덱스를 모아놓았다. */
	vector<_int>			m_BoneIndices;
	_float4x4				m_BoneMatrices[g_iMaxNumBones] = {};
	vector<_float4x4>		m_OffsetMatrices;


	vector<uint8_t> m_RawVB;
	vector<uint8_t> m_RawIB;

private:
	HRESULT Ready_NonAnim_Mesh(const aiMesh* pAIMesh, _fmatrix PreTransformMatrix);
	HRESULT Ready_Anim_Mesh(const aiMesh* pAIMesh, const vector<class CBone*>& Bones);

public:
	static CMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, const aiMesh* pAIMesh, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix);
	static CMesh* CreateByBinary(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ifstream& ifs, const vector<CBone*>& bones, const _fmatrix& PreTransformMatrix);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

};

END_NAMESPACE