// Fill out your copyright notice in the Description page of Project Settings.


#include "DemoFunctionLibrary.h"


FGetStringFromHttpDelegate UDemoFunctionLibrary::StringHttpDelegate;

void UDemoFunctionLibrary::RequestGetHttpString(const FString& InHttpAdr, FGetStringFromHttpDelegate OnStringHttpDelegateCallback)
{
    UDemoFunctionLibrary::StringHttpDelegate = OnStringHttpDelegateCallback;
    FHttpModule* Http = &FHttpModule::Get();
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
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
