#pragma once
#include "VIBuffer.h"

BEGIN_NAMESPACE(Engine)
class ENGINE_DLL CVIBuffer_Terrain :public CVIBuffer
{
public:
	typedef struct tagTerrainDesc
	{
		_float fX;
		_float fZ;
		const _tchar* pHeightMapFilePath{ nullptr };
	}TERRAIN_DESC;
private:
	CVIBuffer_Terrain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Terrain(const CVIBuffer_Terrain& Prototype);
	virtual ~CVIBuffer_Terrain() = default;

public:
	virtual HRESULT Initialize_Prototype(const _tchar* pHeightMapFilePath);
	virtual HRESULT Initialize_Prototype(const TERRAIN_DESC& desc);
	virtual HRESULT Initialize(void* pArg);

private:
	_uint			m_iNumVerticesX = {};
	_uint			m_iNumVerticesZ = {};

public:
	static CVIBuffer_Terrain* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pHeightMapFilePath);
	static CVIBuffer_Terrain* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,const TERRAIN_DESC& desc);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

};
END_NAMESPACE

