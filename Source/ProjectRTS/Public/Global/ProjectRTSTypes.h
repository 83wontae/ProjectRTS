// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

// 시스템별로 분리된 세부 헤더들
#include "Global/RTSCoreTypes.h"		// 기초 및 공용 타입
#include "Global/RTSUnitTypes.h"		// 유닛 성장 및 스탯
#include "Global/RTSItemTypes.h"		// 아이템 및 장비
#include "Global/RTSSkillTypes.h"		// 스킬 및 애니메이션
#include "Global/RTSBuildingTypes.h"	// 건물 및 그리드
#include "Global/RTSSaveTypes.h"		// 저장 및 로드 관련 데이터

// 이 파일에는 더 이상 직접적인 구조체/열거형 정의를 두지 않습니다.