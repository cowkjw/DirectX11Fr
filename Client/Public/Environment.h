#pragma once
#include "GameObject.h"
#include "Client_Defines.h"


BEGIN_NAMESPACE(Engine)
class CShader;
class CModel;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class CEnvironment : public CGameObject, public ICollisionListener
{
public:
	typedef struct tagEnvironmentDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_wstring strModelTag;
	}ENVIRONMENT_DESC;
public:
	CEnvironment(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnvironment(const CEnvironment& Prototype);
	virtual ~CEnvironment() = default;
public:

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
public:
	virtual json Serialize() override;
	virtual void Deserialize(const json& j) override;

	void SetInitPos() { m_InitPos = m_pTransformCom->Get_State(STATE::POSITION); }
private:
	HRESULT Ready_Components();
	virtual HRESULT Bind_Shaders();
private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	_wstring m_strModelTag = L"";
	_uint m_iShaderPass{ 0 };
	_vector m_InitPos{};
	_float m_fLimitZ{ -800.f }; // Z축 제한 위치
	_float m_fTilingSpeed{ 300.f }; // 타일링 속도 (무한 배경용)
public:
	static CEnvironment* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

	virtual void OnCollisionEnter(CCollider* other) override;
	virtual void OnCollisionEnter(CCollider* other, const _float3& hitPos) override;
	virtual void OnCollisionStay(CCollider* other, float fTimeDelta) override;
	virtual void OnCollisionExit(CCollider* other) override;
};
END_NAMESPACE

