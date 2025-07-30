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

void BossIdle::Enter(CEnmuMeat* pChar)
{
	pChar->SetState(EnmuState::IDLE);
	m_fTimeElapsed = 0.f; // 시간 초기화
}
void BossIdle::Update(CEnmuMeat* pChar, _float fTimeDelta)
{
    static _int lastHpThreshold = -1;
    static _int lastPatternIndex = -1;  // 마지막 사용한 패턴 기억
    static _int consecutiveCount = 0;   // 연속 사용 횟수

    _float fHp = pChar->GetHp();
    _float fMaxHp = pChar->GetMaxHp();
    _float fHpRatio = fHp / fMaxHp;

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

void BossIdle::Exit(CEnmuMeat* pChar)
{
}
