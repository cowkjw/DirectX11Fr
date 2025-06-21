#pragma once
#include "Decal.h"

BEGIN_NAMESPACE(Client)	
class CWarningZoneDecal : public CDecal
{
private:
	CWarningZoneDecal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWarningZoneDecal(const CWarningZoneDecal& Prototype);
	virtual ~CWarningZoneDecal() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
private:
	virtual HRESULT Ready_Components() override;
	virtual HRESULT Bind_Shaders() override;
public:
	static CWarningZoneDecal* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

