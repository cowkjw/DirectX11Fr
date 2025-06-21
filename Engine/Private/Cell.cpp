#include "Cell.h"
#include "GameInstance.h"

CCell::CCell(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CCell::Initialize(const _float3* pPoints, _int iIndex)
{
	m_iIndex = iIndex;

	memcpy(m_vPoints, pPoints, sizeof(_float3) * POINT_END);

	_vector		vLine = XMVectorSet(0.f, 0.f, 0.f, 0.f);

	vLine = XMLoadFloat3(&m_vPoints[POINT_B]) - XMLoadFloat3(&m_vPoints[POINT_A]);
	m_vNormals[LINE_AB] = _float3(vLine.m128_f32[2] * -1.f, 0.f, vLine.m128_f32[0]);

	vLine = XMLoadFloat3(&m_vPoints[POINT_C]) - XMLoadFloat3(&m_vPoints[POINT_B]);
	m_vNormals[LINE_BC] = _float3(vLine.m128_f32[2] * -1.f, 0.f, vLine.m128_f32[0]);

	vLine = XMLoadFloat3(&m_vPoints[POINT_A]) - XMLoadFloat3(&m_vPoints[POINT_C]);
	m_vNormals[LINE_CA] = _float3(vLine.m128_f32[2] * -1.f, 0.f, vLine.m128_f32[0]);


#ifdef _DEBUG
	m_pVIBuffer = CVIBuffer_Cell::Create(m_pDevice, m_pContext, pPoints);
	if (nullptr == m_pVIBuffer)
		return E_FAIL;
#endif


	return S_OK;
}

_bool CCell::isIn(_fvector vLocalPos, _int* pNeighborIndex)
{
	for (size_t i = 0; i < LINE_END; i++)
	{
		_vector		vDir = vLocalPos - XMLoadFloat3(&m_vPoints[i]);

		if (0 < XMVectorGetX(XMVector3Dot(XMVector3Normalize(vDir), XMVector3Normalize(XMLoadFloat3(&m_vNormals[i])))))
		{
			*pNeighborIndex = m_iNeighborIndices[i];
			return false;
		}
			
	}

	return true;
}

_bool CCell::Compare(_fvector vSour, _fvector vDest)
{
	/*XMVectorEqual(vSour, vDest);*/
	if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_A]), vSour))
	{
		if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_B]), vDest))
			return true;
		if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_C]), vDest))
			return true;		
	}

	if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_B]), vSour))
	{
		if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_C]), vDest))
			return true;
		if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_A]), vDest))
			return true;
	}

	if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_C]), vSour))
	{
		if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_A]), vDest))
			return true;
		if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_B]), vDest))
			return true;
	}

	return false;
}

_float CCell::Compute_Height(_fvector vLocalPos)
{
	_vector		vPlane = XMPlaneFromPoints(XMLoadFloat3(&m_vPoints[POINT_A]), XMLoadFloat3(&m_vPoints[POINT_B]), XMLoadFloat3(&m_vPoints[POINT_C]));

	return (-vPlane.m128_f32[0] * vLocalPos.m128_f32[0] - vPlane.m128_f32[2] * vLocalPos.m128_f32[2] - vPlane.m128_f32[3]) / vPlane.m128_f32[1];

	// y = (-ax - cz - d) / b


	
}

_bool CCell::Slide(const _vector& oldPos, const _vector& vDelta, _vector& outSlidePos) const
{
	_vector tryPos = oldPos + vDelta;

	// 부딪힌 변 찾기
	_int hitEdge = -1;
	for (_int i = 0; i < LINE_END; ++i)
	{
		_vector P = XMLoadFloat3(&m_vPoints[i]); // 벽의 한 점
		_vector N = XMLoadFloat3(&m_vNormals[i]); // 벽 법선
		// 가려는 위치와 부딪히는 벽에 대한 내적 계산
		_float   d = XMVectorGetX(XMVector3Dot(XMVector3Normalize(tryPos - P),XMVector3Normalize(N)));
		if (d > 0.f) // 부딪히는 벽이면 0보다 크다는건 외부로 나가려고 했다는 것
		{
			hitEdge = i;
			break;
		}
	}
	if (hitEdge < 0)
		return false; // 충돌된 변 없음

	_vector N = XMLoadFloat3(&m_vNormals[hitEdge]);
	_float   dotND = XMVectorGetX(XMVector3Dot(vDelta, N));
	// 이동할 벡터가 Delta - (Delta·N) * N (N은 벽의 법선 벡터) 로 투영
	_vector rawSld = vDelta - N * dotND;  // 현재 크기는 |vDelta|*sinθ

	// 원래 이동량 처리
	_float   origLen = XMVectorGetX(XMVector3Length(vDelta));
	_vector sldDir = XMVector3Normalize(rawSld);
	_vector slide = sldDir * origLen; 
	//const _float THRESH = origLen * 0.1f; // 원래 이동량의 10퍼정도 보정
	////_float slideLen = XMVectorGetX(XMVector3Length(rawSld));
	////if (slideLen < THRESH)
	////	return false; // 슬라이딩 벡터가 너무 작으면 슬라이딩 불필요
	outSlidePos = oldPos + slide*1.5f;
	return true;

	//_vector tryPos = oldPos + vDelta;

	//// 부딪힌 변 찾기
	//_int hitEdge = -1;
	//for (_int i = 0; i < LINE_END; ++i)
	//{
	//	_vector P = XMLoadFloat3(&m_vPoints[i]);
	//	_vector N = XMLoadFloat3(&m_vNormals[i]);
	//	_float d = XMVectorGetX(
	//		XMVector3Dot(
	//			XMVector3Normalize(tryPos - P),
	//			XMVector3Normalize(N)
	//		));
	//	if (d > 0.f)
	//	{
	//		hitEdge = i;
	//		break;
	//	}
	//}

	//if (hitEdge < 0)
	//	return false;

	//_vector N = XMLoadFloat3(&m_vNormals[hitEdge]);
	//_float dotND = XMVectorGetX(XMVector3Dot(vDelta, N));
	//_vector rawSld = vDelta - N * dotND;

	//_float origLen = XMVectorGetX(XMVector3Length(vDelta));
	//_float rawSldLen = XMVectorGetX(XMVector3Length(rawSld));

	//// *** 하이브리드 떨림 방지 처리 ***

	//// 1. 절대 최소값 체크
	//const _float ABS_MIN_THRESH = 0.005f;
	//if (rawSldLen < ABS_MIN_THRESH)
	//{
	//	outSlidePos = oldPos;
	//	return false;
	//}

	//// 2. 상대 최소값 체크 (원래 이동량 대비)
	//const _float REL_MIN_THRESH = origLen * 0.05f; // 5%
	//if (rawSldLen < REL_MIN_THRESH)
	//{
	//	outSlidePos = oldPos;
	//	return false;
	//}

	//// 3. 각도 체크 (거의 정면 충돌 방지)
	//_vector vDeltaNorm = XMVector3Normalize(vDelta);
	//_float cosAngle = fabsf(XMVectorGetX(XMVector3Dot(vDeltaNorm, N)));
	//const _float MAX_COS_ANGLE = cosf(XM_PI / 8.0f); // 22.5도 이하면 정지

	//if (cosAngle > MAX_COS_ANGLE)
	//{
	//	outSlidePos = oldPos;
	//	return false;
	//}

	//// 4. 슬라이딩 벡터 계산 (약간의 감쇠 적용)
	//_vector sldDir = XMVector3Normalize(rawSld);
	//const _float SLIDE_DAMPING = 0.95f; // 5% 감쇠
	//_vector slide = sldDir * (origLen * SLIDE_DAMPING);

	//outSlidePos = oldPos + slide;
	//return true;

//
//_vector tryPos = oldPos + vDelta;
//
//// 부딪힌 변 찾기 (동일)
//_int hitEdge = -1;
//for (_int i = 0; i < LINE_END; ++i)
//{
//	_vector P = XMLoadFloat3(&m_vPoints[i]);
//	_vector N = XMLoadFloat3(&m_vNormals[i]);
//	_float d = XMVectorGetX(
//		XMVector3Dot(
//			XMVector3Normalize(tryPos - P),
//			XMVector3Normalize(N)
//		));
//	if (d > 0.f)
//	{
//		hitEdge = i;
//		break;
//	}
//}
//
//if (hitEdge < 0)
//	return false;
//
//_vector N = XMLoadFloat3(&m_vNormals[hitEdge]);
//_float dotND = XMVectorGetX(XMVector3Dot(vDelta, N));
//
//// *** 떨림 방지 처리 2: 각도 체크 ***
//_float origLen = XMVectorGetX(XMVector3Length(vDelta));
//_vector vDeltaNorm = XMVector3Normalize(vDelta);
//_float cosAngle = XMVectorGetX(XMVector3Dot(vDeltaNorm, N));
//_float angle = acosf(fabsf(cosAngle)); // 0 ~ PI/2
//
//const _float MIN_SLIDE_ANGLE = XM_PI / 5.0f; // 15도
//
//if (angle < MIN_SLIDE_ANGLE)
//{
//	// 거의 정면 충돌이면 완전 정지
//	outSlidePos = oldPos;
//	return false;
//}
//
//_vector rawSld = vDelta - N * dotND;
//_vector sldDir = XMVector3Normalize(rawSld);
//_vector slide = sldDir * origLen;
//
//outSlidePos = oldPos + slide;
//return true;
}

#ifdef _DEBUG
HRESULT CCell::Render()
{
	m_pVIBuffer->Bind_Buffers();
	m_pVIBuffer->Render();

	return S_OK;
}
#endif

CCell* CCell::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _float3* pPoints, _int iIndex)
{
	CCell* pInstance = new CCell(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pPoints, iIndex)))
	{
		MSG_BOX("Failed to Created : CCell");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CCell::Free()
{
	__super::Free();


	Safe_Release(m_pVIBuffer);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
