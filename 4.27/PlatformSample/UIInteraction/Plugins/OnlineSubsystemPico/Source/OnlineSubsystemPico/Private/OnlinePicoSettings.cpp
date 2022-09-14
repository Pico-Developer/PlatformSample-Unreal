// Copyright 2022 Pico Technology Co., Ltd.All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc.In the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2022, Epic Games, Inc.All rights reserved.

#include "OnlinePicoSettings.h"

DEFINE_LOG_CATEGORY(PicoSettings);

UOnlinePicoSettings::UOnlinePicoSettings(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer),
    bIsForeign(false),
    bIsGlobal(false),
    bStartTimeEntitlementCheck(false),
    bEntitlementCheckSimulation(false)
{
    DefaultPlatformService = TEXT("Pico");
    AndroidPlatformService = TEXT("Pico");
}

void UOnlinePicoSettings::PostInitProperties()
{
    Super::PostInitProperties();

    switch (RegionType)
    {
    case ERegionType::China:
    {
        bIsForeign = false;
        bIsGlobal = false;
    }
    break;
    case ERegionType::NonChina:
    {
        bIsForeign = true;
        bIsGlobal = false;
    }
    break;
    case ERegionType::Both:
    {
        bIsForeign = false;
        bIsGlobal = true;
    }
    break;
    default:
        break;
    }

    Super::UpdateSinglePropertyInConfigFile(GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UOnlinePicoSettings, bIsForeign)), GetDefaultConfigFilename());
    Super::UpdateSinglePropertyInConfigFile(GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UOnlinePicoSettings, bIsGlobal)), GetDefaultConfigFilename());
}

void UOnlinePicoSettings::GetOnlinePicoSettings(bool& OutbIsEnable, ERegionType& OutRegionType, FString& OutAppID, FString& OutAppKey, FString& OutScope, bool& OutbStartTimeEntitlementCheck, FString& OutEntitlementCheckAppID, bool& OutbEntitlementCheckSimulation, TArray<FString>& OutDeviceSN)
{
    UOnlinePicoSettings* OnlinePicoSettings = GetMutableDefault<UOnlinePicoSettings>();
    OutbIsEnable = OnlinePicoSettings->bEnabled;
    OutRegionType = OnlinePicoSettings->RegionType;
    OutAppID = OnlinePicoSettings->AppID;
    OutAppKey = OnlinePicoSettings->AppKey;
    OutScope = OnlinePicoSettings->Scope;
    OutbStartTimeEntitlementCheck = OnlinePicoSettings->bStartTimeEntitlementCheck;
    OutEntitlementCheckAppID = OnlinePicoSettings->EntitlementCheckAppID;
    OutbEntitlementCheckSimulation = OnlinePicoSettings->bEntitlementCheckSimulation;
    OutDeviceSN = OnlinePicoSettings->DeviceSN;
}

void UOnlinePicoSettings::LoadSettings()
{
    // Get values from config files.
    GConfig->GetBool(TEXT("OnlineSubsystemPico"), TEXT("bEnabled"), bEnabled, FPaths::ProjectDir() + "Config/DefaultEngine.ini");
    FString GetRegionType;
    GConfig->GetString(TEXT("/Script/OnlineSubsystemPico.OnlinePicoSettings"), TEXT("RegionType"), GetRegionType, FPaths::ProjectDir() + "Config/DefaultEngine.ini");
    if (GetRegionType == TEXT("China"))
    {
        RegionType = ERegionType::China;
    }
    else if (GetRegionType == TEXT("NonChina"))
    {
        RegionType = ERegionType::NonChina;
    }
    else
    {
        RegionType = ERegionType::Both;
    }
    GConfig->GetString(TEXT("/Script/OnlineSubsystemPico.OnlinePicoSettings"), TEXT("AppID"), AppID, FPaths::ProjectDir() + "Config/DefaultEngine.ini");
    GConfig->GetString(TEXT("/Script/OnlineSubsystemPico.OnlinePicoSettings"), TEXT("AppKey"), AppKey, FPaths::ProjectDir() + "Config/DefaultEngine.ini");
    GConfig->GetString(TEXT("/Script/OnlineSubsystemPico.OnlinePicoSettings"), TEXT("Scope"), Scope, FPaths::ProjectDir() + "Config/DefaultEngine.ini");
    GConfig->GetBool(TEXT("/Script/OnlineSubsystemPico.OnlinePicoSettings"), TEXT("bStartTimeEntitlementCheck"), bStartTimeEntitlementCheck, FPaths::ProjectDir() + "Config/DefaultEngine.ini");
    GConfig->GetString(TEXT("/Script/OnlineSubsystemPico.OnlinePicoSettings"), TEXT("EntitlementCheckAppID"), EntitlementCheckAppID, FPaths::ProjectDir() + "Config/DefaultEngine.ini");
    GConfig->GetBool(TEXT("/Script/OnlineSubsystemPico.OnlinePicoSettings"), TEXT("bEntitlementCheckSimulation"), bEntitlementCheckSimulation, FPaths::ProjectDir() + "Config/DefaultEngine.ini");
    GConfig->GetArray(TEXT("/Script/OnlineSubsystemPico.OnlinePicoSettings"), TEXT("bEntitlementCheckSimulation"), DeviceSN, FPaths::ProjectDir() + "Config/DefaultEngine.ini");
    GConfig->GetString(TEXT("OnlineSubsystem"), TEXT("DefaultPlatformService"), AndroidPlatformService, FPaths::ProjectDir() + "Config/Android/AndroidEngine.ini");
#if WITH_EDITOR
    bool bShouldForceSaveSettings = ValidateSettings();

    UE_LOG(PicoSettings, Log, TEXT("DefaultEngine.ini->bEnabled = %s"), bEnabled ? TEXT("True") : TEXT("False"));
    UE_LOG(PicoSettings, Log, TEXT("DefaultEngine.ini->AppId = %s"), *AppID);
    UE_LOG(PicoSettings, Log, TEXT("DefaultEngine.ini->AppKey = %s"), *AppKey);
    UE_LOG(PicoSettings, Log, TEXT("DefaultEngine.ini->Scope = %s"), *Scope);
    UE_LOG(PicoSettings, Log, TEXT("DefaultEngine.ini->DefaultPlatformService = %s"), *DefaultPlatformService);
    UE_LOG(PicoSettings, Log, TEXT("AndroidEngine.ini->DefaultPlatformService = %s"), *AndroidPlatformService);
#endif //WITH_EDITOR
}

#if WITH_EDITOR
void UOnlinePicoSettings::SaveSettings()
{
    // Copy values to config files, using DefaultPlatformService value for both, to ensure they're always the same.
    GConfig->SetBool(TEXT("OnlineSubsystemPico"), TEXT("bEnabled"), bEnabled, FPaths::ProjectDir() + "Config/DefaultEngine.ini");
    if (bEnabled)
    {
        GConfig->SetString(TEXT("OnlineSubsystem"), TEXT("DefaultPlatformService"), *DefaultPlatformService, FPaths::ProjectDir() + "Config/DefaultEngine.ini");
        GConfig->SetString(TEXT("OnlineSubsystem"), TEXT("DefaultPlatformService"), *AndroidPlatformService, FPaths::ProjectDir() + "Config/Android/AndroidEngine.ini");
    }


    GConfig->Flush(false, FPaths::ProjectDir() + "Config/DefaultEngine.ini");
    GConfig->Flush(false, FPaths::ProjectDir() + "Config/Android/AndroidEngine.ini");

    UE_LOG(PicoSettings, Log, TEXT("Finished saving values to config files."));
}

bool UOnlinePicoSettings::ValidateSettings()
{
    if (bEnabled)
    {
        //if (DefaultPlatformService != "Pico")
        //{
        //    UE_LOG(PicoSettings, Warning, TEXT("OnlineSubsystemPico is enabled, but DefaultPlatformService is NOT set to Pico."));
        //}
        //if (AndroidPlatformService != "Pico")
        //{
        //    AndroidPlatformService = DefaultPlatformService;
        //    UE_LOG(PicoSettings, Warning, TEXT("OnlineSubsystemPico is enabled, but DefaultPlatformService is NOT set to Pico in AndroidEngine.ini ."));
        //}
        return true;
    }
    return false;
}

void UOnlinePicoSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    GConfig->Flush(true, FPaths::ProjectDir() + "Config/DefaultEngine.ini");
    GConfig->Flush(true, FPaths::ProjectDir() + "Config/Android/AndroidEngine.ini");

    // Save updated settings to config files
    SaveSettings();

    Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif