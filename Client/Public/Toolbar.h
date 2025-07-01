#pragma once
#include "Pannel.h"
#include <JsonLoader.h>
BEGIN_NAMESPACE(Engine)
class CGameObject;
class CAnimation;
class CNavigation;
class CParticleSystem;
class CShader;
class CTexture;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class CToolbar : public CPannel
{
private:
	CToolbar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	~CToolbar() = default;
public:
	HRESULT Initialize();
	void Update(_float fTimeDelta);
	HRESULT Render();

	// 네비 매쉬용
	void CreatePoints(const _float3& worldPos);
	void DeletePoints(const _float3& worldPos);
private:
	void DrawToolbar();

	void RegisterDefaultPrototypes();
	void SetLevelEnumToString();
	void UpdatePrototypeList();
	// 프로토타입 리스트
	void Get_PrototypeList();
	// FBX 로더
	void FBXLoader();
	///애니메이션 
	void DrawAnimEventEditor();
	// 네비 매쉬
	void ShowCells();
	// 파티클
	void ParticleEditor();
	HRESULT DrawParticlePreview();
	// 파티클로 만든 이펙트 생성
	void ParticleEffectEditor();
	void CreateParticleEffect(const _wstring& particleName, PARTICLE_UV vUV, _uint iTextureIndex = 0, _uint iShaderPass = 0);
	// 컷씬 카메라용 프리뷰
	void EditCutSceneCamera();
	void RenderCutScene(_float fTimeDelta);

	// 프로토타입 생성
	CGameObject* ClonePrototype(const string& prototypeName, const wstring& instanceName,void* pArg = nullptr);
	_float CrossZ(const _float3& a, const _float3& b, const _float3& c) const
	{
		return (b.x - a.x) * (c.z - a.z) - (b.z - a.z) * (c.x - a.x);
	}
	_float Snap(_float v, _float gridSize)
	{
		// v 를 gridSize 단위로 반올림
		return round(v / gridSize) * gridSize;
	}
	XMFLOAT3 SnapXZ(const XMFLOAT3& pos, _float gridSize)
	{
		XMFLOAT3 out = pos;
		out.x = Snap(out.x, gridSize);
		out.z = Snap(out.z, gridSize);
		// y는 그대로
		return out;
	}

private:
	unordered_set<string> m_PrototypeSet;
	unordered_map<string, _uint> m_LevelStringMap;
	string           m_CurrentPrototype;
	_uint	         m_iCurrentSelectedLevel{ 0 }; // 현재 선택 레벨
	vector<_wstring> m_ShaderKeys; // 쉐이더 키들
	vector<_wstring> m_TextureKeys; // 텍스쳐 키들
	vector<_wstring> m_ModelKeys; // 모델 키들
	string			 m_ModelKey;
	_char			 m_NameBuf[128] = ""; // 이름 저장용 버퍼
	vector<map<const _wstring, class CBase*>> m_pPrototypes;

	// 파일 경로용
	_char    m_FilePathBuf[260];   // 저장/로딩할 파일 경로
	_char    m_NavFilePathBuf[260]; // 프로토타입 이름
	_char     m_ParticleFilePathBuf[260];
	_char     m_CutScenePathBuf[260];
	vector<string> m_ParticleFilePaths;
	vector<string> m_CutScenePropertyFilePaths;
	vector<_wstring> m_FbxFilePaths;

	// JSON 로더
	CJsonLoader*  m_JsonLoader = nullptr;    // JsonLoader 인스턴스



	// 파티클 시스템 관련
	_bool m_bIsPointInstance = false; // 포인트 인스턴스 모드 여부
	_int m_iShaderPass = 0; // 현재 셰이더 패스
	_int m_iTextureIndex = 0;
	_uint m_iMaxTextureCount = 1; // 최대 텍스처 개수
	CParticleSystem* m_pParticleSystem = nullptr; // 파티클 시스템
	CShader* m_pPreviewShader = nullptr;
	CTexture* m_pPreviewTexture = nullptr;
	ID3D11ShaderResourceView* m_pEffectPreviewSRV = nullptr; // 렌더 타겟 뷰
	class CParticleEffect* m_pParticleEffect = nullptr; // 파티클 이펙트


	// 네비 매쉬용
	CNavigation* m_pNavigation = nullptr;
	vector<_float3> m_NavMeshPoints; // 네비메쉬 포인트들
	_bool m_bIsNavMeshCreating = false; // 네비메쉬 생성 중인지 여부
	
	// 컷씬 카메라용
	_bool m_bIsCutSceneCameraActive = false; // 컷씬 카메라 활성화 여부
	class CCutSceneCamera* m_pCutSceneCamera = nullptr;
	class CFreeCamera* m_pFreeCamera = nullptr; // 기존 카메라
	ID3D11ShaderResourceView* m_pCutSceneCameraSRV = nullptr; // 컷씬 카메라용 렌더 타겟 뷰
	_int m_iSelectedCutSceneIndex = -1;
public:
	static CToolbar* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};
END_NAMESPACE

