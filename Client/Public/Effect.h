#pragma once
#include "GameObject.h"
#include "Client_Defines.h"

BEGIN_NAMESPACE(Engine)
class CParticleSystem;
class CShader;
class CTexture;
class CModel;
class CBone;
END_NAMESPACE;

BEGIN_NAMESPACE(Client)
class CEffect : public CGameObject
{
public:
	enum EFFECT_TEXTURE 
	{
		TEX_DIFFUSE = 0,    // 기본 색상
		TEX_MASK,           // 마스킹 (투명도나 영역 제어용)
		TEX_NOISE,          // 왜곡 효과용 노이즈
		TEX_DISTORT,        // 화면 왜곡 전용 (Screen UV로)
		TEX_ALPHA,          // 별도 알파 조절 텍스처
		TEX_EMISSION,       // 발광용 텍스처 (자체 발광 영역)
		TEX_DISSOLVE,        // 디졸브	효과용 텍스처
		TEX_MAX
	};
protected:
	CEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEffect(const CEffect& Prototype);
	virtual ~CEffect() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void Set_BoneSocket(CBone* pBoneSocket) {
		m_pBoneSocket = pBoneSocket;
	}

	virtual void OnEnable() override;
	virtual void OnDisable() override;

	void SetTextureIndex(_uint iTextureIndex) {
		m_iTextureIndex = iTextureIndex;
	}

	void SetShaderPass(_uint iShaderPass) {
		m_iShaderPass = iShaderPass;
	}
protected:
	virtual HRESULT Bind_Shader();
protected:
	_float4x4				m_CombinedWorldMatrix{};
	array<CTexture*, TEX_MAX> m_Textures{};
	CShader* m_pShaderCom = { nullptr };
	_uint m_iShaderPass = { 0 };
	_uint m_iTextureIndex = { 0 };
	_float m_fDuration = 0.f;
	_float m_fElapsed = 0.f;
	_bool m_bLoop = false;
	CBone* m_pBoneSocket = { nullptr };


public:
	virtual void Free();
};
END_NAMESPACE

