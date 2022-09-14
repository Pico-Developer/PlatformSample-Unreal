//Unreal® Engine, Copyright 1998 – 2022, Epic Games, Inc. All rights reserved.

#pragma once
#include "CoreMinimal.h"
#include "RHIResources.h"
#include "Engine/Texture2D.h"
#include "UObject/SoftObjectPath.h"
#include "PXR_HMDTypes.generated.h"

USTRUCT()
struct FPXRSplashDesc
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(config, EditAnywhere, Category = Settings, meta = (AllowedClasses = "Texture",ToolTip = "Splash texture to show"))
	FSoftObjectPath		SplashTexturePath;

	UPROPERTY(config, EditAnywhere, Category = Settings, meta = (ToolTip = "The center transform in meters"))
	FTransform			SplashTransformInMeters;

	UPROPERTY(config, EditAnywhere, Category = Settings, meta = (ToolTip = "The splash layer quad size in meters"))
	FVector2D			SplashQuadSizeInMeters;

	UPROPERTY(config, EditAnywhere, Category = Settings, meta = (ToolTip = "Splash texture offset relative to the upper left corner"))
	FVector2D			SplashTextureOffset;

	UPROPERTY(config, EditAnywhere, Category = Settings, meta = (ToolTip = "Splash texture scale (default=1.0f)"))
	FVector2D			SplashTextureScale;

	UPROPERTY(config, EditAnywhere, Category = Settings, meta = (ToolTip = "Ignore the splash texture alpha channel"))
	bool				bNoAlpha;

	UTexture*			LoadingTextureFromPath;
	FTextureRHIRef		LoadedTextureRef;
	bool				bIsLiveUpdate;

	FPXRSplashDesc()
		: SplashTransformInMeters(FVector(8.0f, 0.f, 0.f))
		, SplashQuadSizeInMeters(3.f, 3.f)
		, SplashTextureOffset(0.0f, 0.0f)
		, SplashTextureScale(1.0f, 1.0f)
		, bNoAlpha(true)
		, LoadingTextureFromPath(nullptr)
		, LoadedTextureRef(nullptr)
		, bIsLiveUpdate(false)
	{
	}
};