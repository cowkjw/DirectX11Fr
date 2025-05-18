#pragma once

#include "Transform.h"
#include "Serializable.h"
BEGIN_NAMESPACE(Engine)

class ENGINE_DLL CGameObject abstract : public CBase, public ISerializable
{ public:
	typedef struct tagGameObjectDesc : public CTransform::TRANSFORM_DESC
	{
		_wstring strName;
		_wstring strTag;

	}GAMEOBJECT_DESC;

protected:
	CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject(const CGameObject& Prototype);
	virtual ~CGameObject() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	virtual void RenderInspector() {};

	virtual void OnEnable() {};
	virtual void OnDisable() {};

	virtual json Serialize() override;
	virtual void Deserialize(const json& j) override;

public:
	_bool IsActive() const { return m_bIsActive; }
	void SetActive(_bool bActive) {
		if (m_bIsActive == bActive) 
			return;
		m_bIsActive = bActive;
		if (bActive) OnEnable();
		else         OnDisable();
	}

	CComponent* Get_Component(const _wstring& strComponentTag);
	const map<const _wstring, class CComponent*>& GetComponents() const { return m_Components; }

	const _wstring& Get_Name() const { return m_strName; }
	void Set_Name(const _wstring& name) { m_strName = name; }

	void SetParent(CGameObject* pParent) { m_pParent = pParent; }
	CGameObject* GetParent() { return m_pParent; }
	auto GetChildren() const { return m_vecChildren; }
	void AddChild(CGameObject* pGameObject);
	void RemoveChild(CGameObject* pGameObject)
	{
		auto newEnd = remove_if(m_vecChildren.begin(), m_vecChildren.end(),
			[&](CGameObject* child) { return child == pGameObject; });
		if (newEnd != m_vecChildren.end())
		{
			m_vecChildren.erase(newEnd, m_vecChildren.end());
			pGameObject->SetParent(nullptr);
		}
	}

	_uint GetID() const { return m_uID; }

	CTransform* GetTransform() const { return m_pTransformCom; }
	HRESULT Add_Component(const _wstring& strComponentTag, CComponent* pComponent, CComponent** ppOut);
	HRESULT Add_Component(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strComponentTag, CComponent** ppOut, void* pArg = nullptr);

	void SetPrototypeLevel(_uint iPrototypeLevel) { m_iPrototypeLevel = iPrototypeLevel; }
	void SetCreateLevel(_uint iCreateLevel) { m_iCreateLevel = iCreateLevel; }
	_uint GetPrototypeLevel() const { return m_iPrototypeLevel; }
	_uint GetCreateLevel() const { return m_iCreateLevel; }

	void SetProtoTypeTag(const _wstring& strPrototypeTag) { m_strPrototypeTag = strPrototypeTag; }
	const _wstring& GetProtoTypeTag() const { return m_strPrototypeTag; }

protected:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	class CGameInstance*		m_pGameInstance = { nullptr };

	CTransform*			m_pTransformCom = { nullptr };
	map<const _wstring, class CComponent*>		m_Components;
	_bool m_bIsActive{ true };
	_bool m_bIsPooled{ false };
	_bool m_bIsCloned = { false };
	_wstring m_strName{};
	_wstring m_strTag{};
	_wstring m_strPrototypeTag{};
	_uint m_iPrototypeLevel{ 0 };
	_uint m_iCreateLevel{ 0 };

	CGameObject* m_pParent{ nullptr };
	vector<CGameObject*> m_vecChildren;

	static _uint s_uNextID;
	_uint        m_uID = 0;

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};
END_NAMESPACE