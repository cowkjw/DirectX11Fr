#include "Collider.h"
#include "GameInstance.h"
#include "GameObject.h"


CCollider::CCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent(pDevice, pContext)
	, m_offset(0.f, 0.f, 0.f)
	, m_bIsTrigger(false)
{

}

CCollider::CCollider(const CCollider& Prototype)
	: CComponent(Prototype)
	, m_offset(Prototype.m_offset)
	, m_bIsTrigger(Prototype.m_bIsTrigger)
	, m_bIsCollision(Prototype.m_bIsCollision)
	, m_bIsDebugDraw(Prototype.m_bIsDebugDraw)
	, m_eColliderType(Prototype.m_eColliderType)

{
}
HRESULT CCollider::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CCollider::Initialize(void* pArg)
{
	m_pGameInstance->Register_Collider(this);

	m_pEffect = new BasicEffect(m_pDevice);
	m_pEffect->SetVertexColorEnabled(true);
	m_pBatch = new PrimitiveBatch<VertexPositionColor>(m_pContext);
	const void* vsBlob = nullptr;
	size_t      vsBlobSize = 0;
	m_pEffect->GetVertexShaderBytecode(&vsBlob, &vsBlobSize);

	// 2) InputLayout 생성
	HRESULT hr = m_pDevice->CreateInputLayout(
		VertexPositionColor::InputElements,    // D3D11_INPUT_ELEMENT_DESC 배열
		VertexPositionColor::InputElementCount,// 배열 크기
		vsBlob,                                // VS bytecode 포인터
		vsBlobSize,                            // bytecode 크기
		&m_pInputLayout                       // 결과 InputLayout
	);

    return S_OK;
}

void CCollider::Update()
{
}

void CCollider::DebugDraw()
{
	XMMATRIX view = m_pGameInstance->Get_Transform_Matrix(TRANSFORM::VIEW);
	XMMATRIX proj = m_pGameInstance->Get_Transform_Matrix(TRANSFORM::PROJECTION);
	XMMATRIX world = XMMatrixIdentity();

	// 2) Effect에 셋팅
	m_pEffect->SetWorld(world);
	m_pEffect->SetView(view);
	m_pEffect->SetProjection(proj);


	m_pContext->IASetInputLayout(m_pInputLayout);

}

void CCollider::RenderInspector(IInspector& inspector)
{
	if (inspector.TreeNode("Collider Properties"))
	{
		_bool changed = false;
		_float off[3] = { m_offset.x, m_offset.y, m_offset.z };
		_int priority = m_iPriority;
		if (inspector.DragFloat3("Offset", off, 0.1f))
		{
			m_offset = { off[0], off[1], off[2] };
			changed = true;
		}
		if (inspector.Checkbox("Is Trigger", &m_bIsTrigger))
		{
			changed = true;
		}
		if (inspector.InputInt("Priority", &priority))
		{
			m_iPriority = priority;
			changed = true;
		}
		if (changed)
		{
			// 트리거 플래그 적용
			SetTrigger(m_bIsTrigger);
		}
		inspector.TreePop();
	}
}

json CCollider::Serialize()
{
	json j = CComponent::Serialize();
	j["Offset"] = { m_offset.x, m_offset.y, m_offset.z };
	j["IsTrigger"] = m_bIsTrigger;
	j["Priority"] = m_iPriority;
	return j;
}

void CCollider::Deserialize(const json& j)
{
	CComponent::Deserialize(j);
	if (j.contains("Offset"))
	{
		auto offset = j["Offset"];
		m_offset = { offset[0], offset[1], offset[2] };
	}
	if (j.contains("IsTrigger"))
	{
		m_bIsTrigger = j["IsTrigger"];
	}
	if (j.contains("Priority"))
	{
		m_iPriority = j["Priority"];
	}
}

void CCollider::Free()
{
    __super::Free();

	Safe_Delete(m_pBatch);
	Safe_Delete(m_pEffect);
	Safe_Release(m_pInputLayout);
}
