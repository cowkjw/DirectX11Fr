#pragma once
#include "GameObject.h"
#include "Client_Defines.h"

BEGIN_NAMESPACE(Engine)
class CTexture;
class CShader;
class CVIBuffer_Rect;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class CDecal : public CGameObject
{
public:
	typedef struct tagDecalDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_float3 vPosition;
		_float3 vScale;
		_float fLifeTime;
		_uint iShaderPass;
	} DECAL_DESC;
protected:
	CDecal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDecal(const CDecal& Prototype);
	virtual ~CDecal() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void SetShaderPass(_uint iShaderPass) { m_iShaderPass = iShaderPass; }

protected:
	virtual HRESULT Ready_Components();
	virtual HRESULT Bind_Shaders();

protected:
	CTexture* m_pTextureCom{ nullptr };
	CShader* m_pShaderCom{ nullptr };
	CVIBuffer_Rect* m_pVIBufferCom{ nullptr };
	_uint m_iShaderPass = 0;
	_float m_fLifeTime = 0.f;
	_float m_fElapsedTime = 0.f;
public:
	static CDecal* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;


};
END_NAMESPACE

