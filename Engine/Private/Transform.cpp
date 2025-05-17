#include "Transform.h"

#include "Shader.h"

CTransform::CTransform(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CTransform::CTransform(const CTransform& Prototype)
	: CComponent( Prototype )
	, m_WorldMatrix{ Prototype.m_WorldMatrix }
	, m_bDirty{ Prototype.m_bDirty }
{
}

_float3 CTransform::Get_Scaled()
{
	return _float3(XMVectorGetX(XMVector3Length(Get_State(STATE::RIGHT))),
		XMVectorGetX(XMVector3Length(Get_State(STATE::UP))),
		XMVectorGetX(XMVector3Length(Get_State(STATE::LOOK))));
}

HRESULT CTransform::Initialize_Prototype()
{
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());

	return S_OK;
}

HRESULT CTransform::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return S_OK;

	TRANSFORM_DESC* pDesc = static_cast<TRANSFORM_DESC*>(pArg);

	m_fSpeedPerSec = pDesc->fSpeedPerSec;
	m_fRotationPerSec = pDesc->fRotationPerSec;

	return S_OK;
}

void CTransform::Scaling(_float fX, _float fY, _float fZ)
{
	Set_State(STATE::RIGHT, XMVector3Normalize(Get_State(STATE::RIGHT)) * fX);
	Set_State(STATE::UP, XMVector3Normalize(Get_State(STATE::UP)) * fY);
	Set_State(STATE::LOOK, XMVector3Normalize(Get_State(STATE::LOOK)) * fZ);
}

void CTransform::Scaling(const _float3& vScale)
{
	Set_State(STATE::RIGHT, XMVector3Normalize(Get_State(STATE::RIGHT)) * vScale.x);
	Set_State(STATE::UP, XMVector3Normalize(Get_State(STATE::UP)) * vScale.y);
	Set_State(STATE::LOOK, XMVector3Normalize(Get_State(STATE::LOOK)) * vScale.z);
}

void CTransform::Go_Straight(_float fTimeDelta)
{
	_vector		vPosition = Get_State(STATE::POSITION);
	_vector		vLook = Get_State(STATE::LOOK);

	vPosition += XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;

	Set_State(STATE::POSITION, vPosition);
	m_bDirty = true;
}

void CTransform::Go_Backward(_float fTimeDelta)
{
	_vector		vPosition = Get_State(STATE::POSITION);
	_vector		vLook = Get_State(STATE::LOOK);

	vPosition -= XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;

	Set_State(STATE::POSITION, vPosition);
	m_bDirty = true;
}

void CTransform::Go_Right(_float fTimeDelta)
{
	_vector		vPosition = Get_State(STATE::POSITION);
	_vector		vRight = Get_State(STATE::RIGHT);

	vPosition += XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;

	Set_State(STATE::POSITION, vPosition);
	m_bDirty = true;
}

void CTransform::Go_Left(_float fTimeDelta)
{
	_vector		vPosition = Get_State(STATE::POSITION);
	_vector		vRight = Get_State(STATE::RIGHT);

	vPosition -= XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;

	Set_State(STATE::POSITION, vPosition);
	m_bDirty = true;
}

void CTransform::Follow_Target(_fvector vTarget, _float fTimeDelta, _float fMinDistance)
{
	_vector		vMoveDir = vTarget - Get_State(STATE::POSITION);

	if (fMinDistance <= XMVectorGetX(XMVector3Length(vMoveDir)))
		Set_State(STATE::POSITION, Get_State(STATE::POSITION) + XMVector3Normalize(vMoveDir) * m_fSpeedPerSec * fTimeDelta);
	m_bDirty = true;
}

void CTransform::Turn(_fvector vAxis, _float fTimeDelta)
{
	_matrix			RotationMatrix = XMMatrixRotationAxis(vAxis, m_fRotationPerSec * fTimeDelta);

	Set_State(STATE::RIGHT, XMVector4Transform(Get_State(STATE::RIGHT), RotationMatrix));
	Set_State(STATE::UP, XMVector4Transform(Get_State(STATE::UP), RotationMatrix));
	Set_State(STATE::LOOK, XMVector4Transform(Get_State(STATE::LOOK), RotationMatrix));
	m_bDirty = true;
}

void CTransform::FlllowParent(const CTransform* pParentTransform)
{
	if (nullptr == pParentTransform)
		return;
	XMMATRIX parentW = XMLoadFloat4x4(&pParentTransform->m_WorldMatrix);

	// 2) 자식 로컬 정보
	//   - 스케일
	XMFLOAT3 localScale = Get_Scaled();            // { sx, sy, sz }
	//   - 위치
	XMVECTOR localPos = Get_State(STATE::POSITION);

	// 3) 부모 축(회전+스케일) 꺼내기
	XMVECTOR parentRight = parentW.r[0];  // 부모 X축 벡터 (이미 스케일 포함)
	XMVECTOR parentUp = parentW.r[1];
	XMVECTOR parentLook = parentW.r[2];

	// 4) 월드 위치 계산
	XMVECTOR worldPos = XMVector3TransformCoord(localPos, parentW);

	// 5) 월드 축 계산 (부모 축(normalize) × 자식 로컬 스케일)
	XMVECTOR worldRight = XMVectorScale(XMVector3Normalize(parentRight), localScale.x);
	XMVECTOR worldUp = XMVectorScale(XMVector3Normalize(parentUp), localScale.y);
	XMVECTOR worldLook = XMVectorScale(XMVector3Normalize(parentLook), localScale.z);

	// 6) 최종 월드 매트릭스 구성 & 저장
	XMMATRIX world;
	world.r[0] = worldRight;  // X축
	world.r[1] = worldUp;     // Y축
	world.r[2] = worldLook;   // Z축
	world.r[3] = worldPos;    // 위치
	XMStoreFloat4x4(&m_WorldMatrix, world);
}

json CTransform::Serialize()
{
	json j = CComponent::Serialize();
	XMFLOAT3 p;
	XMStoreFloat3(&p, Get_State(STATE::POSITION));
	j["Position"] = { p.x, p.y, p.z };

	XMFLOAT3 s = Get_Scaled();
	j["Scale"] = { s.x, s.y, s.z };


	return j;
}

void CTransform::Deserialize(const json& j)
{
	// 월드 매트릭스 초기화
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());

	// 1) 위치 복원
	if (j.contains("Position"))
	{
		_float3 vPosition = {
			j["Position"][0].get<_float>(),
			j["Position"][1].get<_float>(),
			j["Position"][2].get<_float>()
		};

		XMVECTOR vecPosition = XMLoadFloat3(&vPosition);

		// 3) w를 1.0f 로 설정해야 할 경우
		vecPosition = XMVectorSetW(vecPosition, 1.0f);
		Set_State(STATE::POSITION, vecPosition);
	}

	// 2) 스케일 복원 (축 방향을 정규화한 뒤, 스칼라로 스케일)
	if (j.contains("Scale"))
	{
		_float3 vScale = {
			j["Scale"][0].get<_float>(),
			j["Scale"][1].get<_float>(),
			j["Scale"][2].get<_float>()
		};

		Scaling(vScale);
	}
}

void CTransform::LookAt(_fvector vAt)
{
	_float3		vScaled = Get_Scaled();

	_vector		vLook = vAt - Get_State(STATE::POSITION);
	_vector		vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
	_vector		vUp = XMVector3Cross(vLook, vRight);

	Set_State(STATE::RIGHT, XMVector3Normalize(vRight) * vScaled.x);
	Set_State(STATE::UP, XMVector3Normalize(vUp) * vScaled.y);
	Set_State(STATE::LOOK, XMVector3Normalize(vLook) * vScaled.z);
	m_bDirty = true;
}

HRESULT CTransform::Bind_ShaderResource(CShader* pShader, const _char* pConstantName)
{
	return pShader->Bind_Matrix(pConstantName, &m_WorldMatrix);
}

CTransform* CTransform::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTransform* pInstance = new CTransform(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CTransform");
		Safe_Release(pInstance);
	}

	return pInstance;
}


CComponent* CTransform::Clone(void* pArg)
{
	CTransform* pInstance = new CTransform(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CTransform");
		Safe_Release(pInstance);
	}

	return pInstance;

}

void CTransform::Free()
{
	__super::Free();

}
