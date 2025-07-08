#pragma once
#include "Base.h"
#include "Engine_Defines.h"

BEGIN_NAMESPACE(Engine)
class ENGINE_DLL CSoundMag final : public CBase
{
   DECLARE_SINGLETON(CSoundMag)
public:
    HRESULT Initialize(_int iMaxChannels = 512);
    void Update();
    void Shutdown();
    
    // 뱅크 파일 로드(Master + Strings)
    _bool LoadBanks(const char* bankPath, const char* stringsPath);
    _bool LoadBank(const char* bankPath);
    _bool LoadAllBanks(const string& directoryPath);
    
    // 원샷 재생 (Fire and Forget)
    void PlayOneShot(const string& eventPath);
    
    // 이펙트 재생 (관리 가능한 인스턴스)
    void PlayEffect(const string& eventPath, const string& instanceName = "");
    void StopEffect(const string& instanceName);
    
    // BGM 재생/정지
    void PlayBGM(const string& eventPath);
    void StopBGM();
    void PauseBGM(_bool pause);
    
    // BGM 루프 제어 (이벤트에 Loop 파라미터가 있는 경우)
    void SetBGMLoop(_bool loop);
    
    // 전체 사운드 제어
    void StopAll(_bool allowFadeout = false);
    void PauseAll(_bool pause);
	void StopAllEffects(_bool allowFadeout = false);
    
    // 볼륨 제어
    void SetBusVolume(const string& busPath, _float fVolume);
    _float GetBusVolume(const string& busPath);
    
    // 마스터 볼륨
    void SetMasterVolume(_float fVolume);
    _float GetMasterVolume();
    
    // 이벤트 파라미터 설정
    void SetEventParameter(const string& instanceName, const string& paramName, _float value);
    
private:
    CSoundMag() = default;
    virtual ~CSoundMag() = default;
    
private:
    FMOD::Studio::EventDescription* GetEventDesc(const std::string& path);
    _bool CheckResult(FMOD_RESULT result, const char* operation = "");
    
    FMOD::Studio::System* studioSystem = nullptr;
    FMOD::Studio::Bank* masterBank = nullptr;
    FMOD::Studio::Bank* stringsBank = nullptr;
    FMOD::Studio::EventInstance* bgmInstance = nullptr;
    
    // 이벤트 캐시
    unordered_map<string, FMOD::Studio::EventDescription*> eventDescriptions;
    
    // 관리되는 이펙트 인스턴스들
    unordered_map<string, FMOD::Studio::EventInstance*> effectInstances;
    
    // 마스터 버스
    FMOD::Studio::Bus* masterBus = nullptr;

    vector<FMOD::Studio::Bank*>                  extraBanks;
    
public:
    virtual void Free() override;
};
END_NAMESPACE

