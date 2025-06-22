#pragma once
#include "Pannel.h"
#include <JsonLoader.h>
BEGIN_NAMESPACE(Engine)
class CGameObject;
class CAnimation;
class CNavigation;
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
	void CreatePoints(const _float3& worldPos);
	void DeletePoints(const _float3& worldPos);
private:
	void DrawToolbar();

	void RegisterDefaultPrototypes();
	void SetLevelEnumToString();
	void UpdatePrototypeList();
	void Get_PrototypeList();
	void FBXLodaer();
	void DrawAnimEventEditor();
	void SpawnMouse(void* pArg = nullptr);
	void ShowCells();

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
	//unordered_map<string, CGameObject*> m_PrototypeMap;
	unordered_set<string> m_PrototypeSet;
	unordered_map<string, _uint> m_LevelStringMap;
	string           m_CurrentPrototype;
	_uint	         m_iCurrentSelectedLevel{ 0 };
	vector<_wstring> m_ShaderKeys;
	vector<_wstring> m_TextureKeys;
	vector<_wstring> m_ModelKeys;
	vector<map<const _wstring, class CBase*>> m_pPrototypes;
	_char         m_NameBuf[128] = "";

	_char    m_FilePathBuf[260];   // 저장/로딩할 파일 경로
	_char    m_NavFilePathBuf[260]; // 프로토타입 이름
	CJsonLoader*  m_JsonLoader = nullptr;    // JsonLoader 인스턴스

	mutex m_FbxLoadMutex;
	string m_ModelKey;

	vector<_wstring> m_FbxFilePaths;


	// 네비 매쉬용
	CNavigation* m_pNavigation = nullptr;
	vector<_float3> m_NavMeshPoints; // 네비메쉬 포인트들
	_bool m_bIsNavMeshCreating = false; // 네비메쉬 생성 중인지 여부
public:
	static CToolbar* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};
END_NAMESPACE

