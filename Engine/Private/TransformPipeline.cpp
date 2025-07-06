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

const _float4x4* CTransformPipeline::Get_Transform_Float4x4_Inv(TRANSFORM eState) const
{
	return &m_TransformationMatrixInverse[ToIndex(eState)];
}

const _matrix CTransformPipeline::Get_Transform_Matrix_Inv(TRANSFORM eState) const
{
	return XMLoadFloat4x4(&m_TransformationMatrixInverse[ToIndex(eState)]);
}

const _float4* CTransformPipeline::Get_CamPosition() const
{
	return &m_vCamPosition;
}

_vector CTransformPipeline::UnprojectToGround(_float mx, _float my, const D3D11_VIEWPORT& vp) 
{
   _matrix view = Get_Transform_Matrix(TRANSFORM::VIEW);
   _matrix proj = Get_Transform_Matrix(TRANSFORM::PROJECTION);
   _matrix world = XMMatrixIdentity();  // 필요시 월드 행렬 사용

    // 2) 근/원 평면에서의 점 계산
   _vector nearPt = XMVector3Unproject(
        XMVectorSet(mx, my, 0.f, 1.f),
        vp.TopLeftX, vp.TopLeftY, vp.Width, vp.Height,
        vp.MinDepth, vp.MaxDepth,
        proj, view, world
    );
   _vector farPt = XMVector3Unproject(
        XMVectorSet(mx, my, 1.f, 1.f),
        vp.TopLeftX, vp.TopLeftY, vp.Width, vp.Height,
        vp.MinDepth, vp.MaxDepth,
        proj, view, world
    );

    // Ray 방향
   _vector dir = XMVector3Normalize(farPt - nearPt);

    // 4) Y=0 평면과 교차 t 계산: nearPt.y + dir.y * t = 0
   _float oy = XMVectorGetY(nearPt);
   _float dy = XMVectorGetY(dir);
    if (fabsf(dy) < 1e-6f) return XMVectorZero();  // 평행 시 무효

    _float t = -oy / dy;
    return nearPt + dir * t;
}

 _float CTransformPipeline::Get_CameraFar() const
{
	_matrix proj = Get_Transform_Matrix(TRANSFORM::PROJECTION);

	_float A = proj.r[2].m128_f32[2];       //   zf/(zf-zn)
	_float B = proj.r[3].m128_f32[2];       //  -zn*zf/(zf-zn)

	_float farPlane = -B / (A - 1.f);       // = zf
	return farPlane;
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
