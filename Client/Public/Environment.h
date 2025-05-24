#pragma once
#include "GameObject.h"
#include "Client_Defines.h"


BEGIN_NAMESPACE(Engine)
class CShader;
class CModel;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class CEnvironment : public CGameObject
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
private:
	HRESULT Ready_Components();
private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	_wstring m_strModelTag = L"";
public:
	static CEnvironment* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

