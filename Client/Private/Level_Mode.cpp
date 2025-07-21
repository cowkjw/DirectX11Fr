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
	CJsonLoader jsonLoader;
	jsonLoader.Load_Objects("../Asset/Json/ModeObjects.json", [&]() {});

	jsonLoader.Load_Objects("../Asset/Json/InkCanvas.json", [&]() {});

	Ready_UI_Setup();

	CSoundMag::Get_Instance()->PlayBGM("event:/BGM/TitleBGM");
	jsonLoader.Free();
	return S_OK;
}

void CLevel_Mode::Update(_float fTimeDelta)
{
	UpdateButtonState();
	MoveCloud(fTimeDelta);
}

HRESULT CLevel_Mode::Render()
{
	return S_OK;
}


void CLevel_Mode::Ready_UI_Setup()
{
	m_pModeButton = dynamic_cast<CUIButton*>(
		m_pGameInstance->Get_UI(TEXT("ModeCanvas"), TEXT("ModeButton")));

	if (m_pModeButton)
	{
		m_pModeButton->Set_OnClick([this]() {
			StartGamePlay();
			});
	}

	m_pCursorImage = static_cast<CUIImage*>(
		m_pGameInstance->Get_UI(TEXT("ModeCanvas"), TEXT("Cursor")));


	const _tchar* names[4] = { L"CloudDarkLeft",L"CloudDarkRight",L"CloudLeft",L"CloudRight" };
	for (_int i = 0; i < 4; ++i)
	{

		m_CloudImages[i] = static_cast<CUIImage*>(
			m_pGameInstance->Get_UI(TEXT("ModeCanvas"), names[i]));
		m_vCloudOrigin[i] = m_CloudImages[i]->GetTransform()->Get_State(STATE::POSITION);
	}
	auto uiWind = static_cast<CUIImage*>(m_pGameInstance->Get_UI(TEXT("ModeCanvas"), TEXT("Wind")));
	if (uiWind)
	{
		uiWind->Set_Color(_float4(0.35f, 0.35f, 0.35f, 0.65f)); // 반투명하게 설정
	}
}

void CLevel_Mode::StartGamePlay()
{
	if (FAILED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING),
		CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::BATTLE))))
		return;
}

void CLevel_Mode::MoveCloud(_float fTimeDelta)
{
	m_fCloudTime += fTimeDelta;

	const _float ampDark = 5.f;            // 다크 구름 진폭
	const _float ampNormal = 10.f;            // 일반 구름 진폭
	const _float speed = 0.2f;            // 주기 조절 

	_float phase = m_fCloudTime * XM_2PI * speed;
	_float offsetDark = sinf(phase) * ampDark;
	_float offsetNormal = sinf(phase + XM_PI) * ampNormal;

	auto apply = [&](CUIImage* cloud, const _vector& origin, _float offsetX)
		{
			if (!cloud) 
				return;
			_vector pos = origin;
			pos = XMVectorSetX(pos, XMVectorGetX(origin) + offsetX);
			cloud->GetTransform()->Set_State(STATE::POSITION, pos);
		};

	apply(m_CloudImages[0], m_vCloudOrigin[0], offsetDark);
	apply(m_CloudImages[1], m_vCloudOrigin[1], -offsetDark);
	apply(m_CloudImages[2], m_vCloudOrigin[2], offsetNormal);
	apply(m_CloudImages[3], m_vCloudOrigin[3], -offsetNormal);

}

void CLevel_Mode::UpdateButtonState()
{
	if (!m_pModeButton) return;
	if (m_pModeButton->IsHovered())
	{
		if (m_pCursorImage && !m_pCursorImage->IsActive())
		{
			m_pCursorImage->SetActive(true);
			CSoundMag::Get_Instance()->PlayEffect("event:/UI/ModeSelectHover");
		}
		m_pModeButton->GetButtonImage()->SetActive(false);
	}
	else
	{
		if (m_pCursorImage) m_pCursorImage->SetActive(false);
		m_pModeButton->GetButtonImage()->SetActive(true);
	}
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
}
