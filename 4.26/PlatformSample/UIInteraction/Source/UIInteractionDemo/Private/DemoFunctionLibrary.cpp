// Fill out your copyright notice in the Description page of Project Settings.


#include "DemoFunctionLibrary.h"


FGetStringFromHttpDelegate UDemoFunctionLibrary::StringHttpDelegate;

void UDemoFunctionLibrary::RequestGetHttpString(const FString& InHttpAdr, FGetStringFromHttpDelegate OnStringHttpDelegateCallback)
{
    UDemoFunctionLibrary::StringHttpDelegate = OnStringHttpDelegateCallback;
    FHttpModule* Http = &FHttpModule::Get();
#if ENGINE_MAJOR_VERSION > 4
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
#elif ENGINE_MINOR_VERSION > 25
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
#elif ENGINE_MINOR_VERSION > 24
    TSharedRef<IHttpRequest> Request = Http->CreateRequest();
#endif
    Request->OnProcessRequestComplete().BindStatic(UDemoFunctionLibrary::OnRequestComplete);
    Request->SetURL(InHttpAdr);
    Request->SetVerb("GET");
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));
    Request->ProcessRequest();
}

void UDemoFunctionLibrary::OnRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        FString GetStr = Response->GetContentAsString();
        GetStr.RemoveAt(0);
        GetStr.RemoveAt(GetStr.Len() - 1);
        UDemoFunctionLibrary::StringHttpDelegate.ExecuteIfBound(GetStr);
    }
    else
    {
        FString GetStr = "";
        UDemoFunctionLibrary::StringHttpDelegate.ExecuteIfBound(GetStr);
    }
}
