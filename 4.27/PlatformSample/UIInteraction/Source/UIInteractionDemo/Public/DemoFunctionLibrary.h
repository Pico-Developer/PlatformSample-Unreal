// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
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
};
