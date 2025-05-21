#pragma once

#include "Base.h"
#include "Serializable.h"	

BEGIN_NAMESPACE(Engine)

class ENGINE_DLL CComponent abstract : public CBase, public ISerializable
{
protected:
	CComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CComponent(const CComponent& Prototype);
	virtual ~CComponent() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);	

public:
	_bool IsActive() const { return m_bIsActive; }
	void SetActive(_bool bActive) {	m_bIsActive = bActive;}
	void SetOwner(class CGameObject* pOwner);

	void SetPrototypeLevel(_uint iPrototypeLevel) { m_iPrototypeLevel = iPrototypeLevel; }
	void SetCreateLevel(_uint iCreateLevel) { m_iCreateLevel = iCreateLevel; }

	void SetProtoTypeTag(const _wstring& strPrototypeTag) { m_strPrototypeTag = strPrototypeTag; }
	const _wstring& GetProtoTypeTag() const { return m_strPrototypeTag; }

	virtual json Serialize() override;
	virtual void Deserialize(const json& j) override;

protected:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	class CGameInstance*        m_pGameInstance = { nullptr };

	_bool						m_isCloned = { false };
	_bool m_bIsActive{ true };
	class CGameObject* m_pOwner{ nullptr };
	_wstring m_strPrototypeTag{};
	_uint m_iPrototypeLevel{ 0 };
	_uint m_iCreateLevel{ 0 };
public:
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END_NAMESPACE