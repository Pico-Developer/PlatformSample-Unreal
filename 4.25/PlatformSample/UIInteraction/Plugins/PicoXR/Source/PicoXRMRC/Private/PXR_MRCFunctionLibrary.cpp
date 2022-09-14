//Unreal® Engine, Copyright 1998 – 2022, Epic Games, Inc. All rights reserved.


#include "PXR_MRCFunctionLibrary.h"
#include "PXR_MRCModule.h"

bool UPicoXRMRCFunctionLibrary::IsMrcActivated()
{
	if (FPicoXRMRCModule::IsAvailable())
	{
		return FPicoXRMRCModule::Get().IsMrcActivated();
	}
	return false;
}

bool UPicoXRMRCFunctionLibrary::GetInGameThirdCameraRT(UTextureRenderTarget2D* & Background_RT, UTextureRenderTarget2D*& Forgound_RT)
{
	if (FPicoXRMRCModule::IsAvailable())
	{
		return FPicoXRMRCModule::Get().GetMRCRT(Background_RT,Forgound_RT);
	}
	return false;
}

void UPicoXRMRCFunctionLibrary::SimulateEnableMRC(bool enable)
{
	if (FPicoXRMRCModule::IsAvailable())
	{
		FPicoXRMRCModule::Get().bSimulateEnableMRC=enable;
	}
}

void UPicoXRMRCFunctionLibrary::EnableForegroundMRC(bool enable)
{
	if (FPicoXRMRCModule::IsAvailable())
	{
		FPicoXRMRCModule::Get().EnableForeground(enable);
	}
}

void UPicoXRMRCFunctionLibrary::SetMRCTrackingReference(USceneComponent* TrackingReference)
{
	if (FPicoXRMRCModule::IsAvailable())
	{
		if (FPicoXRMRCModule::Get().GetMRCState())
		{
			FPicoXRMRCModule::Get().GetMRCState()->CurrentTrackingReference = TrackingReference;
		}
	}
}

void UPicoXRMRCFunctionLibrary::SetMRCUseCustomTrans(const FTransform& CustomTrans, bool UseCutomTrans)
{
	if (FPicoXRMRCModule::IsAvailable())
	{
		if (FPicoXRMRCModule::Get().GetMRCState())
		{
			FPicoXRMRCModule::Get().GetMRCState()->bUseCustomTrans = UseCutomTrans;
			FPicoXRMRCModule::Get().GetMRCState()->CustomTrans = CustomTrans;
		}
	}
}

void UPicoXRMRCFunctionLibrary::GetMRCRelativePose(FTransform& CustomTrans)
{
	if (FPicoXRMRCModule::IsAvailable())
	{
		if (FPicoXRMRCModule::Get().GetMRCState())
		{
			CustomTrans = FPicoXRMRCModule::Get().GetMRCState()->FinalTransform;
		}
	}
}
