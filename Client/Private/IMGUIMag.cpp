#include "IMGUIMag.h"
#include "EditorManager.h"
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
	ImGuiStyle& style = ImGui::GetStyle();
//	style.Colors[ImGuiCol_WindowBg].w = 0.5f;  // 전체 창 투명도 설정
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // ★ 도킹 활성화
	//	| ImGuiConfigFlags_ViewportsEnable; // (선택) 여러 뷰포트
	if (!ImGui_ImplWin32_Init(g_hWnd))   
		return E_FAIL;
	if (!ImGui_ImplDX11_Init(m_pDevice, m_pContext))
		return E_FAIL;

	m_pEditorMag = CEditorManager::Create(m_pDevice, m_pContext);
	if (m_pEditorMag == nullptr)
		return E_FAIL;

	ImNodes::CreateContext();
	return S_OK;
}

void CIMGUIMag::Update(_float fTimeDelta)
{
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX11_NewFrame();
	ImGui::NewFrame();
	m_pEditorMag->Update(fTimeDelta);
}

HRESULT CIMGUIMag::Render()
{
	//ImGui::ShowDemoWindow();
	Docking();
	m_pEditorMag->Render();


	ImGui::End();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    return S_OK;
}

void CIMGUIMag::Docking()
{
	ImGuiViewport* vp = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(vp->Pos);
	ImGui::SetNextWindowSize(vp->Size);
	ImGui::SetNextWindowViewport(vp->ID);
	// 모서리 둥글게/테두리 없애기
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGuiWindowFlags host_flags =
		ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoBringToFrontOnFocus
		| ImGuiWindowFlags_NoNavFocus
		| ImGuiWindowFlags_NoBackground;
	ImGui::Begin("##DockHost", nullptr, host_flags);
	ImGui::PopStyleVar(2);

	// 2) 도킹 스페이스 생성 (남은 중앙 영역을 자동으로 채움)
	ImGuiID dock_id = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(dock_id, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);

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
	ImNodes::DestroyContext();
	ImGui::DestroyContext();

	Safe_Release(m_pEditorMag);
	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);
	Safe_Release(m_pGameInstance);
}
