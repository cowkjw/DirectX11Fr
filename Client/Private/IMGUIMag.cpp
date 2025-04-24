#include "IMGUIMag.h"
#include "GameInstance.h"

CIMGUIMag::CIMGUIMag(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pGameInstance{ CGameInstance::Get_Instance() },
	m_pDevice(pDevice),
	m_pContext(pContext)
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pDevice);
}


HRESULT CIMGUIMag::Initialize()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();               // ImGui 컨텍스트 생성
	ImGui::StyleColorsDark();             // 다크 테마 설정

	if (!ImGui_ImplWin32_Init(g_hWnd))   
		return E_FAIL;
	if (!ImGui_ImplDX11_Init(m_pDevice, m_pContext))
		return E_FAIL;

	return S_OK;
}

void CIMGUIMag::Update(_float fTimeDelta)
{
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX11_NewFrame();
	ImGui::NewFrame();
}

HRESULT CIMGUIMag::Render()
{
	ImGui::ShowDemoWindow();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    return S_OK;
}

CIMGUIMag* CIMGUIMag::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CIMGUIMag* pInstance = new CIMGUIMag(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CIMGUIMag");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CIMGUIMag::Free()
{
	__super::Free();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);
	Safe_Release(m_pGameInstance);
}
