#pragma once
#include "GameObject.h"
#include "Client_Defines.h"

BEGIN_NAMESPACE(Engine)
class CShader;
class CModel;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class CSky : public CGameObject
{
public:
	CSky(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSky(const CSky& Prototype);
	virtual ~CSky() = default;
public:

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
private:
	HRESULT Ready_Components();
private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };

public:
	static CSky* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

