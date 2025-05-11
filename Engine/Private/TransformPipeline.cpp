#include "TransformPipeline.h"

CTransformPipeline::CTransformPipeline()
{
}

void CTransformPipeline::Set_Transform(TRANSFORM eState, _fmatrix TransformMatrix)
{
	XMStoreFloat4x4(&m_TransformationMatrices[ToIndex(eState)], TransformMatrix);
}

const _float4x4* CTransformPipeline::Get_Transform_Float4x4(TRANSFORM eState) const
{
	return &m_TransformationMatrices[ToIndex(eState)];
}

const _matrix CTransformPipeline::Get_Transform_Matrix(TRANSFORM eState) const
{
	return XMLoadFloat4x4(&m_TransformationMatrices[ToIndex(eState)]);
}

const _float4* CTransformPipeline::Get_CamPosition() const
{
	return &m_vCamPosition;
}

void CTransformPipeline::Update()
{
	for (_uint i = 0; i < ToIndex(TRANSFORM::END); i++)
	{
		XMStoreFloat4x4(&m_TransformationMatrixInverse[i],
			XMMatrixInverse(nullptr, Get_Transform_Matrix(static_cast<TRANSFORM>(i))));
	}

	memcpy(&m_vCamPosition, &m_TransformationMatrixInverse[ToIndex(TRANSFORM::VIEW)].m[3], sizeof(_float4));
}

CTransformPipeline* CTransformPipeline::Create()
{
	return new CTransformPipeline();
}

void CTransformPipeline::Free()
{
	__super::Free();
}
