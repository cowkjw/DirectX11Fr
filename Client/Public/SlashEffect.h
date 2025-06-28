#pragma once
#include "MeshEffect.h"

BEGIN_NAMESPACE(Client)
class CSlashEffect : public CMeshEffect
{
private:
	CSlashEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSlashEffect(const CSlashEffect& Prototype);
	virtual ~CSlashEffect() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void SetColor(const _float4& vColor)
	{
		m_vColor = vColor;
	}

private:
	HRESULT Ready_Components();
	virtual HRESULT Bind_Shader() override;
	virtual void OnDisable() override;
	virtual void OnEnable() override;

private:
	_float4 m_vColor = { 1.f, 1.f, 1.f, 1.f };

public:
	static CSlashEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

