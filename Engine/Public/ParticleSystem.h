#pragma once
#include "VIBuffer_Instance.h"

BEGIN_NAMESPACE(Engine)
class ENGINE_DLL CParticleSystem : public CVIBuffer_Instance
{
public:
	typedef struct tagParticleSystemDesc : public CVIBuffer_Instance::INSTANCE_DESC
	{
		PARTICLE_TYPE eParticleType = PARTICLE_TYPE::RECT; // 입자 타입 (POINT, RECT)
		_bool	isLoop = true;
		_bool   bPlayAwake = true;				// 시작 시 자동 재생 여부
		_float2 vLifeTime = _float2(0.f, 5.f);					// 생명주기 고정 5초
		_float2 vSpeed = _float2(0.f, 5.f);						// 속도 고정 5
		_float3 vStartColor = _float3(1.f, 1.f, 1.f);			// 시작 색상
		_float3 vEndColor = _float3(1.f, 1.f, 1.f);				// 끝날 때  색상
		_float3 vVelocity = _float3(0.f, 0.f, 0.f);				// 방향성 없음
		_float fGravity = 0.f;									// 중력 없음
		_float fSpreadAngle = 25.f;								// 확산 각도 (fAngle과 동일하게 기본값 설정)
		_float fAlphaVariation = 1.f;							// 알파 변화 없음
	} PARTICLE_DESC;
private:
	CParticleSystem(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CParticleSystem(const CParticleSystem& Prototype);
	virtual ~CParticleSystem() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	HRESULT Initialize_Prototype(const PARTICLE_DESC& desc);
	virtual HRESULT Initialize(void* pArg) override;

	void SetIsLoop(_bool bIsLoop) { m_bIsLoop = bIsLoop; }
	_bool IsLoop() const { return m_bIsLoop; }
	HRESULT UpdateVertexInstances(_float fTimeDelta);
	virtual json Serialize() override;

	void SetParticleDesc(const PARTICLE_DESC& desc)
	{
		m_ParticleDesc = desc;
		SetVertexInfo(m_ParticleDesc);
	}
	void SetVelocity(const _float3& vVelocity)
	{
		for (auto& velocity : m_vecVelocities)
		{
			velocity = vVelocity;
		}
	}
	void SetGravity(_float fGravity)
	{
		m_ParticleDesc.fGravity = fGravity;
	}
	void SetPlayAwake(_bool bPlayAwake)
	{
		m_ParticleDesc.bPlayAwake = bPlayAwake;
	}

	PARTICLE_TYPE GetParticleType() const
	{
		return m_ParticleDesc.eParticleType;
	}

	void ResetParticle();
	void PlayParticle() { m_bStarted = true; }
	void StopParticle() { m_bStarted = false; }

	PARTICLE_DESC GetParticleDesc() const
	{
		return m_ParticleDesc;
	}
	_bool IsPlaying() const { return m_bStarted; }
private:
	void    SetVertexInfo(const PARTICLE_DESC& desc);
	HRESULT CreateVertexBuffer();
	HRESULT CreateIndexBuffer();
	HRESULT CreateVertexInstances(const PARTICLE_DESC& desc);
	virtual void OnEnable() override;



private:
	void* m_pVertexInstances = { nullptr }; // 입자 인스턴스 버퍼
	vector<_float>				m_vecSpeeds;
	vector<_float3>				m_vecVelocities;
	vector<_float3>			    m_vecUseVelocities; // 사용되는 속도 벡터
	_bool						m_bIsLoop = { true };
	_bool 						m_bStarted = true;
	PARTICLE_DESC				m_ParticleDesc{};
public:
	static CParticleSystem* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const PARTICLE_DESC& desc);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

