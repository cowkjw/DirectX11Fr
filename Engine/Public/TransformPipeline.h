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
	void Set_Transform(TRANSNFORM eState, _fmatrix TransformMatrix);
	const _float4x4* Get_Transform_Float4x4(TRANSNFORM eState) const;
	const _matrix Get_Transform_Matrix(TRANSNFORM eState) const;
	const _float4* Get_CamPosition() const;


public:
	void Update();

private:
	_float4x4				m_TransformationMatrices[ToIndex(TRANSNFORM::END)] = {};
	_float4x4				m_TransformationMatrixInverse[ToIndex(TRANSNFORM::END)] = {};
	_float4					m_vCamPosition = {};

public:
	static CTransformPipeline* Create();
	virtual void Free() override;
};

END_NAMESPACE