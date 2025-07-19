#pragma once
#include "GameObject.h"
#include "Client_Defines.h"

BEGIN_NAMESPACE(Engine)
class CVIBuffer_Rect;
class CShader;
class CTexture;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class CTrainPointLight : public CGameObject
{
private:
	CTrainPointLight(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTrainPointLight(const CTrainPointLight& Prototype);
	virtual ~CTrainPointLight() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Components();
	HRESULT Bind_Shader();
	

private:
	CShader* m_pShaderCom = nullptr;
	CTexture* m_pTextureCom = nullptr;
	CVIBuffer_Rect* m_pVIBufferCom = nullptr;
	_float4 m_vColor = { 1.f, 1.f, 1.f, 1.f }; // »ö»ó

public:
	virtual void Free() override;
	static CTrainPointLight* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
};
END_NAMESPACE

