// Fill out your copyright notice in the Description page of Project Settings.

#include "DemoFunctionLibrary.h"
#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "Misc/FileHelper.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"




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

UTexture2D* UDemoFunctionLibrary::LoadTexture2D(const FString& FilePath, bool& IsValid, int32& OutWidth, int32& OutHeight)
{

    UTexture2D* Texture = nullptr;
    IsValid = false;
    if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
    {
        return nullptr;
    }
    TArray<uint8> CompressedData;
    if (!FFileHelper::LoadFileToArray(CompressedData, *FilePath))
    {
        return nullptr;
    }
    IImageWrapperModule& Module = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
    TSharedPtr<IImageWrapper> ImageWrapper = nullptr;
    if (FilePath.EndsWith(TEXT(".png")))
    {
        ImageWrapper = Module.CreateImageWrapper(EImageFormat::PNG);
    }
    else if (FilePath.EndsWith(TEXT(".jpg")) || FilePath.EndsWith(TEXT(".jpeg")))
    {
        ImageWrapper = Module.CreateImageWrapper(EImageFormat::JPEG);
    }
    
    if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(CompressedData.GetData(), CompressedData.Num()))
    {
        TArray<uint8> UncompressedRGBA;
        if (ImageWrapper->GetRaw(ERGBFormat::RGBA, 8, UncompressedRGBA))
        {
            Texture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_R8G8B8A8);
            if (Texture != nullptr)
            {
                IsValid = true;
                OutWidth = ImageWrapper->GetWidth();
                OutHeight = ImageWrapper->GetHeight();
                void* TextureData = Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
                FMemory::Memcpy(TextureData, UncompressedRGBA.GetData(), UncompressedRGBA.Num());
                Texture->PlatformData->Mips[0].BulkData.Unlock();
                Texture->UpdateResource();
            }
        }
    }
    return Texture;
}
