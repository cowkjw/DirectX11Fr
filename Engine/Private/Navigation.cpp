#include "Navigation.h"

#include "Cell.h"
#include "GameInstance.h"

_float4x4		CNavigation::m_WorldMatrix = {};

CNavigation::CNavigation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent { pDevice, pContext }		
{
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());
}

CNavigation::CNavigation(const CNavigation& Prototype)
	: CComponent ( Prototype )
	, m_Cells { Prototype.m_Cells }
	, m_iIndex { Prototype.m_iIndex } 	
#ifdef _DEBUG
	, m_pShader { Prototype.m_pShader }
#endif
{	 

	for (auto& pCell : m_Cells)
		Safe_AddRef(pCell);

#ifdef _DEBUG
	Safe_AddRef(m_pShader);
#endif

}


HRESULT CNavigation::Initialize_Prototype(const _tchar* pNavigationDataFile)
{
	//_ulong	dwByte = {};
	//HANDLE	hFile = CreateFile(pNavigationDataFile, GENERIC_READ, 0, nullptr,
	//	OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	//if (0 == hFile)
	//	return E_FAIL;

	//while (true)
	//{
	//	_float3		vPoints[3] = {};

	//	ReadFile(hFile, vPoints, sizeof(_float3) * 3, &dwByte, nullptr);

	//	if (0 == dwByte)
	//		break;

	//	CCell* pCell = CCell::Create(m_pDevice, m_pContext, vPoints, m_Cells.size());
	//	if (nullptr == pCell)
	//		return E_FAIL;

	//	m_Cells.push_back(pCell);
	//}

	//CloseHandle(hFile);

	//if (FAILED(SetUp_Neighbors()))
	//	return E_FAIL;


#ifdef _DEBUG
	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Asset/ShaderFiles/Shader_Cell.hlsl"), VTXPOS::Elements, VTXPOS::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;

#endif


	if (pNavigationDataFile == nullptr)
		return S_OK;
	HRESULT hr = LoadCells(pNavigationDataFile);
	if (FAILED(hr))
		return hr;





	return S_OK;
}

HRESULT CNavigation::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return S_OK;

	NAVIGATION_DESC* pDesc = static_cast<NAVIGATION_DESC*>(pArg);

	m_iIndex = pDesc->iIndex;

	return S_OK;
}

void CNavigation::Update(_fmatrix WorldMatrix)
{
	XMStoreFloat4x4(&m_WorldMatrix, WorldMatrix);
}

_bool CNavigation::isMove(_fvector vWorldPos)
{
	//_vector		vLocalPos = XMVector3TransformCoord(vWorldPos, XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_WorldMatrix)));

	_int		iNeighborIndex = { -1 };

	if (m_iIndex < 0 || m_iIndex >= static_cast<_int>(m_Cells.size()))
	{
		/* 인덱스가 유효하지 않다면 */
		m_iIndex = FindIndexCell(vWorldPos);
		if (m_iIndex < 0)
			return false;
	}
	if (true == m_Cells[m_iIndex]->isIn(vWorldPos, &iNeighborIndex))
		return true;

	else
	{
		if(-1 == iNeighborIndex)
		/* 이웃이 없다면 */
			return false;

		else
		{
			while (true)
			{
				if (true == m_Cells[iNeighborIndex]->isIn(vWorldPos, &iNeighborIndex))
					break;

				if (-1 == iNeighborIndex)
					return false;
			}

			m_iIndex = iNeighborIndex;

			/* 이웃이 있다면 */
			return true;
		}
	}	
}

_vector CNavigation::SetUp_Height(_fvector vWorldPos)
{
	_vector		vLocalPos = XMVector3TransformCoord(vWorldPos, XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_WorldMatrix)));

	vLocalPos = XMVectorSetY(vLocalPos, m_Cells[m_iIndex]->Compute_Height(vLocalPos));

	return XMVector3TransformCoord(vLocalPos, XMLoadFloat4x4(&m_WorldMatrix));
}

HRESULT CNavigation::SaveCells(const _tchar* pFilePath)
{
	// 파일 열기
	HANDLE hFile = CreateFile(pFilePath, GENERIC_WRITE, 0, nullptr,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return E_FAIL;

	_ulong dwByte = 0;
	// 셀 개수 헤더로 쓰기 
	uint32_t count = static_cast<uint32_t>(m_Cells.size());
	if (!WriteFile(hFile, &count, sizeof(count), &dwByte, nullptr)
		|| dwByte != sizeof(count))
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	// 각 셀의 3점씩 쓰기
	for (auto& pCell : m_Cells)
	{
		_float3 a, b, c;
		XMStoreFloat3(&a, pCell->Get_Point(CCell::POINT_A));
		XMStoreFloat3(&b, pCell->Get_Point(CCell::POINT_B));
		XMStoreFloat3(&c, pCell->Get_Point(CCell::POINT_C));
		_float3 vPoints[3] = { a, b, c };

		if (!WriteFile(hFile, vPoints, sizeof(vPoints), &dwByte, nullptr)
			|| dwByte != sizeof(vPoints))
		{
			CloseHandle(hFile);
			return E_FAIL;
		}
	}

	CloseHandle(hFile);
	return S_OK;
}

HRESULT CNavigation::LoadCells(const _tchar* pFilePath)
{
	for (auto& c : m_Cells) Safe_Release(c);
	m_Cells.clear();

	// 파일 열기
	HANDLE hFile = CreateFile(pFilePath, GENERIC_READ, 0, nullptr,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return E_FAIL;

	_ulong dwByte = 0;
	// 셀 개수 읽기 
	uint32_t count = 0;
	if (!ReadFile(hFile, &count, sizeof(count), &dwByte, nullptr)
		|| dwByte != sizeof(count))
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	// 개수만큼 반복해서 읽기
	for (uint32_t i = 0; i < count; ++i)
	{
		_float3 vPoints[3] = {};
		if (!ReadFile(hFile, vPoints, sizeof(vPoints), &dwByte, nullptr)
			|| dwByte != sizeof(vPoints))
		{
			CloseHandle(hFile);
			return E_FAIL;
		}

		CCell* pCell = CCell::Create(m_pDevice, m_pContext, vPoints, static_cast<int>(m_Cells.size()));
		if (!pCell)
		{
			CloseHandle(hFile);
			return E_FAIL;
		}
		m_Vertices.push_back(vPoints[0]);
		m_Vertices.push_back(vPoints[1]);
		m_Vertices.push_back(vPoints[2]);
		m_Cells.push_back(pCell);
	}

	CloseHandle(hFile);

	return SetUp_Neighbors();
}

void CNavigation::AddCell(CCell* pCell)
{
	if (!pCell) return;

	// 1) 입력받은 세 점을 스냅
	_float3 a, b, c;

	XMStoreFloat3(&a, pCell->Get_Point(CCell::POINT_A));
	XMStoreFloat3(&b, pCell->Get_Point(CCell::POINT_B));
	XMStoreFloat3(&c, pCell->Get_Point(CCell::POINT_C));
	SnapVertex(a,5.f);
	SnapVertex(b,5.f);
	SnapVertex(c,5.f);

	// 2) 시계방향 보정 (CrossZ > 0 이면 CCW)
	if (CrossZ(a, b, c) > 0.0f)
		swap(b, c);

	_float3 vPoints[3] = { a, b, c };
	// 3) 스냅&정렬된 좌표로 새로운 셀 생성
	CCell* newCell = CCell::Create(
		m_pDevice, m_pContext,
		vPoints,
		static_cast<int>(m_Cells.size())
	);
	if (!newCell) return;
	_vector pa = XMLoadFloat3(&a);
	_vector pb = XMLoadFloat3(&b);
	_vector pc = XMLoadFloat3(&c);
	// 4) 기존 셀들과 인접 검사 (이제는 Compare가 내부에서 == 으로 해도 OK)
	for (auto* ex : m_Cells)
	{
		if (ex->Compare(pa, pb))
		{
			ex->Set_Neighbor(CCell::LINE_AB, newCell);
			newCell->Set_Neighbor(CCell::LINE_AB, ex);
		}
		if (ex->Compare(pb, pc))
		{
			ex->Set_Neighbor(CCell::LINE_BC, newCell);
			newCell->Set_Neighbor(CCell::LINE_BC, ex);
		}
		if (ex->Compare(pc, pa))
		{
			ex->Set_Neighbor(CCell::LINE_CA, newCell);
			newCell->Set_Neighbor(CCell::LINE_CA, ex);
		}
	}

	// 5) 리스트에 추가
	m_Cells.push_back(newCell);
	Safe_Release(pCell);
}

void CNavigation::ClearCells()
{
	for (auto& pCell : m_Cells)
	{
		Safe_Release(pCell);
	}
	m_Cells.clear();
	m_iIndex = -1; // 초기화
}

_int CNavigation::FindIndexCell(const _vector& vWorldPos)
{
	for (size_t i = 0; i < m_Cells.size(); ++i)
	{
		_int nd = -1;
		if (m_Cells[i]->isIn(vWorldPos, &nd))
		{
			m_iIndex = static_cast<_int>(i);
			return m_iIndex;
		}
	}
	m_iIndex = -1; // 해당 셀을 찾지 못한 경우
	return m_iIndex;
}

void CNavigation::DeleteCell(const _vector& vWorldPos)
{
	auto iIndex = FindIndexCell(vWorldPos);
	if (iIndex != -1)
	{
		CCell* pCellToDelete = m_Cells[iIndex];
		m_Cells.erase(m_Cells.begin() + iIndex);
		Safe_Release(pCellToDelete);
		SetUp_Neighbors();
		m_iIndex = -1; // 삭제 후 인덱스 초기화
	}
}

_vector CNavigation::GetHitCellNormal(_int iIndex)
{
	if (iIndex < 0 || iIndex >= (int)m_Cells.size())
		return XMVectorSet(0.f, 1.f, 0.f, 0.f);

	_vector vA = m_Cells[iIndex]->Get_Point(CCell::POINT_A);
	_vector vB = m_Cells[iIndex]->Get_Point(CCell::POINT_B);
	_vector vC = m_Cells[iIndex]->Get_Point(CCell::POINT_C);
	_vector vAB = XMVectorSubtract(vB, vA);
	_vector vAC = XMVectorSubtract(vC, vA);
	return XMVector3Normalize(XMVector3Cross(vAB, vAC));
}

void CNavigation::DeleteCell(_int iIndex)
{
	if (iIndex < 0 || iIndex >= (int)m_Cells.size())
		return;
	CCell* pCellToDelete = m_Cells[iIndex];
	m_Cells.erase(m_Cells.begin() + iIndex);
	Safe_Release(pCellToDelete);
	SetUp_Neighbors();
	m_iIndex = -1; // 삭제 후 인덱스 초기화
}
#ifdef _DEBUG

HRESULT CNavigation::Render()
{	
	m_pShader->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::VIEW));
	m_pShader->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::PROJECTION));

	_float4		vColor = {};
	//m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix);

	//vColor = _float4(0.f, 1.f, 0.f, 1.f);

	//m_pShader->Bind_RawValue("g_vColor", &vColor, sizeof(_float4));

	//m_pShader->Begin(0);

	//for (auto& pCell : m_Cells)
	//	pCell->Render();
	if (-1 == m_iIndex)
	{
		m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix);

		vColor = _float4(0.f, 1.f, 0.f, 1.f);		

		m_pShader->Bind_RawValue("g_vColor", &vColor, sizeof(_float4));

		m_pShader->Begin(0);

		for (auto& pCell : m_Cells)
			pCell->Render();
	}
	else
	{
		_float4x4		WorldMatrix = m_WorldMatrix;
		WorldMatrix.m[3][1] += 0.1f;

		m_pShader->Bind_Matrix("g_WorldMatrix", &WorldMatrix);

		vColor = _float4(1.f, 0.f, 0.f, 1.f);

		m_pShader->Bind_RawValue("g_vColor", &vColor, sizeof(_float4));

		m_pShader->Begin(0);
		
		m_Cells[m_iIndex]->Render();
	}

	return S_OK;
}
_bool CNavigation::SnapVertex(_float3& p, _float eps)
{
	for (auto& v : m_Vertices)
	{
		if (fabs(v.x - p.x) < eps
			&& fabs(v.y - p.y) < eps
			&& fabs(v.z - p.z) < eps)
		{
			p = v;       // 기존 풀의 좌표로 스냅
			return true;
		}
	}
	// 풀에 없는 점이면 새로운 정점으로 추가
	m_Vertices.push_back(p);
	return false;
}
#endif


HRESULT CNavigation::SetUp_Neighbors()
{
	for (auto& pSourCell : m_Cells)
	{
		for (auto& pDestCell : m_Cells)
		{
			if (pSourCell == pDestCell)
				continue;

			if (true == pDestCell->Compare(pSourCell->Get_Point(CCell::POINT_A), pSourCell->Get_Point(CCell::POINT_B)))
			{
				pSourCell->Set_Neighbor(CCell::LINE_AB, pDestCell);
			}

			if (true == pDestCell->Compare(pSourCell->Get_Point(CCell::POINT_B), pSourCell->Get_Point(CCell::POINT_C)))
			{
				pSourCell->Set_Neighbor(CCell::LINE_BC, pDestCell);
			}

			if (true == pDestCell->Compare(pSourCell->Get_Point(CCell::POINT_C), pSourCell->Get_Point(CCell::POINT_A)))
			{
				pSourCell->Set_Neighbor(CCell::LINE_CA, pDestCell);
			}
		}
	}

	return S_OK;
}

CNavigation* CNavigation::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pNavigationDataFile)
{
	CNavigation* pInstance = new CNavigation(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pNavigationDataFile)))
	{
		MSG_BOX("Failed to Created : CNavigation");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CNavigation::Clone(void* pArg)
{
	CNavigation* pInstance = new CNavigation(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CNavigation");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CNavigation::Free()
{
	__super::Free();

	for (auto& pCell : m_Cells)
		Safe_Release(pCell);

#ifdef _DEBUG
	Safe_Release(m_pShader);
#endif	
}
