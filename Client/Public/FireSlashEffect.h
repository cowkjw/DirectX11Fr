#pragma once
#include "MeshEffect.h"

BEGIN_NAMESPACE(Client)
class CFireSlashEffect : public CMeshEffect
{
private:
	CFireSlashEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFireSlashEffect(const CFireSlashEffect& Prototype);
	virtual ~CFireSlashEffect() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Components();
	virtual HRESULT Bind_Shader() override;

public:
	static CFireSlashEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

