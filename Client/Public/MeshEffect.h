#pragma once
#include "Effect.h"

BEGIN_NAMESPACE(Client)

class CMeshEffect : public CEffect
{
protected:
	CMeshEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMeshEffect(const CMeshEffect& Prototype);
	virtual ~CMeshEffect() = default;
public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
public:
	void SetMesh(class CMesh* pMesh);
	void SetModel(class CModel* pModel);
	void SetBone(class CBone* pBone);

protected:
	HRESULT Ready_Components();

protected:
	CModel* m_pModelCom = { nullptr };
	_float2 m_fUVOffset = { 0.f, 0.f };
	_bool   m_bUseOffset{ false };

public:
	static CMeshEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE
