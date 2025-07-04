#pragma once

#include "Base.h"

/* 렌더링 파이프라인의 변환을 위한 뷰, 투영행려을 보관한다. */
/* 렌더링 파이프라인의 변환을 위한 뷰, 투영행렬의 역행렬을 계산, 보관한다. */
/* 카메라 위치도 보관한ㄷ,ㅏㅡ ./*/

BEGIN_NAMESPACE(Engine)

class CTransformPipeline final : public CBase
{
private:
	CTransformPipeline();
	virtual ~CTransformPipeline() = default;

public:
	void Set_Transform(TRANSFORM eState, _fmatrix TransformMatrix);
	const _float4x4* Get_Transform_Float4x4(TRANSFORM eState) const;
	const _matrix Get_Transform_Matrix(TRANSFORM eState) const;

	const _float4x4* Get_Transform_Float4x4_Inv(TRANSFORM eState) const;
	const _matrix Get_Transform_Matrix_Inv(TRANSFORM eState) const;
	const _float4* Get_CamPosition() const;
	_vector UnprojectToGround(_float mx, _float my, const D3D11_VIEWPORT& vp);
	_float Get_CameraFar() const;


public:
	void Update();

private:
	_float4x4				m_TransformationMatrices[ToIndex(TRANSFORM::END)] = {};
	_float4x4				m_TransformationMatrixInverse[ToIndex(TRANSFORM::END)] = {};
	_float4					m_vCamPosition = {};

public:
	static CTransformPipeline* Create();
	virtual void Free() override;
};

END_NAMESPACE