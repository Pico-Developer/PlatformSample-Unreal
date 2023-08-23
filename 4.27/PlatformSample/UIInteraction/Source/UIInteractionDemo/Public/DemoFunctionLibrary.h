// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

// include test
//#include "PicoPresenceInterface.h"
//#include "ApplicationLifecycleInterface.h"
//#include "OnlineMessageTaskManagerPico.h"
//#include "OnlineSessionInterfacePico.h"
//#include "Pico_Achievements.h"
//#include "Pico_AssetFile.h"
//#include "Pico_Challenges.h"
//#include "Pico_DataStore.h"
//#include "Pico_IAP.h"
//#include "Pico_Leaderboard.h"
//#include "Pico_Leaderboards.h"
//#include "Pico_Room.h"
//#include "Pico_Sport.h"
//#include "Pico_User.h"
//#include "PicoApplicationInterface.h"
//#include "PicoPresenceInterface.h"
//#include "RTCPicoUserInterface.h"
//#include "OnlineSubsystemPico.h"
//#include "OnlinePicoFunctionLibrary.h"

#include "Runtime/Online//HTTP/Public/Http.h"
#include "DemoFunctionLibrary.generated.h"


DECLARE_DYNAMIC_DELEGATE_OneParam(FGetStringFromHttpDelegate, const FString&, code);
/**
 * 
 */
UCLASS()
class UIINTERACTIONDEMO_API UDemoFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
    static FGetStringFromHttpDelegate StringHttpDelegate;

    UFUNCTION(BlueprintCallable)
    static void RequestGetHttpString(const FString& InHttpAdr, FGetStringFromHttpDelegate OnStringHttpDelegateCallback);

    static void OnRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    UFUNCTION(BlueprintCallable)
    static UTexture2D* LoadTexture2D(const FString& FilePath, bool& IsValid, int32& OutWidth, int32& OutHeight);
};
