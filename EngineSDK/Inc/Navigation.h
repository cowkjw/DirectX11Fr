#pragma once

#include "Component.h"

/* 내 게임 내에 길을 구성해주는 삼각형들을 모아서 보관한다. */

BEGIN_NAMESPACE(Engine)

class ENGINE_DLL CNavigation final : public CComponent
{
public:
	typedef struct tagNavigationDesc
	{
		_int			iIndex = { -1 };
	}NAVIGATION_DESC;
private:
	CNavigation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CNavigation(const CNavigation& Prototype);
	virtual ~CNavigation() = default;

public:
	HRESULT Initialize_Prototype(const _tchar* pNavigationDataFile);
	HRESULT Initialize(void* pArg);

	void Update(_fmatrix WorldMatrix);
	_bool isMove(_fvector vWorldPos);
	_vector SetUp_Height(_fvector vWorldPos);

	HRESULT SaveCells(const _tchar* pFilePath);
	HRESULT LoadCells(const _tchar* pFilePath);
	void AddCell(class CCell* pCell);
	void ClearCells();
	_int GetCellCount() const { return static_cast<_int>(m_Cells.size()); }
	vector<class CCell*>& GetCells() { return m_Cells; }

	void SetIndex(_int iIndex) { m_iIndex = iIndex; }
	_int FindIndexCell(const _vector& vWorldPos);

	static _float4x4 GetWorldMatrix() { return m_WorldMatrix; }
	void DeleteCell(const _vector& vWorldPos);
	_vector GetHitCellNormal(_int iIndex);
	_int GetIndex() const { return m_iIndex; }
	class CCell* GetCellByIndex(_int iIndex) const
	{
		if (iIndex < 0 || iIndex >= static_cast<_int>(m_Cells.size()))
			return nullptr;
		return m_Cells[iIndex];
	}
#ifdef _DEBUG
public:
	HRESULT Render();

#endif

private:
	_float CrossZ(const _float3& a, const _float3& b, const _float3& c) const
	{
		return (b.x - a.x) * (c.z - a.z) - (b.z - a.z) * (c.x - a.x);
	}
	_bool SnapVertex(_float3& p, _float eps = 1e-3f);

private:	
	vector<class CCell*>				m_Cells;
	_int								m_iIndex = { -1 };

	static _float4x4					m_WorldMatrix;

	vector<_float3> m_Vertices;

#ifdef _DEBUG
	class CShader* m_pShader = { nullptr };
#endif

private:
	HRESULT SetUp_Neighbors();

public:
	static CNavigation* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pNavigationDataFile);
	virtual CComponent* Clone(void* pArg);
	virtual void Free() override;
};	

END_NAMESPACE