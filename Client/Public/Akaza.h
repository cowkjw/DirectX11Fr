#pragma once
#include "BaseCharacter.h"

BEGIN_NAMESPACE(Client)
class CAkaza :public CBaseCharacter
{
private:
	CAkaza(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAkaza(const CAkaza& Prototype);
	virtual ~CAkaza() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;


private:
	virtual HRESULT Ready_Components() override;
	virtual void Ready_Animation() override;


public:
	static CAkaza* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

