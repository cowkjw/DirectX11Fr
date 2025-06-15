#include "Level_Mode.h"

#include "GameInstance.h"
#include "Level_Loading.h"
#include "JsonLoader.h"
#include <UIButton.h>
#include "UIImage.h"
#include "UICanvas.h"
CLevel_Mode::CLevel_Mode(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
		: CLevel { pDevice, pContext }
{

}

HRESULT CLevel_Mode::Initialize()
{
	//if (FAILED(Ready_Layer_BackGround(TEXT("Layer_BackGround"))))
	//	return E_FAIL;


 //auto pStartButton = dynamic_cast<CUIButton*>(m_pGameInstance->Get_UI(TEXT("TitleCanvas"), TEXT("StartButton")));

 //if (pStartButton)
 //{
	// pStartButton->Set_OnClick([this]() {
	//	 StartGamePlay();
	//	 });
 //}


	CJsonLoader jsonLoader;
	jsonLoader.Load_Objects("../Asset/Json/ModeObjects.json", [&]() {
		// 이곳에 로드 후 처리할 작업을 추가합니다.
		});

	jsonLoader.Load_Objects("../Asset/Json/InkCanvas.json", [&]() {
		// 이곳에 로드 후 처리할 작업을 추가합니다.
		});

	Ready_UI_Setup();


	return S_OK;
}

void CLevel_Mode::Update(_float fTimeDelta)
{



	POINT pt = m_pGameInstance->GetMousePos();

	{
		char buf[64];
		// 포맷팅: 변수 pt.x, pt.y를 문자열에 삽입
		sprintf_s(buf, "MousePos: %d, %d", pt.x, pt.y);
		// 윈도우 타이틀(또는 컨트롤)에 출력
		SetWindowTextA(g_hWnd, buf);
	}
	
	auto pButton = m_pGameInstance->Get_UI(TEXT("ModeCanvas"), TEXT("ModeButton"));
	auto pCursorImage = m_pGameInstance->Get_UI(TEXT("ModeCanvas"), TEXT("Cursor"));
	if (auto pModeButton = dynamic_cast<CUIButton*>(pButton))
	{
		if (pModeButton->IsHovered())
		{
			
			if (pCursorImage)
			{
				pCursorImage->SetActive(true);
			}
			pModeButton->GetButtonImage()->SetActive(false);
		}
		else
		{
			if (pCursorImage)
			{
				pCursorImage->SetActive(false);
			}
			pModeButton->GetButtonImage()->SetActive(true);
		}
	}

	MoveCloud(fTimeDelta);

}

HRESULT CLevel_Mode::Render()
{
//	SetWindowText(g_hWnd, TEXT("로고레벨입니다."));

	return S_OK;
}

HRESULT CLevel_Mode::Ready_Layer_BackGround(const _wstring strLayerTag)
{

  ///*  CUICanvas::UIOBJECT_DESC CanvasDesc{};
  //  CanvasDesc.fX = g_iWinSizeX * 0.5f;
  //  CanvasDesc.fY = g_iWinSizeY * 0.5f;
  //  CanvasDesc.fSizeX = g_iWinSizeX;
  //  CanvasDesc.fSizeY = g_iWinSizeY;
  //  CanvasDesc.strName = L"TitleCanvas";

  //  auto pUICanvas = CTitleCanvas::Create(m_pDevice, m_pContext);
  //  if (!pUICanvas)
  //      return E_FAIL;

  //  if (FAILED(pUICanvas->Initialize(&CanvasDesc)))
  //  {
  //      Safe_Release(pUICanvas);
  //      return E_FAIL;
  //  }
  //  m_pGameInstance->AddCanvasUI(pUICanvas);*/

    return S_OK;
}

void CLevel_Mode::Ready_UI_Setup()
{
	auto pUICanvas = m_pGameInstance->Get_UI(TEXT("ModeCanvas"), TEXT("ModeButton"));
	if (pUICanvas)
	{
		auto pButton = dynamic_cast<CUIButton*>(pUICanvas);
		if (pButton)
		{
			pButton->Set_OnClick([this]() {
				StartGamePlay();
				});
		}
	}
	auto uiDarkL = static_cast<CUIImage*>(m_pGameInstance->Get_UI(TEXT("ModeCanvas"), TEXT("CloudDarkLeft")));
	auto uiDarkR = static_cast<CUIImage*>(m_pGameInstance->Get_UI(TEXT("ModeCanvas"), TEXT("CloudDarkRight")));
	auto uiL = static_cast<CUIImage*>(m_pGameInstance->Get_UI(TEXT("ModeCanvas"), TEXT("CloudLeft")));
	auto uiR = static_cast<CUIImage*>(m_pGameInstance->Get_UI(TEXT("ModeCanvas"), TEXT("CloudRight")));

	//초기 위치 저장
	if (uiDarkL) m_vCloudOrigin[0] = uiDarkL->GetTransform()->Get_State(STATE::POSITION);
	if (uiDarkR) m_vCloudOrigin[1] = uiDarkR->GetTransform()->Get_State(STATE::POSITION);
	if (uiL)     m_vCloudOrigin[2] = uiL->GetTransform()->Get_State(STATE::POSITION);
	if (uiR)     m_vCloudOrigin[3] = uiR->GetTransform()->Get_State(STATE::POSITION);

	auto uiWind = static_cast<CUIImage*>(m_pGameInstance->Get_UI(TEXT("ModeCanvas"), TEXT("Wind")));
	if (uiWind)
	{
		uiWind->Set_Color(_float4(0.35f, 0.35f, 0.35f, 0.65f)); // 반투명하게 설정
	}

}

void CLevel_Mode::StartGamePlay()
{
	if (FAILED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING),
		CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::GAMEPLAY))))
		return;
}

void CLevel_Mode::MoveCloud(_float fTimeDelta)
{
	m_fCloudTime += fTimeDelta;

	// 파라미터: 진폭(amplitude)과 속도(speed)
	const _float ampDark = 5.f;            // 다크 구름 진폭
	const _float ampNormal = 10.f;            // 일반 구름 진폭
	const _float speed = 0.2f;            // 주기 조절 (Hz)

	// 0~2π 속도로 시간에 따라 변화
	_float phase = m_fCloudTime * XM_2PI * speed;

	// Dark 구름 offset: sin(phase) * ampDark
	_float offsetDark = sinf(phase) * ampDark;
	// 일반 구름은 180°(π) 위상차
	_float offsetNormal = sinf(phase + XM_PI) * ampNormal;

	auto apply = [&](CUIImage* cloud, const _vector& origin, _float offsetX)
		{
			if (!cloud) return;
			_vector pos = origin;
			pos = XMVectorSetX(pos, XMVectorGetX(origin) + offsetX);
			cloud->GetTransform()->Set_State(STATE::POSITION, pos);
		};

	auto uiDarkL = static_cast<CUIImage*>(m_pGameInstance->Get_UI(TEXT("ModeCanvas"), TEXT("CloudDarkLeft")));
	auto uiDarkR = static_cast<CUIImage*>(m_pGameInstance->Get_UI(TEXT("ModeCanvas"), TEXT("CloudDarkRight")));
	auto uiL = static_cast<CUIImage*>(m_pGameInstance->Get_UI(TEXT("ModeCanvas"), TEXT("CloudLeft")));
	auto uiR = static_cast<CUIImage*>(m_pGameInstance->Get_UI(TEXT("ModeCanvas"), TEXT("CloudRight")));

	apply(uiDarkL, m_vCloudOrigin[0], offsetDark);
	apply(uiDarkR, m_vCloudOrigin[1], -offsetDark);
	apply(uiL, m_vCloudOrigin[2], offsetNormal);
	apply(uiR, m_vCloudOrigin[3], -offsetNormal);

}


CLevel_Mode* CLevel_Mode::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_Mode* pInstance = new CLevel_Mode(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_Mode");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_Mode::Free()
{
	__super::Free();
	Safe_Release(m_pGameInstance);
}
