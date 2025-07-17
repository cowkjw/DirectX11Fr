#include "BossIdle.h"
#include "BossFreezeAttack.h"
#include "BossSwingAttack.h"
#include "BossAngryFreeze.h"	
#include "BossFollowPunch.h"
#include "BossHandAttack.h"
#include "GameInstance.h"
#include "BossTentacle.h"
#include "BossPunch.h"
#include "BossDeath.h"
#include "BossHurt.h"
#include "BossOpen.h"

_bool BossIdle::m_bPatternUsed[6] = { false, false, false, false, false, false }; // 패턴 사용 여부 초기화

void BossIdle::Enter(CEnmuMeat* pChar)
{
	pChar->SetState(EnmuState::IDLE);
	m_fTimeElapsed = 0.f; // 시간 초기화

//    static bool s_consoleCreated = false;
//    if (!s_consoleCreated)
//    {
//        AllocConsole();
//        FILE* fpOut = nullptr;
//        freopen_s(&fpOut, "CONOUT$", "w", stdout);
//        FILE* fpIn = nullptr;
//        freopen_s(&fpIn, "CONIN$", "r", stdin);
//        cout << "=== Debug Console Opened ===" << std::endl;
//        s_consoleCreated = true;
//    }
}
void BossIdle::Update(CEnmuMeat* pChar, _float fTimeDelta)
{
    static _int lastHpThreshold = -1;
    static _int lastPatternIndex = -1;  // 마지막 사용한 패턴 기억
    static _int consecutiveCount = 0;   // 연속 사용 횟수

    _float fHp = pChar->GetHp();
    _float fMaxHp = pChar->GetMaxHp();
    _float fHpRatio = fHp / fMaxHp;

    cout << "[BossIdle] Current HP: " << pChar->GetHp() << endl;

  
    // 150씩 피가 떨어질 때 보스 열리도록
    _int currentHpThreshold = static_cast<_int>(fHp) / 150;
    if (lastHpThreshold == -1)
    {
        lastHpThreshold = currentHpThreshold;
    }
    else if (currentHpThreshold < lastHpThreshold && fHp > 0 && fHp <= 400.f)
    {
        pChar->ChangeState(new BossOpen(TEXT("BossOpen")));
        lastHpThreshold = currentHpThreshold;
        return;
    }

    // 쿨타임 감소
    auto DecayCD = [&](_float& cd)
        {
            if (cd > 0.f)
            {
                cd -= fTimeDelta;
                if (cd < 0.f)
                    cd = 0.f;
            }
        };

    DecayCD(pChar->m_CD_Punch);
    DecayCD(pChar->m_CD_Swing);
    DecayCD(pChar->m_CD_Hand);
    DecayCD(pChar->m_CD_Freeze);
    DecayCD(pChar->m_CD_FollowPunch);
    DecayCD(pChar->m_CD_Open);
    DecayCD(pChar->m_CD_Tentacle);

    // Idle 시간 증가
    m_fTimeElapsed += fTimeDelta;
    if (m_fTimeElapsed < 0.5f)
        return;

    _float fDist = pChar->GetDistanceToTarget();
    cout << "Distance to target: " << fDist << endl;

    // 거리별 사용 가능한 패턴 찾기 
    vector<pair<_int, _float>> availablePatterns; // 패턴 인덱스, 가중치

    // 거리 조건과 가중치 설정
    if (fDist >= -50.f && fDist <= 120.f)  // 근거리 범위 확장
    {
        availablePatterns.push_back({ 0, 1.0f }); // BossPunch
        if (fDist <= 100.f)
            availablePatterns.push_back({ 1, 0.8f }); // BossHandAttack 근거리에서도 사용 
    }
    if (fDist > 80.f && fDist <= 160.f)   // 중근거리
    {
        availablePatterns.push_back({ 1, 1.0f }); // BossHandAttack
        availablePatterns.push_back({ 2, 0.9f }); // BossFreezeAttack
    }
    if (fDist > 120.f && fDist <= 190.f)  // 중거리
    {
        availablePatterns.push_back({ 2, 1.0f }); // BossFreezeAttack
        availablePatterns.push_back({ 3, 1.0f }); // BossSwingAttack
    }
    if (fDist > 150.f && fDist <= 220.f)  
    {
        availablePatterns.push_back({ 3, 1.0f }); // BossSwingAttack
        availablePatterns.push_back({ 4, 1.0f }); // BossFollowPunch
    }
    if (fDist > 180.f)                    // 원거리
    {
        availablePatterns.push_back({ 4, 1.0f }); // BossFollowPunch
        availablePatterns.push_back({ 5, 1.0f }); // BossTentacle
    }

    // 사용 가능한 패턴이 없으면 대기
    if (availablePatterns.empty())
    {
        m_fTimeElapsed = 0.f;
        return;
    }

    // 쿨타임이 완료된 패턴 찾기
    vector<pair<_int, _float>> readyPatterns;
    for (auto& pattern : availablePatterns)
    {
        _int idx = pattern.first;
        _float weight = pattern.second;
        _bool isReady = false;

        switch (idx)
        {
        case 0: // BossPunch
            if (pChar->m_CD_Punch == 0.f)
                isReady = true;
            break;
        case 1: // BossHandAttack
            if (pChar->m_CD_Hand == 0.f)
                isReady = true;
            break;
        case 2: // BossFreezeAttack
            if (pChar->m_CD_Freeze == 0.f) 
                isReady = true;
            break;
        case 3: // BossSwingAttack
            if (pChar->m_CD_Swing == 0.f) 
                isReady = true;
            break;
        case 4: // BossFollowPunch
            if (pChar->m_CD_FollowPunch == 0.f) 
                isReady = true;
            break;
        case 5: // BossTentacle
            if (pChar->m_CD_Tentacle == 0.f)
                isReady = true;
            break;
        }

        if (isReady)
        {
            if (idx == lastPatternIndex)
            {
                if (consecutiveCount >= 2)
                    weight *= 0.3f;  // 3번 연속 사용시 많이 감소
                else if (consecutiveCount >= 1)
                    weight *= 0.6f;  // 2번 연속 사용시 가중치 감소
            }

            readyPatterns.push_back({ idx, weight });
        }
    }

    // 패턴 가중치 기반 랜덤으로
    _int chosenIdx = -1;
    if (!readyPatterns.empty())
    {
        // 가중치 기반 랜덤 선택
        _float totalWeight = 0.f;
        for (auto& pattern : readyPatterns)
        {
            totalWeight += pattern.second;
        }

        _float randomValue = static_cast<_float>(rand()) / RAND_MAX * totalWeight;
        _float currentWeight = 0.f;

        for (auto& pattern : readyPatterns)
        {
            currentWeight += pattern.second;
            if (randomValue <= currentWeight)
            {
                chosenIdx = pattern.first;
                break;
            }
        }

        // 연속 사용 카운트 업데이트
        if (chosenIdx == lastPatternIndex)
        {
            consecutiveCount++;
        }
        else
        {
            consecutiveCount = 0;
            lastPatternIndex = chosenIdx;
        }
    }
    else
    {
        // 쿨타임이 완료된 패턴이 없다면 다시 대기
        m_fTimeElapsed = 0.f;
        return;
    }

    // 패턴 실행 및 쿨타임 설정
    switch (chosenIdx)
    {
    case 0: // BossPunch
        pChar->ChangeState(new BossPunch(TEXT("BossPunchAttack")));
        pChar->m_CD_Punch = 2.f;
        break;
    case 1: // BossHandAttack
        pChar->ChangeState(new BossHandAttack(TEXT("BossHandAttack")));
        pChar->m_CD_Hand = 3.f;
        break;
    case 2: // BossFreezeAttack
        pChar->ChangeState(new BossFreezeAttack(TEXT("BossFreezeAttack")));
        pChar->m_CD_Freeze = 4.f;
        break;
    case 3: // BossSwingAttack
        pChar->ChangeState(new BossSwingAttack(TEXT("BossSwingAttack")));
        pChar->m_CD_Swing = 5.f;
        break;
    case 4: // BossFollowPunch
        pChar->ChangeState(new BossFollowPunch(TEXT("BossFollowPunchAttack")));
        pChar->m_CD_FollowPunch = 3.f;
        break;
    case 5: // BossTentacle
        pChar->ChangeState(new BossTentacle(TEXT("BossTentacleAttack")));
        pChar->m_CD_Tentacle = 4.f;
        break;
    }
    m_fTimeElapsed = 0.f;
}
//void BossIdle::Update(CEnmuMeat* pChar, _float fTimeDelta)
//{
//    static _int lastHpThreshold = -1;
//    _float fHp = pChar->GetHp();
//    _float fMaxHp = pChar->GetMaxHp();
//    _float fHpRatio = fHp / fMaxHp;
//
//    cout << "[BossIdle] Current HP: " << pChar->GetHp() << endl;
//
//    // HP 체크 로직 (기존 유지)
//    _int currentHpThreshold = static_cast<_int>(fHp) / 150;
//    if (lastHpThreshold == -1)
//    {
//        lastHpThreshold = currentHpThreshold;
//    }
//    else if (currentHpThreshold < lastHpThreshold && fHp > 0 && fHp <= 400.f)
//    {
//        pChar->ChangeState(new BossOpen(TEXT("BossOpen")));
//        lastHpThreshold = currentHpThreshold;
//        return;
//    }
//
//    // 쿨타임 감소
//    auto DecayCD = [&](_float& cd)
//        {
//            if (cd > 0.f)
//            {
//                cd -= fTimeDelta;
//                if (cd < 0.f)
//                    cd = 0.f;
//            }
//        };
//
//    DecayCD(pChar->m_CD_Punch);
//    DecayCD(pChar->m_CD_Swing);
//    DecayCD(pChar->m_CD_Hand);
//    DecayCD(pChar->m_CD_Freeze);
//    DecayCD(pChar->m_CD_FollowPunch);
//    DecayCD(pChar->m_CD_Open);
//    DecayCD(pChar->m_CD_Tentacle);
//
//    // Idle 시간 증가 (0.5초로 단축)
//    m_fTimeElapsed += fTimeDelta;
//    if (m_fTimeElapsed < 0.5f)  // 2초 -> 0.5초로 단축
//        return;
//
//    _float fDist = pChar->GetDistanceToTarget();
//    cout << "[BossIdle] Distance to target: " << fDist << endl;
//
//    // 거리별 사용 가능한 패턴 찾기
//    vector<int> availablePatterns;
//
//    // 거리 조건만 체크 (패턴 사용 여부는 체크하지 않음)
//    if (fDist >= -50.f && fDist <= 100.f)
//        availablePatterns.push_back(0); // BossPunch
//    if (fDist > 100.f && fDist <= 130.f)
//        availablePatterns.push_back(1); // BossHandAttack
//    if (fDist > 130.f && fDist <= 160.f)
//        availablePatterns.push_back(2); // BossFreezeAttack
//    if (fDist > 160.f && fDist <= 190.f)
//        availablePatterns.push_back(3); // BossSwingAttack
//    if (fDist > 190.f && fDist <= 220.f)
//        availablePatterns.push_back(4); // BossFollowPunch
//    if (fDist > 220.f)
//        availablePatterns.push_back(5); // BossTentacle
//
//    // 사용 가능한 패턴이 없으면 대기
//    if (availablePatterns.empty())
//    {
//        m_fTimeElapsed = 0.f;
//        return;
//    }
//
//    // 쿨타임이 완료된 패턴 찾기
//    vector<_int> readyPatterns;
//    for (_int idx : availablePatterns)
//    {
//        _bool isReady = false;
//        switch (idx)
//        {
//        case 0: // BossPunch
//            if (pChar->m_CD_Punch == 0.f) isReady = true;
//            break;
//        case 1: // BossHandAttack
//            if (pChar->m_CD_Hand == 0.f) isReady = true;
//            break;
//        case 2: // BossFreezeAttack
//            if (pChar->m_CD_Freeze == 0.f) isReady = true;
//            break;
//        case 3: // BossSwingAttack
//            if (pChar->m_CD_Swing == 0.f) isReady = true;
//            break;
//        case 4: // BossFollowPunch
//            if (pChar->m_CD_FollowPunch == 0.f) isReady = true;
//            break;
//        case 5: // BossTentacle
//            if (pChar->m_CD_Tentacle == 0.f) isReady = true;
//            break;
//        }
//
//        if (isReady)
//        {
//            readyPatterns.push_back(idx);
//        }
//    }
//
//    // 실행할 패턴 선택
//    int chosenIdx = -1;
//    if (!readyPatterns.empty())
//    {
//        // 쿨타임이 완료된 패턴 중에서 선택 (랜덤하게 선택하거나 순서대로)
//        chosenIdx = readyPatterns[0];
//    }
//    else
//    {
//        // 쿨타임이 완료된 패턴이 없으면 대기
//        m_fTimeElapsed = 0.f;
//        return;
//    }
//
//    // 패턴 실행 및 쿨타임 설정
//    switch (chosenIdx)
//    {
//    case 0: // BossPunch
//        pChar->ChangeState(new BossPunch(TEXT("BossPunchAttack")));
//        pChar->m_CD_Punch = 2.f;   // 쿨타임 설정
//        break;
//    case 1: // BossHandAttack
//        pChar->ChangeState(new BossHandAttack(TEXT("BossHandAttack")));
//        pChar->m_CD_Hand = 3.f;
//        break;
//    case 2: // BossFreezeAttack
//        pChar->ChangeState(new BossFreezeAttack(TEXT("BossFreezeAttack")));
//        pChar->m_CD_Freeze = 4.f;
//        break;
//    case 3: // BossSwingAttack
//        pChar->ChangeState(new BossSwingAttack(TEXT("BossSwingAttack")));
//        pChar->m_CD_Swing = 5.f;
//        break;
//    case 4: // BossFollowPunch
//        pChar->ChangeState(new BossFollowPunch(TEXT("BossFollowPunchAttack")));
//        pChar->m_CD_FollowPunch = 3.f;
//        break;
//    case 5: // BossTentacle
//        pChar->ChangeState(new BossTentacle(TEXT("BossTentacleAttack")));
//        pChar->m_CD_Tentacle = 4.f;
//        break;
//    }
//
//    cout << "[BossIdle] Chosen Pattern Index: " << chosenIdx << endl;
//    m_fTimeElapsed = 0.f;
//}
//void BossIdle::Update(CEnmuMeat* pChar, _float fTimeDelta)
//{
//    static _int lastHpThreshold = -1; // 마지막으로 체크한 HP 임계값
//    _float fHp = pChar->GetHp();
//    _float fMaxHp = pChar->GetMaxHp();
//    _float fHpRatio = fHp / fMaxHp;
//
//
//    cout << "[BossIdle] Current HP: " << pChar->GetHp() << endl;
//
//    // 현재 HP가 속한 100단위 구간 계산
//    _int currentHpThreshold = static_cast<_int>(fHp) / 300;
//
//    // HP가 감소하여 새로운 100단위 구간에 진입했을 때
//    if (lastHpThreshold == -1)
//    {
//        // 첫 번째 업데이트에서 초기값 설정
//        lastHpThreshold = currentHpThreshold;
//    }
//    else if (currentHpThreshold < lastHpThreshold && fHp > 0&&fHp<=400.f)
//    {
//        // HP가 50씩 감소했을 때 BossOpen 상태로 변경
//        pChar->ChangeState(new BossOpen(TEXT("BossOpen")));
//        lastHpThreshold = currentHpThreshold;
//        return;
//    }
//
//    auto DecayCD = [&](_float& cd)
//        {
//            if (cd > 0.f)
//            {
//                cd -= fTimeDelta;
//                if (cd < 0.f)
//                    cd = 0.f;
//            }
//        };
//
//    DecayCD(pChar->m_CD_Punch);
//    DecayCD(pChar->m_CD_Swing);
//    DecayCD(pChar->m_CD_Hand);
//    DecayCD(pChar->m_CD_Freeze);
//    DecayCD(pChar->m_CD_FollowPunch);
//    DecayCD(pChar->m_CD_Open);
//    DecayCD(pChar->m_CD_Tentacle);
//
//    ////  Idle 누적 시간 증가
//    m_fTimeElapsed += fTimeDelta;
//    if (m_fTimeElapsed < 2.f)
//        return;
//
//    _bool allUsed = true;
//    for (_int i = 0; i < 6; ++i)
//    {
//        if (!m_bPatternUsed[i])
//        { 
//            allUsed = false; 
//            break; 
//        }
//    }
//    if (allUsed)
//    {
//        for (_int i = 0; i < 6; ++i)
//            m_bPatternUsed[i] = false;
//    }
//
//    //  플레이어와의 거리 계산
//    _float fDist = pChar->GetDistanceToTarget();
//
//    cout << "[BossIdle] Distance to target: " << fDist
//        << ", LastPatternIdx: " << pChar->m_LastPatternIdx
//        << endl;
//    if (pChar->GetTarget())
//    {
//        _vector vPos = pChar->GetTarget()->GetTransform()->Get_State(STATE::POSITION);
//        _float3 pos = { XMVectorGetX(vPos), XMVectorGetY(vPos), XMVectorGetZ(vPos) };
//        cout << "Player Position: ("
//            << pos.x << ", " << pos.y << ", " << pos.z << ")" << endl;
//    }
//	
// //   // 4) 순환 검사용 인덱스 시작점
// //   //    (마지막에 실행했던 인덱스 + 1) % 7부터 검사
//    int startIdx = (pChar->m_LastPatternIdx + 1) % 6;
//    int chosenIdx = -1;
//
// //   // 6개 패턴을 순차로 검사 (거리 구간별로 분리)
// //   //    0: BossPunch,      1: BossHandAttack,  2: BossFreeze,
// //   //    3: BossSwingAttack, 4: BossFollowPunch, 5: BossTentacle
// //   for (int offset = 0; offset < 6; ++offset)
// //   {
// //       int idx = (startIdx + offset) % 6;
// //       if (m_bPatternUsed[idx])
// //           continue;
//
// //       switch (idx)
// //       {
// //       case 0: // BossPunch      (50 ~ 100)
// //           if (pChar->m_CD_Punch == 0.f && fDist >= 40.f && fDist <= 90.f)
// //               chosenIdx = 0;
// //           break;
//
// //       case 1: // BossHandAttack (101 ~ 130)
// //           if (pChar->m_CD_Hand == 0.f && fDist > 90.f && fDist <= 120.f)
// //               chosenIdx = 1;
// //           break;
//
// //       case 2: // BossFreezeAttack (131 ~ 160)
// //           if (pChar->m_CD_Freeze == 0.f && fDist > 120.f && fDist <= 150.f)
// //               chosenIdx = 2;
// //           break;
//
// //       case 3: // BossSwingAttack  (161 ~ 190)
// //           if (pChar->m_CD_Swing == 0.f && fDist > 150.f && fDist <= 180.f)
// //               chosenIdx = 3;
// //           break;
//
// //       case 4: // BossFollowPunch  (191 ~ 220)
// //           if (pChar->m_CD_FollowPunch == 0.f && fDist > 180.f && fDist <= 220.f)
// //               chosenIdx = 4;
// //           break;
//
// //       case 5: // BossTentacle     (221+)
// //           if (pChar->m_CD_Tentacle == 0.f && fDist > 220.f)
// //               chosenIdx = 5;
// //           break;
// //       }
//
// //       if (chosenIdx >= 0)
// //           break; // 첫 번째 조건 만족 패턴 선택
// //   }
//
// //   // 6) 실행할 패턴이 하나도 없다면, Idle 시간만 초기화하고 종료
// //   if (chosenIdx < 0)
// //   {
// //       m_fTimeElapsed = 0.f;
// //       return;
// //   }
//
// //   ////// 테스트용
// //   //chosenIdx = 6;
// // 
// //   // 7) chosenIdx에 따라 상태 전이 및 쿨타임 재설정
// //   switch (chosenIdx)
// //   {
// //   case 0: // BossPunch
// //       pChar->ChangeState(new BossPunch(TEXT("BossPunchAttack")));
// //       pChar->m_CD_Punch = 5.f;   // 원하는 쿨타임 설정
// //       break;
//
// //   case 1: // BossHandAttack
// //       pChar->ChangeState(new BossHandAttack(TEXT("BossHandAttack")));
// //       pChar->m_CD_Hand = 4.f;
// //       break;
//
// //   case 2: // BossFreezeAttack
// //       pChar->ChangeState(new BossFreezeAttack(TEXT("BossFreezeAttack")));
// //       pChar->m_CD_Freeze = 9.f;
// //       break;
//
// //   case 3: // BossSwingAttack
// //       pChar->ChangeState(new BossSwingAttack(TEXT("BossSwingAttack")));
// //       pChar->m_CD_Swing = 12.f;
// //       break;
//
// //   case 4: // BossFollowPunch
// //       pChar->ChangeState(new BossFollowPunch(TEXT("BossFollowPunchAttack")));
// //       pChar->m_CD_FollowPunch = 6.f;
// //       break;
//
// //   case 5: // BossTentacle
// //       pChar->ChangeState(new BossTentacle(TEXT("BossTentacleAttack")));
// //       pChar->m_CD_Tentacle = 9.f;
// //       break;
//	////case 6: // BossOpen
//	////	pChar->ChangeState(new BossOpen(TEXT("BossOpen")));
//	//////	pChar->m_CD_Open = 10.f;
//	////	break;
// //   }
//
// //   m_bPatternUsed[chosenIdx] = true;
// //   pChar->m_LastPatternIdx = chosenIdx;
//
// //   std::cout << "[BossIdle] Chosen Pattern Index: " << chosenIdx << std::endl;
//
// //   // 9) Idle 누적 시간 리셋
// //   m_fTimeElapsed = 0.f;
//
//vector<int> availablePatterns;
//for (int offset = 0; offset < 6; ++offset)
//{
//    int idx = (startIdx + offset) % 6;
//    if (m_bPatternUsed[idx])
//        continue;
//
//    bool canUse = false;
//    switch (idx)
//    {
//    case 0: // BossPunch      (0 ~ 100)
//        if (fDist >= -50.f && fDist <= 100.f)
//            canUse = true;
//        break;
//    case 1: // BossHandAttack (101 ~ 130)
//        if (fDist > 100.f && fDist <= 130.f)
//            canUse = true;
//        break;
//    case 2: // BossFreezeAttack (131 ~ 160)
//        if (fDist > 130.f && fDist <= 160.f)
//            canUse = true;
//        break;
//    case 3: // BossSwingAttack  (161 ~ 190)
//        if (fDist > 160.f && fDist <= 190.f)
//            canUse = true;
//        break;
//    case 4: // BossFollowPunch  (191 ~ 220) // 거리 구간 수정
//        if (fDist > 190.f && fDist <= 220.f)
//            canUse = true;
//        break;
//    case 5: // BossTentacle     (221+)
//        if (fDist > 220.f)
//            canUse = true;
//        break;
//    }
//
//    if (canUse)
//    {
//        availablePatterns.push_back(idx);
//    }
//}
//
//// 사용 가능한 패턴이 없으면 Idle 상태 유지
//if (availablePatterns.empty()) 
//{
//    m_fTimeElapsed = 0.f;
//    return;
//}
//
//
//vector<_int> readyPatterns; // 쿨타임이 완료된 패턴들
//for (_int idx : availablePatterns) 
//{
//    _bool isReady = false;
//    switch (idx) 
//    {
//    case 0: // BossPunch
//        if (pChar->m_CD_Punch == 0.f) isReady = true;
//        break;
//    case 1: // BossHandAttack
//        if (pChar->m_CD_Hand == 0.f) isReady = true;
//        break;
//    case 2: // BossFreezeAttack
//        if (pChar->m_CD_Freeze == 0.f) isReady = true;
//        break;
//    case 3: // BossSwingAttack
//        if (pChar->m_CD_Swing == 0.f) isReady = true;
//        break;
//    case 4: // BossFollowPunch
//        if (pChar->m_CD_FollowPunch == 0.f) isReady = true;
//        break;
//    case 5: // BossTentacle
//        if (pChar->m_CD_Tentacle == 0.f) isReady = true;
//        break;
//    }
//
//    if (isReady) 
//    {
//        readyPatterns.push_back(idx);
//    }
//}
//
//// 쿨타임이 완료된 패턴이 있으면 그 중에서 선택, 없으면 사용 가능한 패턴 중에서 선택
//if (!readyPatterns.empty())
//{
//    chosenIdx = readyPatterns[0]; // 첫 번째로 찾은 쿨타임 완료 패턴 선택
//}
//else 
//{
//    chosenIdx = availablePatterns[0]; // 쿨타임 상관없이 첫 번째 사용 가능한 패턴 선택
//}
//
//// 실행할 패턴이 없다면 Idle 시간만 초기화하고 종료
//if (chosenIdx < 0)
//{
//    m_fTimeElapsed = 0.f;
//    return;
//}
//
//////// 테스트용///////
////chosenIdx = 3;
//
//// chosenIdx에 따라 상태 전이 및 쿨타임 재설정
//switch (chosenIdx)
//{
//case 0: // BossPunch
//    pChar->ChangeState(new BossPunch(TEXT("BossPunchAttack")));
////pChar->m_CD_Punch = 4.f;   // 원하는 쿨타임 설정
//    break;
//case 1: // BossHandAttack
//    pChar->ChangeState(new BossHandAttack(TEXT("BossHandAttack")));
//  //  pChar->m_CD_Hand = 8.f;
//    break;
//case 2: // BossFreezeAttack
//    pChar->ChangeState(new BossFreezeAttack(TEXT("BossFreezeAttack")));
//  //  pChar->m_CD_Freeze = 5.f;
//    break;
//case 3: // BossSwingAttack
//    pChar->ChangeState(new BossSwingAttack(TEXT("BossSwingAttack")));
//   // pChar->m_CD_Swing = 9.f;
//    break;
//case 4: // BossFollowPunch
//    pChar->ChangeState(new BossFollowPunch(TEXT("BossFollowPunchAttack")));
//   // pChar->m_CD_FollowPunch = 6.f;
//    break;
//case 5: // BossTentacle
//    pChar->ChangeState(new BossTentacle(TEXT("BossTentacleAttack")));
//   // pChar->m_CD_Tentacle = 7.f;
//    break;
//}
//
//m_bPatternUsed[chosenIdx] = true;
//pChar->m_LastPatternIdx = chosenIdx;
//cout << "[BossIdle] Chosen Pattern Index: " << chosenIdx << endl;
//
//    // Idle 누적 시간 리셋
//    m_fTimeElapsed = 0.f;
//}

void BossIdle::Exit(CEnmuMeat* pChar)
{
}
