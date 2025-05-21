#pragma once

#include "Component.h"

BEGIN_NAMESPACE(Engine)

class ENGINE_DLL CVIBuffer abstract : public CComponent
{
protected:
	CVIBuffer(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CVIBuffer(const CVIBuffer& Prototype);
	virtual ~CVIBuffer() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);

	virtual HRESULT Bind_Buffers();
	virtual HRESULT Render();
	
public:
	_bool Compute_PickedPosition(const _matrix& pWorldMatrixInverse, _float3& outPickedPos);

protected:
	ID3D11Buffer* m_pVB = { nullptr };
	ID3D11Buffer* m_pIB = { nullptr };

protected:
	_float3*					m_pVertexPositions = {};

protected:
	_uint						m_iNumVertices = {}; // 정점 개수
	_uint						m_iVertexStride = {}; // 정점 크기
	_uint						m_iNumIndices = {}; // 인덱스 개수
	_uint						m_iIndexStride = {}; // 인덱스 크기
	_uint						m_iNumVertexBuffers = {}; // 정점 버퍼 개수
	DXGI_FORMAT					m_eIndexFormat = {}; // 인덱스 포맷
	D3D11_PRIMITIVE_TOPOLOGY	m_ePrimitiveTopology = {}; // 프리미티브 토폴로지


	_uint						m_iNumPritimive = {}; // 프리미티브 개수
	void* m_pIndices = {}; // 인덱스 데이터

public:
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;

};

END_NAMESPACE