#pragma once

/* 엔진과 클라이언트 간의 링크의 역활을 수행한다. */
/* 엔진 내에 존재하는 유일한 싱글톤 클래스이다. */
/* 엔진 개발자가 클라개밫자에게 보여주고싶은 함수를 ... */
#include "Prototype_Manager.h"


BEGIN_NAMESPACE(Engine)


class ENGINE_DLL CGameInstance final : public CBase
{
	DECLARE_SINGLETON(CGameInstance)

private:
	CGameInstance();
	virtual ~CGameInstance() = default;

public:
	HRESULT Initialize_Engine(const ENGINE_DESC& EngineDesc, _Out_ ID3D11Device** ppDeviceOut, ID3D11DeviceContext** ppContextOut);
	void Fixed_Update(_float fTimeDelta);
	void Update_Engine(_float fTimeDelta);
	void Clear(_uint iLevelIndex);
	void ClearObejcts(_uint iLevelIndex);
	HRESULT Begin_Draw();
	HRESULT Draw();
	HRESULT End_Draw();

	_float Compute_Random_Normal();
	_float Compute_Random(_float fMin, _float fMax);

	void SetActivePicking(_bool bActive) { m_bActivePicking = bActive; }
	void SetChangeLevel(_bool bChange) { m_bChangedLevel = bChange; }

	_bool IsActivePicking() const { return m_bActivePicking; }

	ID3D11ShaderResourceView* GetSceneViewSRV() const;

	void SetHitStop(_bool bHitStop,_float fHitStopTime) { 
		m_bHitStop = bHitStop;
		m_fHitStopTime = fHitStopTime;
	}

#pragma region LEVEL_MANAGER
public:
	HRESULT Change_Level(_uint iLevelIndex, class CLevel* pNewLevel);
	_int Get_CurrentLevelIndex();
#pragma endregion

#pragma region PROTOTYPE_MANAGER
	HRESULT Add_Prototype(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, class CBase* pPrototype);
	CBase* Clone_Prototype(PROTOTYPE ePrototypeType, _uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, void* pArg = nullptr);
	map<const _wstring, class CBase*>* Get_Prototypes(_uint iLevelIndex);
#pragma endregion

#pragma region OBJECT_MANAGER
	class CGameObject* Add_GameObject(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLevelIndex, const _wstring& strLayerTag, void* pArg = nullptr);
	CComponent* Get_Component(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strComponentTag, _uint iIndex = 0);
	HRESULT Delete_GameObject(_uint iLevelIndex, CGameObject* pGameObject);
	HRESULT Delete_GameObjectByName(_uint iLevelIndex, const _wstring& strName);
	class CGameObject* Find_GameObjectByName(_uint iLevelIndex, const _wstring& strName);
#pragma endregion

#pragma region RENDERER
	HRESULT Add_RenderGroup(RENDERGROUP eRenderGroup, class CGameObject* pRenderObject);
#pragma endregion

#pragma region TIMER_MANAGER
	_float Get_TimeDelta(const _wstring& strTimerTag);
	HRESULT Add_Timer(const _wstring& strTimerTag);
	void Update_Timer(const _wstring& strTimerTag);
#pragma endregion

#pragma region PICKING
	void Transform_Picking_ToLocalSpace(const _matrix& WorldMatrixInverse);
	_bool Picking_InWorld(_float3& vPickedPos, const _float3& vPointA, const _float3& vPointB, const _float3& vPointC);
	_bool Picking_InLocal(_float3& vPickedPos, const _float3& vPointA, const _float3& vPointB, const _float3& vPointC);
#pragma endregion


#pragma region FRUSTUM
	_bool IsPointInFrustum(const _float3& point);
	_bool IsSphereInFrustum(const _float3& point, _float fRadius = 0.f);
	_bool IsAABBInFrustum(const _float3& point, const _float3& scale);
#pragma endregion

#pragma region INPUT_DEVICE
	_bool IsKeyDown(_ushort vkey) const;
	_bool IsKeyPressed(_ushort vkey) const;
	_bool IsMouseDown(_ulonglong btn) const;
	_bool IsMousePressed(_ulonglong btn) const;
	POINT GetMousePos() const;
	LONG GetMouseWheel() const;
	POINT GetMouseDelta() const;
	void ProcessRawInput(LPARAM lParam);
	void Update_Input();
#pragma endregion

#pragma region TRASNFROM_PIPELINE
	void Set_Transform(TRANSFORM eState, _fmatrix TransformMatrix);
	const _float4x4* Get_Transform_Float4x4(TRANSFORM eState) const;
	const _float4x4* Get_Transform_Float4x4_Inv(TRANSFORM eState) const;

	const _matrix Get_Transform_Matrix(TRANSFORM eState) const;
	const _matrix Get_Transform_Matrix_Inv(TRANSFORM eState) const;
	const _float4* Get_CamPosition() const;
	 _float  Get_CameraFar() const;
	_vector UnprojectToGround(_float mx, _float my, const D3D11_VIEWPORT& vp);
#pragma endregion

#pragma region UI
	void AddCanvasUI(class CUICanvas* pCanvas);
	void RemoveCanvasUI(const _wstring& canvasName);
	void RemoveUI(const _wstring& canvasName, const _wstring& uiName);
	class CUIObject* Get_UI(const _wstring& canvasName, const _wstring& uiName);
	CGameObject* CreateUI(void* pDesc, UI_TYPE eUIType);
	CGameObject* GetCanvasUI(const _wstring& canvasName);
	void ClearUI();
#pragma endregion

#pragma region RESOURCE_MANAGER
	class CShader* GetShader(const _wstring& key, _bool bIsStatic);
	class CTexture* GetTexture(const _wstring& key, _bool bIsStatic);
	class CVIBuffer* GetBuffer(const _wstring& key, _bool bIsStatic);
	class CModel* GetModel(const _wstring& key, _bool bIsStatic = true);

	class CShader* LoadShader(const _wstring& key, const _wstring& vsPath, const D3D11_INPUT_ELEMENT_DESC* pElements, _uint iNumElements, _bool bIsStatic);
	class CTexture* LoadTexture(const _wstring& key, const _wstring& filePath, _bool bIsStatic, _uint iNumTextrues = 1);
	class CVIBuffer* LoadBuffer(const _wstring& key, BUFFER_TYPE eType, _bool bIsStatic);
	class CModel* LoadModel(const _wstring& key, const _wstring& filePath, MODEL eType, _matrix preMatrix, _bool bIsStatic = true);

	const vector<_wstring>& GetShaderKeys(_bool bIsStatic = true) const;
	const vector<_wstring>& GetTextureKeys(_bool bIsStatic = true) const;
	const vector<_wstring>& GetModelKeys(_bool bIsStatic = true) const;
#pragma endregion

#pragma region COLLIDER
	void Register_Collider(class CCollider* pCollider);
	void Unregister_Collider(class CCollider* pCollider);
	void ClearColliders();
#pragma endregion

#pragma region LIGHT_MANAGER
	const LIGHT_DESC* Get_Light(_uint iIndex);
	HRESULT Add_Light(const LIGHT_DESC& LightDesc);
	HRESULT Render_Lights(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
	void ClearLights();
#pragma endregion

#pragma region FONT_MANAGER
	HRESULT Add_Font(const _wstring& strFontTag, const _tchar* pFontFilePath);
	void Draw_Font(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition, _fvector vColor = XMVectorSet(1.f, 1.f, 1.f, 1.f), _float fRotation = 0.f, const _float2& vOrigin = _float2(0.f, 0.f), _float fScale = 1.f);
#pragma endregion

#pragma region TARGET_MANAGER
	HRESULT Add_RenderTarget(const _wstring& strTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT ePixelFormat, const _float4& vClearColor);
	HRESULT Add_MRT(const _wstring& strMRTTag, const _wstring& strTargetTag);
	ID3D11ShaderResourceView* Get_RenderTargetSRV(const _wstring& strTargetTag);
	HRESULT Begin_MRT(const _wstring& strMRTTag, ID3D11DepthStencilView* pDSV = nullptr, _bool isTargetClear = true, _bool isDepthClear = false);
	HRESULT End_MRT();
	HRESULT Bind_RT_ShaderResource(const _wstring& strTargetTag, class CShader* pShader, const _char* pContantName);

#ifdef _DEBUG
	HRESULT Ready_RT_Debug(const _wstring& strTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY);
	HRESULT Render_MRT_Debug(const _wstring& strMRTTag, class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
#endif
#pragma endregion

#pragma region SOUND
	CSoundMag* GetSoundMag();
#pragma endregion


private:
	class CGraphic_Device*		m_pGraphic_Device = { nullptr };
	class CLevel_Manager*		m_pLevel_Manager = { nullptr };
	class CPrototype_Manager*	m_pPrototype_Manager = { nullptr };
	class CObject_Manager*		m_pObject_Manager = { nullptr };
	class CRenderer*			m_pRenderer = { nullptr };
	class CTimer_Manager*		m_pTimer_Manager = { nullptr };
    class CPicking*				m_pPicking = { nullptr };
	class CFrustumCull* m_pFrustumCull = { nullptr };
	class CInput_Device* m_pInput_Device = { nullptr };
	class CTransformPipeline* m_pTransformPipeline = { nullptr };
	class CUIManager* m_pUIManager = { nullptr };
	class CResourceMag* m_pResourceMag = { nullptr };
	class CCollisionMag* m_pCollisionMag = { nullptr };
	class CLight_Manager* m_pLight_Manager = { nullptr };
	class CFontMag* m_pFont_Manager = { nullptr };
	class CTarget_Manager* m_pTarget_Manager = { nullptr };


	_bool m_bActivePicking = { false }; // 피킹 활성화 여부
	_bool m_bChangedLevel = { false }; // 레벨 변경 여부
	_bool m_bHitStop = { false };

	_float m_fHitStopTime = { 0.f }; // 히트스탑 시간
	_float m_fHitStopElapsed = { 0.f }; // 히트스탑 경과 시간

public:
	void Release_Engine();
	virtual void Free() override;
};

END_NAMESPACE

