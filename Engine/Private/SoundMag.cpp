#include "SoundMag.h"

IMPLEMENT_SINGLETON(CSoundMag)

HRESULT CSoundMag::Initialize(_int iMaxChannels)
{
    FMOD_RESULT result;

    // Studio 시스템 생성
    result = FMOD::Studio::System::create(&studioSystem);
    if (!CheckResult(result, "Studio System Create"))
        return E_FAIL;

    // Core 시스템 설정
    FMOD::System* core = nullptr;
    result = studioSystem->getCoreSystem(&core);
    if (!CheckResult(result, "Get Core System"))
        return E_FAIL;

    result = core->setSoftwareChannels(iMaxChannels);
    if (!CheckResult(result, "Set Software Channels"))
        return E_FAIL;

    // 초기화
    result = studioSystem->initialize(iMaxChannels, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr);
    if (!CheckResult(result, "Studio System Initialize"))
        return E_FAIL;

   

    return S_OK;
}

void CSoundMag::Update()
{
    if (studioSystem)
        studioSystem->update();
}

void CSoundMag::Shutdown()
{
    StopAll(false);

    // 이펙트 인스턴스들 정리
    for (auto& pair : effectInstances)
    {
        if (pair.second)
        {
            pair.second->stop(FMOD_STUDIO_STOP_IMMEDIATE);
            pair.second->release();
        }
    }
    effectInstances.clear();

    // BGM 인스턴스 정리
    if (bgmInstance)
    {
        bgmInstance->stop(FMOD_STUDIO_STOP_IMMEDIATE);
        bgmInstance->release();
        bgmInstance = nullptr;
    }

    // 뱅크 언로드
    if (stringsBank)
    {
        stringsBank->unload();
        stringsBank = nullptr;
    }

    if (masterBank)
    {
        masterBank->unload();
        masterBank = nullptr;
    }

    // 시스템 해제
    if (studioSystem)
    {
        studioSystem->unloadAll();
        studioSystem->release();
        studioSystem = nullptr;
    }

    for (auto b : extraBanks)
        if (b) b->unload();
    extraBanks.clear();
    masterBus = nullptr;
    eventDescriptions.clear();
}

_bool CSoundMag::LoadBanks(const char* bankPath, const char* stringsPath)
{
    FMOD_RESULT result;

    // Master Bank 로드
    result = studioSystem->loadBankFile(bankPath, FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank);
    if (!CheckResult(result, "Load Master Bank"))
        return false;

    // Strings Bank 로드
    result = studioSystem->loadBankFile(stringsPath, FMOD_STUDIO_LOAD_BANK_NORMAL, &stringsBank);
    if (!CheckResult(result, "Load Strings Bank"))
        return false;

    // 마스터 버스 얻기
    result = studioSystem->getBus("bus:/", &masterBus);
    if (!CheckResult(result, "Get Master Bus"))
        return false;

    return true;
}

_bool CSoundMag::LoadBank(const char* bankPath)
{
    if (!studioSystem) 
        return false;
    FMOD::Studio::Bank* bank = nullptr;
    if (!CheckResult(studioSystem->loadBankFile(bankPath, FMOD_STUDIO_LOAD_BANK_NORMAL, &bank), "Load Extra Bank")) 
        return false;
    extraBanks.push_back(bank);
    return true;
}

_bool CSoundMag::LoadAllBanks(const string& directoryPath)
{
    if (!studioSystem)
        return false;
    string search = directoryPath + "\\*.bank";
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(search.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE)
        return false;
    do {
       string path = directoryPath + "\\" + fd.cFileName;
        LoadBank(path.c_str());
    } while (FindNextFileA(h, &fd));
   
    FindClose(h);
    return true;
}

void CSoundMag::PlayOneShot(const string& eventPath)
{
    auto desc = GetEventDesc(eventPath);
    if (!desc)
        return;

    FMOD::Studio::EventInstance* inst = nullptr;
    if (CheckResult(desc->createInstance(&inst), "Create OneShot Instance"))
    {
        inst->start();
        inst->release(); // 재생 후 자동 해제
    }
}

void CSoundMag::PlayEffect(const string& eventPath, const string& instanceName)
{
    auto desc = GetEventDesc(eventPath);
    if (!desc)
        return;

    string name = instanceName.empty() ? eventPath : instanceName;

    // 기존 인스턴스가 있으면 정지하고 제거
    auto it = effectInstances.find(name);
    if (it != effectInstances.end())
    {
        if (it->second)
        {
            it->second->stop(FMOD_STUDIO_STOP_IMMEDIATE);
            it->second->release();
        }
        effectInstances.erase(it);
    }

    // 새 인스턴스 생성
    FMOD::Studio::EventInstance* inst = nullptr;
    if (CheckResult(desc->createInstance(&inst), "Create Effect Instance"))
    {
        effectInstances[name] = inst;
        inst->start();
    }
}

void CSoundMag::StopEffect(const string& instanceName)
{
    auto it = effectInstances.find(instanceName);
    if (it != effectInstances.end() && it->second)
    {
        it->second->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
        it->second->release();
        effectInstances.erase(it);
    }
}

void CSoundMag::PlayBGM(const string& eventPath)
{
    StopBGM();

    auto desc = GetEventDesc(eventPath);
    if (!desc)
        return;

    if (CheckResult(desc->createInstance(&bgmInstance), "Create BGM Instance"))
    {
        // FMOD Studio에서 루프는 이벤트 자체에서 설정됨
        // 또는 파라미터로 제어 가능
        bgmInstance->start();
    }
}

void CSoundMag::SetBGMLoop(_bool loop)
{
    if (bgmInstance)
    {
        // 이벤트에 "Loop" 파라미터가 있다면 이렇게 설정
        bgmInstance->setParameterByName("Loop", loop ? 1.0f : 0.0f);
    }
}

void CSoundMag::StopBGM()
{
    if (bgmInstance)
    {
        bgmInstance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
        bgmInstance->release();
        bgmInstance = nullptr;
    }
}

void CSoundMag::PauseBGM(_bool pause)
{
    if (bgmInstance)
    {
        bgmInstance->setPaused(pause);
    }
}

void CSoundMag::StopAll(_bool allowFadeout)
{
    FMOD_STUDIO_STOP_MODE stopMode = allowFadeout ? FMOD_STUDIO_STOP_ALLOWFADEOUT : FMOD_STUDIO_STOP_IMMEDIATE;

    // BGM 정지
    if (bgmInstance)
    {
        bgmInstance->stop(stopMode);
        bgmInstance->release();
        bgmInstance = nullptr;
    }

    // 모든 이펙트 인스턴스 정지
    for (auto& pair : effectInstances)
    {
        if (pair.second)
        {
            pair.second->stop(stopMode);
            pair.second->release();
        }
    }
    effectInstances.clear();

    // 시스템 레벨에서 모든 이벤트 정지
    if (studioSystem)
    {
        studioSystem->flushCommands();
    }
}

void CSoundMag::PauseAll(_bool pause)
{
    if (bgmInstance)
    {
        bgmInstance->setPaused(pause);
    }

    for (auto& pair : effectInstances)
    {
        if (pair.second)
        {
            pair.second->setPaused(pause);
        }
    }
}

void CSoundMag::StopAllEffects(_bool allowFadeout)
{
	FMOD_STUDIO_STOP_MODE stopMode = allowFadeout ? FMOD_STUDIO_STOP_ALLOWFADEOUT : FMOD_STUDIO_STOP_IMMEDIATE;
	// 모든 이펙트 인스턴스 정지
	for (auto& pair : effectInstances)
	{
		if (pair.second)
		{
			pair.second->stop(stopMode);
			pair.second->release();
		}
	}
	effectInstances.clear();
}

void CSoundMag::SetBusVolume(const string& busPath, _float fVolume)
{
    FMOD::Studio::Bus* bus = nullptr;
    FMOD_RESULT result = studioSystem->getBus(busPath.c_str(), &bus);
    if (CheckResult(result, "Get Bus") && bus)
    {
        bus->setVolume(fVolume);
    }
}

_float CSoundMag::GetBusVolume(const string& busPath)
{
    FMOD::Studio::Bus* bus = nullptr;
    FMOD_RESULT result = studioSystem->getBus(busPath.c_str(), &bus);
    if (CheckResult(result, "Get Bus Volume") && bus)
    {
        float volume = 0.0f;
        bus->getVolume(&volume);
        return volume;
    }
    return 0.0f;
}

void CSoundMag::SetMasterVolume(_float fVolume)
{
    if (masterBus)
    {
        masterBus->setVolume(fVolume);
    }
}

_float CSoundMag::GetMasterVolume()
{
    if (masterBus)
    {
        float volume = 0.0f;
        masterBus->getVolume(&volume);
        return volume;
    }
    return 0.0f;
}

void CSoundMag::SetEventParameter(const string& instanceName, const string& paramName, _float value)
{
    auto it = effectInstances.find(instanceName);
    if (it != effectInstances.end() && it->second)
    {
        it->second->setParameterByName(paramName.c_str(), value);
    }

    // BGM 파라미터 설정
    if (bgmInstance && instanceName == "BGM")
    {
        bgmInstance->setParameterByName(paramName.c_str(), value);
    }
}

FMOD::Studio::EventDescription* CSoundMag::GetEventDesc(const std::string& path)
{
    auto it = eventDescriptions.find(path);
    if (it != eventDescriptions.end())
        return it->second;

    FMOD::Studio::EventDescription* desc = nullptr;
    FMOD_RESULT result = studioSystem->getEvent(path.c_str(), &desc);
    if (CheckResult(result, "Get Event Description") && desc)
    {
        eventDescriptions[path] = desc;
        return desc;
    }

    return nullptr;
}

_bool CSoundMag::CheckResult(FMOD_RESULT result, const char* operation)
{
    if (result != FMOD_OK)
    {
        std::cerr << "FMOD error";
        if (operation && strlen(operation) > 0)
            std::cerr << " [" << operation << "]";
        std::cerr << " (" << result << ")";

        // 기본적인 에러 메시지
        switch (result)
        {
        case FMOD_ERR_BADCOMMAND:
            std::cerr << ": Bad command"; break;
        case FMOD_ERR_CHANNEL_ALLOC:
            std::cerr << ": Channel allocation failed"; break;
        case FMOD_ERR_CHANNEL_STOLEN:
            std::cerr << ": Channel stolen"; break;
        case FMOD_ERR_DMA:
            std::cerr << ": DMA error"; break;
        case FMOD_ERR_DSP_CONNECTION:
            std::cerr << ": DSP connection error"; break;
        case FMOD_ERR_DSP_DONTPROCESS:
            std::cerr << ": DSP don't process"; break;
        case FMOD_ERR_DSP_FORMAT:
            std::cerr << ": DSP format error"; break;
        case FMOD_ERR_DSP_INUSE:
            std::cerr << ": DSP in use"; break;
        case FMOD_ERR_DSP_NOTFOUND:
            std::cerr << ": DSP not found"; break;
        case FMOD_ERR_DSP_RESERVED:
            std::cerr << ": DSP reserved"; break;
        case FMOD_ERR_DSP_SILENCE:
            std::cerr << ": DSP silence"; break;
        case FMOD_ERR_DSP_TYPE:
            std::cerr << ": DSP type error"; break;
        case FMOD_ERR_FILE_BAD:
            std::cerr << ": Bad file"; break;
        case FMOD_ERR_FILE_COULDNOTSEEK:
            std::cerr << ": Could not seek file"; break;
        case FMOD_ERR_FILE_DISKEJECTED:
            std::cerr << ": Disk ejected"; break;
        case FMOD_ERR_FILE_EOF:
            std::cerr << ": End of file"; break;
        case FMOD_ERR_FILE_ENDOFDATA:
            std::cerr << ": End of data"; break;
        case FMOD_ERR_FILE_NOTFOUND:
            std::cerr << ": File not found"; break;
        case FMOD_ERR_FORMAT:
            std::cerr << ": Format error"; break;
        case FMOD_ERR_HEADER_MISMATCH:
            std::cerr << ": Header mismatch"; break;
        case FMOD_ERR_HTTP:
            std::cerr << ": HTTP error"; break;
        case FMOD_ERR_HTTP_ACCESS:
            std::cerr << ": HTTP access error"; break;
        case FMOD_ERR_HTTP_PROXY_AUTH:
            std::cerr << ": HTTP proxy auth error"; break;
        case FMOD_ERR_HTTP_SERVER_ERROR:
            std::cerr << ": HTTP server error"; break;
        case FMOD_ERR_HTTP_TIMEOUT:
            std::cerr << ": HTTP timeout"; break;
        case FMOD_ERR_INITIALIZATION:
            std::cerr << ": Initialization error"; break;
        case FMOD_ERR_INITIALIZED:
            std::cerr << ": Already initialized"; break;
        case FMOD_ERR_INTERNAL:
            std::cerr << ": Internal error"; break;
        case FMOD_ERR_INVALID_FLOAT:
            std::cerr << ": Invalid float"; break;
        case FMOD_ERR_INVALID_HANDLE:
            std::cerr << ": Invalid handle"; break;
        case FMOD_ERR_INVALID_PARAM:
            std::cerr << ": Invalid parameter"; break;
        case FMOD_ERR_INVALID_POSITION:
            std::cerr << ": Invalid position"; break;
        case FMOD_ERR_INVALID_SPEAKER:
            std::cerr << ": Invalid speaker"; break;
        case FMOD_ERR_INVALID_SYNCPOINT:
            std::cerr << ": Invalid sync point"; break;
        case FMOD_ERR_INVALID_THREAD:
            std::cerr << ": Invalid thread"; break;
        case FMOD_ERR_INVALID_VECTOR:
            std::cerr << ": Invalid vector"; break;
        case FMOD_ERR_MAXAUDIBLE:
            std::cerr << ": Max audible reached"; break;
        case FMOD_ERR_MEMORY:
            std::cerr << ": Memory error"; break;
        case FMOD_ERR_MEMORY_CANTPOINT:
            std::cerr << ": Can't point memory"; break;
        case FMOD_ERR_NEEDS3D:
            std::cerr << ": Needs 3D"; break;
        case FMOD_ERR_NEEDSHARDWARE:
            std::cerr << ": Needs hardware"; break;
        case FMOD_ERR_NET_CONNECT:
            std::cerr << ": Network connect error"; break;
        case FMOD_ERR_NET_SOCKET_ERROR:
            std::cerr << ": Network socket error"; break;
        case FMOD_ERR_NET_URL:
            std::cerr << ": Network URL error"; break;
        case FMOD_ERR_NET_WOULD_BLOCK:
            std::cerr << ": Network would block"; break;
        case FMOD_ERR_NOTREADY:
            std::cerr << ": Not ready"; break;
        case FMOD_ERR_OUTPUT_ALLOCATED:
            std::cerr << ": Output allocated"; break;
        case FMOD_ERR_OUTPUT_CREATEBUFFER:
            std::cerr << ": Output create buffer error"; break;
        case FMOD_ERR_OUTPUT_DRIVERCALL:
            std::cerr << ": Output driver call error"; break;
        case FMOD_ERR_OUTPUT_FORMAT:
            std::cerr << ": Output format error"; break;
        case FMOD_ERR_OUTPUT_INIT:
            std::cerr << ": Output init error"; break;
        case FMOD_ERR_OUTPUT_NODRIVERS:
            std::cerr << ": No output drivers"; break;
        case FMOD_ERR_PLUGIN:
            std::cerr << ": Plugin error"; break;
        case FMOD_ERR_PLUGIN_MISSING:
            std::cerr << ": Plugin missing"; break;
        case FMOD_ERR_PLUGIN_RESOURCE:
            std::cerr << ": Plugin resource error"; break;
        case FMOD_ERR_PLUGIN_VERSION:
            std::cerr << ": Plugin version error"; break;
        case FMOD_ERR_RECORD:
            std::cerr << ": Record error"; break;
        case FMOD_ERR_REVERB_CHANNELGROUP:
            std::cerr << ": Reverb channel group error"; break;
        case FMOD_ERR_REVERB_INSTANCE:
            std::cerr << ": Reverb instance error"; break;
        case FMOD_ERR_SUBSOUNDS:
            std::cerr << ": Subsounds error"; break;
        case FMOD_ERR_SUBSOUND_ALLOCATED:
            std::cerr << ": Subsound allocated"; break;
        case FMOD_ERR_SUBSOUND_CANTMOVE:
            std::cerr << ": Subsound can't move"; break;
        case FMOD_ERR_TAGNOTFOUND:
            std::cerr << ": Tag not found"; break;
        case FMOD_ERR_TOOMANYCHANNELS:
            std::cerr << ": Too many channels"; break;
        case FMOD_ERR_TRUNCATED:
            std::cerr << ": Truncated"; break;
        case FMOD_ERR_UNIMPLEMENTED:
            std::cerr << ": Unimplemented"; break;
        case FMOD_ERR_UNINITIALIZED:
            std::cerr << ": Uninitialized"; break;
        case FMOD_ERR_UNSUPPORTED:
            std::cerr << ": Unsupported"; break;
        case FMOD_ERR_VERSION:
            std::cerr << ": Version error"; break;
        case FMOD_ERR_EVENT_ALREADY_LOADED:
            std::cerr << ": Event already loaded"; break;
        case FMOD_ERR_EVENT_LIVEUPDATE_BUSY:
            std::cerr << ": Event live update busy"; break;
        case FMOD_ERR_EVENT_LIVEUPDATE_MISMATCH:
            std::cerr << ": Event live update mismatch"; break;
        case FMOD_ERR_EVENT_LIVEUPDATE_TIMEOUT:
            std::cerr << ": Event live update timeout"; break;
        case FMOD_ERR_EVENT_NOTFOUND:
            std::cerr << ": Event not found"; break;
        case FMOD_ERR_STUDIO_UNINITIALIZED:
            std::cerr << ": Studio uninitialized"; break;
        case FMOD_ERR_STUDIO_NOT_LOADED:
            std::cerr << ": Studio not loaded"; break;
        case FMOD_ERR_INVALID_STRING:
            std::cerr << ": Invalid string"; break;
        default:
            std::cerr << ": Unknown error"; break;
        }

        std::cerr << std::endl;
        return false;
    }
    return true;
}

void CSoundMag::Free()
{
    Shutdown();
}