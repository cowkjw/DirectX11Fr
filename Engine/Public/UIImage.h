#pragma once
#include "UIObject.h"   

BEGIN_NAMESPACE(Engine)
class CShader;
class CTexture;

class ENGINE_DLL CUIImage : public CUIObject
{
protected:
	CUIImage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUIImage(const CUIImage& Prototype);
	virtual ~CUIImage()  = default;
public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
public:
	void Set_TextureIndex(_uint iIndex) {
		if (m_iNumTextures > iIndex)
			m_iTextureIndex = iIndex; }

	virtual json Serialize() override;
	virtual void Deserialize(const json& j) override;
	void Set_Color(const _float4& vColor) { m_vColor = vColor; }
	void EnableUVAnim(_int cols, _int rows, _float frameSec);
	CShader* Get_Shader() const { return m_pShaderCom; }
	void SetUVOffset(_float uOffset, _float vOffset) {
		m_UVOffset[0] = uOffset; m_UVOffset[1] = vOffset;
	}
	void SetUVSpeed(_float uSpeed, _float vSpeed) {
		m_UVSpeed[0] = uSpeed; m_UVSpeed[1] = vSpeed;
	}
	void SetUVScale(_float uScale, _float vScale) {
		m_UVScale[0] = uScale; m_UVScale[1] = vScale;
	}
	void SetIsBarChild(_bool bIsBar) { m_bParentIsBar = bIsBar; }
	void SetShaderPass(_uint iPass) { m_iShaderPass = iPass; }
	void SetRatio(_float ratio) { m_fRatio = ratio; }
protected:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	_wstring m_strTextureKey{};
	_wstring m_strShaderKey{};
	_uint m_iNumTextures{ 0 };
	_uint m_iTextureIndex{ 0 };
	_bool m_bIsUVAnim{ false };
	_bool m_bParentIsBar{ false }; // 부모가 바 형태일 때 UV도 사용함
	array<_float, 2> m_UVOffset = { 0,0 };
	array<_float, 2> m_UVSpeed = { 0,0 };
	array<_float, 2> m_UVScale = { 1,1 };
	_float4 m_vColor = { 1.f, 1.f, 1.f, 1.f }; // RGBA
	_float m_fRows = 1.f; // UV 애니메이션 행
	_float m_fCols = 1.f; // UV 애니메이션 열
	_float m_fRatio = 0.f;
	_uint m_iShaderPass = 0; // 셰이더 패스 인덱스

protected:
	HRESULT Ready_Components();
public:
	static CUIImage* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

