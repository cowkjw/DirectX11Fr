#include "VIBuffer_Instance.h"

CVIBuffer_Instance::CVIBuffer_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer { pDevice, pContext }
{
}

CVIBuffer_Instance::CVIBuffer_Instance(const CVIBuffer_Instance& Prototype)
	: CVIBuffer{ Prototype }
	, m_VBInstanceDesc { Prototype.m_VBInstanceDesc }
	, m_VBInstanceSubresourceData{ Prototype.m_VBInstanceSubresourceData }
	, m_iNumInstance { Prototype.m_iNumInstance }
	, m_iVertexInstanceStride { Prototype.m_iVertexInstanceStride }
	, m_iNumIndexPerInstance { Prototype.m_iNumIndexPerInstance }
{
	m_pVBInstance = nullptr;
}

HRESULT CVIBuffer_Instance::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CVIBuffer_Instance::Initialize(void* pArg)
{

	return S_OK;
}

HRESULT CVIBuffer_Instance::Bind_Buffers()
{
	ID3D11Buffer* pVertexBuffers[] = {
		   m_pVB,
		   m_pVBInstance,		   
	};

	_uint		iVertexStrides[] = {
		m_iVertexStride,
		m_iVertexInstanceStride

	};

	_uint		iOffsets[] = {
		0, 
		0
	};

	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);

		m_pContext->IASetIndexBuffer(m_pIB, m_eIndexFormat, 0);

	m_pContext->IASetPrimitiveTopology(m_ePrimitiveTopology);


	return S_OK;
}

HRESULT CVIBuffer_Instance::Render()
{	

	m_pContext->DrawIndexedInstanced(m_iNumIndexPerInstance, m_iNumInstance, 0, 0, 0);
	ID3D11Buffer* nullVBs[2] = { nullptr, nullptr };
	UINT         zeroStrides[2] = { 0, 0 };
	UINT         zeroOffsets[2] = { 0, 0 };
	// 슬롯 0·1에 바인딩된 버퍼를 전부 nullptr로 클리어
	m_pContext->IASetVertexBuffers(0, 2, nullVBs, zeroStrides, zeroOffsets);
	// (필요하다면 인덱스 버퍼도 해제)
	m_pContext->IASetIndexBuffer(nullptr, m_eIndexFormat, 0);
	return S_OK;
}

void CVIBuffer_Instance::Free()
{
	__super::Free();

	Safe_Release(m_pVBInstance);

}
