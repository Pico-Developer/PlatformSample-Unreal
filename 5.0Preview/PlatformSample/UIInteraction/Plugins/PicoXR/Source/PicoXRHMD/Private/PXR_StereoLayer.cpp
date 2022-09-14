//Unreal® Engine, Copyright 1998 – 2022, Epic Games, Inc. All rights reserved.

#include "PXR_StereoLayer.h"
#include "HardwareInfo.h"
#include "IXRTrackingSystem.h"
#include "PXR_HMD.h"
#include "PXR_Utils.h"
#include "GameFramework/PlayerController.h"
#include "PXR_Log.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "XRThreadUtils.h"

#if PLATFORM_ANDROID
#include "OpenGLDrvPrivate.h"
#include "OpenGLResources.h"
#include "VulkanRHIPrivate.h"
#include "VulkanResources.h"
#endif

FPxrLayer::FPxrLayer(uint32 InPxrLayerId) :
	PxrLayerId(InPxrLayerId)
{
}

FPxrLayer::~FPxrLayer()
{
	if (IsInGameThread())
	{
		ExecuteOnRenderThread([this]()
			{
				ExecuteOnRHIThread_DoNotWait([this]()
					{
#if PLATFORM_ANDROID
                        PXR_LOGD(PxrUnreal, "~FPxrLayer()a DestroyLayer %d", PxrLayerId);
						Pxr_DestroyLayer(PxrLayerId);
#endif
					});
			});
	}
	else
	{
		ExecuteOnRHIThread_DoNotWait([this]()
			{
#if PLATFORM_ANDROID
                PXR_LOGD(PxrUnreal, "~FPxrLayer()b DestroyLayer %d", PxrLayerId);
				Pxr_DestroyLayer(PxrLayerId);
#endif
			});
	}
}

uint64_t OverlayImages[2] = {};
uint64_t OverlayNativeImages[2][3] = {};

FPicoXRStereoLayer::FPicoXRStereoLayer(FPicoXRHMD* InHMDDevice, uint32 InPXRLayerId, const IStereoLayers::FLayerDesc& InDesc)
    : bMRCLayer(false)
    , HMDDevice(InHMDDevice)
    , LayerId(InPXRLayerId)
    , bTextureNeedUpdate(false)
    , UnderlayMeshComponent(NULL)
    , UnderlayActor(NULL)
    , PxrLayer(nullptr)
{
    PXR_LOGD(PxrUnreal, "FPicoXRStereoLayer with LayerId=%d", LayerId);

#if PLATFORM_ANDROID
	FMemory::Memzero(PxrLayerCreateParam);
#endif

    SetPXRLayerDesc(InDesc);
}

FPicoXRStereoLayer::FPicoXRStereoLayer(const FPicoXRStereoLayer& InPXRLayer)
    : bSplashLayer(InPXRLayer.bSplashLayer)
    , bMRCLayer(InPXRLayer.bMRCLayer)
    , HMDDevice(InPXRLayer.HMDDevice)
    , LayerId(InPXRLayer.LayerId)
    , LayerDesc(InPXRLayer.LayerDesc)
    , SwapChain(InPXRLayer.SwapChain)
    , LeftSwapChain(InPXRLayer.LeftSwapChain)
    , FoveationSwapChain(InPXRLayer.FoveationSwapChain)
    , bTextureNeedUpdate(InPXRLayer.bTextureNeedUpdate)
    , UnderlayMeshComponent(InPXRLayer.UnderlayMeshComponent)
    , UnderlayActor(InPXRLayer.UnderlayActor)
    , PxrLayer(InPXRLayer.PxrLayer)
{
#if PLATFORM_ANDROID
	FMemory::Memcpy(&PxrLayerCreateParam, &InPXRLayer.PxrLayerCreateParam, sizeof(PxrLayerCreateParam));
#endif
}

FPicoXRStereoLayer::~FPicoXRStereoLayer()
{
}

TSharedPtr<FPicoXRStereoLayer, ESPMode::ThreadSafe> FPicoXRStereoLayer::CloneMyself() const
{
	return MakeShareable(new FPicoXRStereoLayer(*this));
}

void FPicoXRStereoLayer::SetPXRLayerDesc(const IStereoLayers::FLayerDesc& InDesc)
{
	if (LayerDesc.Texture != InDesc.Texture || LayerDesc.LeftTexture != InDesc.LeftTexture)
	{
		bTextureNeedUpdate = true;
	}
	LayerDesc = InDesc;

	ManageUnderlayComponent();
}

void FPicoXRStereoLayer::ManageUnderlayComponent()
{
	if (IsLayerSupportDepth())
	{
		const FString UnderlayNameStr = FString::Printf(TEXT("PicoUnderlay_%d"), LayerId);
		const FName UnderlayComponentName(*UnderlayNameStr);
		if (UnderlayMeshComponent == NULL)
		{
			UWorld* World = NULL;
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
				{
					World = Context.World();
				}
			}
			if (World == NULL)
			{
				return;
			}
			UnderlayActor = World->SpawnActor<AActor>();
			UnderlayMeshComponent = NewObject<UProceduralMeshComponent>(UnderlayActor, UnderlayComponentName);
			UnderlayMeshComponent->RegisterComponent();

			TArray<FVector> VerticePos;
			TArray<int32> TriangleIndics;
			TArray<FVector> Normals;
			TArray<FVector2D> TexUV;
			TArray<FLinearColor> VertexColors;
			TArray<FProcMeshTangent> Tangents;

			CreateQuadUnderlayMesh(VerticePos, TriangleIndics, TexUV);
			UnderlayMeshComponent->CreateMeshSection_LinearColor(0, VerticePos, TriangleIndics, Normals, TexUV, VertexColors, Tangents, false);

			if (HMDDevice && HMDDevice->GetContentResourceFinder())
			{
				UMaterialInstanceDynamic* DynamicMaterialInstance = UMaterialInstanceDynamic::Create(HMDDevice->GetContentResourceFinder()->StereoLayerDepthMat, NULL);
				UnderlayMeshComponent->SetMaterial(0, DynamicMaterialInstance);
			}
		}
		UnderlayMeshComponent->SetWorldTransform(LayerDesc.Transform);
	}
	return;
}

void FPicoXRStereoLayer::CreateQuadUnderlayMesh(TArray<FVector>& VerticePos, TArray<int32>& TriangleIndics, TArray<FVector2D>& TexUV)
{
#if ENGINE_MINOR_VERSION > 24||ENGINE_MAJOR_VERSION>4
	if (LayerDesc.HasShape<FQuadLayer>())
#else
	if (LayerDesc.ShapeType==IStereoLayers::QuadLayer)
#endif
	{
		FIntPoint TextureSize = LayerDesc.Texture.IsValid() ? LayerDesc.Texture->GetTexture2D()->GetSizeXY() : LayerDesc.LayerSize;
		float Aspect = 1.0f;
		if (TextureSize.X > 0)
			Aspect = (float)TextureSize.Y / (float)TextureSize.X;

		float QuadSizeX = LayerDesc.QuadSize.X;
		float QuadSizeY = LayerDesc.QuadSize.Y;
		if (LayerDesc.Flags & IStereoLayers::LAYER_FLAG_QUAD_PRESERVE_TEX_RATIO)
		{
			QuadSizeY = QuadSizeX * Aspect;
		}

		VerticePos.Init(FVector::ZeroVector, 4);
		VerticePos[0] = FVector(0.0, -QuadSizeX / 2, -QuadSizeY / 2);
		VerticePos[1] = FVector(0.0, QuadSizeX / 2, -QuadSizeY / 2);
		VerticePos[2] = FVector(0.0, QuadSizeX / 2, QuadSizeY / 2);
		VerticePos[3] = FVector(0.0, -QuadSizeX / 2, QuadSizeY / 2);

		TexUV.Init(FVector2D::ZeroVector, 4);
		TexUV[0] = FVector2D(1, 0);
		TexUV[1] = FVector2D(1, 1);
		TexUV[2] = FVector2D(0, 0);
		TexUV[3] = FVector2D(0, 1);

		TriangleIndics.Reserve(6);
		TriangleIndics.Add(0);
		TriangleIndics.Add(1);
		TriangleIndics.Add(2);
		TriangleIndics.Add(0);
		TriangleIndics.Add(2);
		TriangleIndics.Add(3);
	}
}

void FPicoXRStereoLayer::PXRLayersCopy_RenderThread(FPicoXRRenderBridge* RenderBridge, FRHICommandListImmediate& RHICmdList)
{
    check(IsInRenderingThread());
    if ((LayerDesc.Flags & IStereoLayers::LAYER_FLAG_HIDDEN)!=0)
    {
        return;
    }
    PXR_LOGV(PxrUnreal, "LayerId=%d, bUpdateTexture=%d, SwapChain_Valid=%d, LayerDesc.Texture.IsValid()=%d", LayerId, bTextureNeedUpdate, SwapChain.IsValid(), LayerDesc.Texture.IsValid());

    if (bTextureNeedUpdate)
    {
        // Copy textures
        if (LayerDesc.Texture.IsValid() && SwapChain.IsValid())
        {
            bool bNoAlpha = (LayerDesc.Flags & IStereoLayers::LAYER_FLAG_TEX_NO_ALPHA_CHANNEL) != 0;

            // Mono
            FRHITexture* SrcTexture = LayerDesc.Texture;
            FRHITexture* DstTexture = SwapChain->GetTexture();
            RenderBridge->TransferImage_RenderThread(RHICmdList, DstTexture, SrcTexture, FIntRect(), FIntRect(), false, bNoAlpha, bMRCLayer);

            // Stereo
            if (LayerDesc.LeftTexture.IsValid() && LeftSwapChain.IsValid())
            {
                FRHITexture* LeftSrcTexture = LayerDesc.LeftTexture;
                FRHITexture* LeftDstTexture = LeftSwapChain->GetTexture();
                RenderBridge->TransferImage_RenderThread(RHICmdList, LeftDstTexture, LeftSrcTexture, FIntRect(), FIntRect(), false, bNoAlpha, bMRCLayer);
            }
			
			bTextureNeedUpdate = false;
        }
    }
}

bool FPicoXRStereoLayer::InitPXRLayer_RenderThread(FPicoXRRenderBridge* CustomPresent, FRHICommandListImmediate& RHICmdList, const FPicoXRStereoLayer* InLayer)
{   
	check(IsInRenderingThread());

	int32 MSAAValue = 1;
	bool bNeedFFRSwapChain = false;
	if (LayerId == 0)
	{
		if (HMDDevice)
		{
			MSAAValue = HMDDevice->GetMSAAValue();
		}

		if (CustomPresent->RHIString == TEXT("Vulkan"))
		{
			bNeedFFRSwapChain = true;
		}
	}
	else
	{
		MSAAValue = 1;
#if PLATFORM_ANDROID
		PxrLayerCreateParam.layerId = LayerId;
		PxrLayerCreateParam.layerShape = static_cast<PxrLayerShape>(GetShapeType());
		PxrLayerCreateParam.layerType = IsLayerSupportDepth() ? PXR_UNDERLAY : PXR_OVERLAY;

		if (LayerDesc.Texture.IsValid())
		{
			FRHITexture2D* Texture2D = LayerDesc.Texture->GetTexture2D();
			if (Texture2D)
			{
				PxrLayerCreateParam.width = Texture2D->GetSizeX();
				PxrLayerCreateParam.height = Texture2D->GetSizeY();
				PxrLayerCreateParam.sampleCount = Texture2D->GetNumSamples();
				PxrLayerCreateParam.mipmapCount = Texture2D->GetNumMips();
			}
		}
		else
		{
			PxrLayerCreateParam.width = LayerDesc.QuadSize.X;
			PxrLayerCreateParam.height = LayerDesc.QuadSize.Y;
			PxrLayerCreateParam.sampleCount = 1;
			PxrLayerCreateParam.mipmapCount = 1;
		}

		if (PxrLayerCreateParam.width == 0 || PxrLayerCreateParam.height == 0)
		{
			return false;
		}

		PxrLayerCreateParam.faceCount = 1;
		PxrLayerCreateParam.arraySize = 1;

		if (!(LayerDesc.Flags & IStereoLayers::LAYER_FLAG_TEX_CONTINUOUS_UPDATE))
		{
			PxrLayerCreateParam.layerFlags |= PXR_LAYER_FLAG_STATIC_IMAGE;
		}

		PxrLayerCreateParam.layerLayout = LayerDesc.LeftTexture.IsValid() ? PXR_LAYER_LAYOUT_STEREO : PXR_LAYER_LAYOUT_MONO;
#endif
	}

#if PLATFORM_ANDROID
	if (CustomPresent->RHIString == TEXT("OpenGL"))
	{
		PxrLayerCreateParam.format = IsMobileColorsRGB() ? GL_SRGB8_ALPHA8 : GL_RGBA8;
	}
	else if (CustomPresent->RHIString == TEXT("Vulkan"))
	{
		PxrLayerCreateParam.format = IsMobileColorsRGB() ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
	}
#endif

	if (IfCanReuseLayers(InLayer))
	{
		bSplashLayer = InLayer->bSplashLayer;
		bMRCLayer = InLayer->bMRCLayer;
		HMDDevice = InLayer->HMDDevice;
		LayerId = InLayer->LayerId;
		LayerDesc = InLayer->LayerDesc;
		SwapChain = InLayer->SwapChain;
		LeftSwapChain = InLayer->LeftSwapChain;
        FoveationSwapChain =InLayer->FoveationSwapChain;
		bTextureNeedUpdate |= InLayer->bTextureNeedUpdate;
		UnderlayMeshComponent = InLayer->UnderlayMeshComponent;
		UnderlayActor = InLayer->UnderlayActor;
		PxrLayer = InLayer->PxrLayer;
	}
    else
	{
		if (InLayer && InLayer->PxrLayer)
		{
			ExecuteOnRHIThread([&]()
				{
					PXR_LOGD(PxrUnreal, "Layer ReCreateLayer_RHIThread=%d", LayerId);
#if PLATFORM_ANDROID
					Pxr_DestroyLayer(LayerId);
#endif
				});
			PxrLayer = InLayer->PxrLayer;
		}

		TArray<uint64> TextureResources;
		TArray<uint64> LeftTextureResources;
		TArray<uint64> FFRTextureResources;
		uint32_t FoveationWidth = 0;
		uint32_t FoveationHeight = 0;
		TextureResources.Empty();
		LeftTextureResources.Empty();
		FFRTextureResources.Empty();
		bool bNativeTextureCreated = false;
#if PLATFORM_ANDROID
		ExecuteOnRHIThread([&]()
			{
				if (Pxr_IsInitialized())
				{
					Pxr_CreateLayer(&PxrLayerCreateParam);

					uint32_t ImageCounts = 0;
					uint64_t LayerImages[2][3] = {};
					Pxr_GetLayerImageCount(LayerId, PXR_EYE_RIGHT, &ImageCounts);
					ensure(ImageCounts != 0);
					for (uint32_t i = 0; i < ImageCounts; i++)
					{
						Pxr_GetLayerImage(LayerId, PXR_EYE_RIGHT, i, &LayerImages[1][i]);
						PXR_LOGI(PxrUnreal, "Pxr_GetLayerImage Right LayerImages[1][%d]=u_%u", i, (uint32_t)LayerImages[1][i]);
						TextureResources.Add(LayerImages[1][i]);
					}

					if (PxrLayerCreateParam.layerLayout == PXR_LAYER_LAYOUT_STEREO)
					{
						Pxr_GetLayerImageCount(LayerId, PXR_EYE_LEFT, &ImageCounts);
						ensure(ImageCounts != 0);
						for (uint32_t i = 0; i < ImageCounts; i++)
						{
							Pxr_GetLayerImage(LayerId, PXR_EYE_LEFT, i, &LayerImages[0][i]);
							PXR_LOGI(PxrUnreal, "Pxr_GetLayerImage Left LayerImages[0][%d]=u_%u", i, (uint32_t)LayerImages[0][i]);
							LeftTextureResources.Add(LayerImages[0][i]);
						}
					}

					if (bNeedFFRSwapChain)
					{
						uint64_t FoveationImage;
						Pxr_GetLayerFoveationImage(LayerId, PXR_EYE_RIGHT, &FoveationImage, &FoveationWidth, &FoveationHeight);
						FFRTextureResources.Add(FoveationImage);
					}

					bNativeTextureCreated = true;
				}
				else
				{
					PXR_LOGE(PxrUnreal, "Pxr_IsInitialized return false");
				}
			});

		if (bNativeTextureCreated)
		{
			if (!PxrLayer)
			{
				PxrLayer = MakeShareable<FPxrLayer>(new FPxrLayer(LayerId));
			}

#if ENGINE_MINOR_VERSION > 25||ENGINE_MAJOR_VERSION>4
			ETextureCreateFlags Flags;
			ETextureCreateFlags TargetableTextureFlags;
#if ENGINE_MAJOR_VERSION>4
			Flags = TargetableTextureFlags = TexCreate_None;
#else
			Flags = TargetableTextureFlags = ETextureCreateFlags::TexCreate_None;
#endif
#else
			uint32 Flags;
			uint32 TargetableTextureFlags;
			Flags = TargetableTextureFlags = 0;
#endif
#if ENGINE_MAJOR_VERSION>4
			Flags = TargetableTextureFlags |= TexCreate_RenderTargetable | TexCreate_ShaderResource | (IsMobileColorsRGB() ? TexCreate_SRGB : TexCreate_None);
#else
			Flags = TargetableTextureFlags |= ETextureCreateFlags::TexCreate_RenderTargetable | ETextureCreateFlags::TexCreate_ShaderResource | (IsMobileColorsRGB() ? TexCreate_SRGB : TexCreate_None);
#endif
			SwapChain = CustomPresent->CreateSwapChain(LayerId, TextureResources, PF_R8G8B8A8, PxrLayerCreateParam.width, PxrLayerCreateParam.height, PxrLayerCreateParam.arraySize, PxrLayerCreateParam.mipmapCount, PxrLayerCreateParam.sampleCount, Flags, TargetableTextureFlags, MSAAValue);
			if (PxrLayerCreateParam.layerLayout == PXR_LAYER_LAYOUT_STEREO)
			{
				LeftSwapChain = CustomPresent->CreateSwapChain(LayerId, LeftTextureResources, PF_R8G8B8A8, PxrLayerCreateParam.width, PxrLayerCreateParam.height, PxrLayerCreateParam.arraySize, PxrLayerCreateParam.mipmapCount, PxrLayerCreateParam.sampleCount, Flags, TargetableTextureFlags, MSAAValue);
			}

			if (bNeedFFRSwapChain)
			{
#if ENGINE_MINOR_VERSION > 25||ENGINE_MAJOR_VERSION>4
#if ENGINE_MAJOR_VERSION>4
				ETextureCreateFlags	TCF = TexCreate_Foveation;
#else
				ETextureCreateFlags	TCF = ETextureCreateFlags::TexCreate_Foveation;
#endif

#else
				ETextureCreateFlags	TCF = ETextureCreateFlags::TexCreate_None;
#endif
				FoveationSwapChain = CustomPresent->CreateSwapChain(LayerId, FFRTextureResources, PF_R8G8, FoveationWidth, FoveationHeight, PxrLayerCreateParam.arraySize, 1, 1, Flags, TCF, 1);

				if (FoveationSwapChain)
				{
					HMDDevice->bNeedReAllocateFoveationTexture_RenderThread = true;
				}
			}	
			bTextureNeedUpdate = true;
		}
		else
		{
			return false;
		}
#endif
	}            
	if (LayerDesc.Flags & IStereoLayers::LAYER_FLAG_TEX_CONTINUOUS_UPDATE && LayerDesc.Texture.IsValid())
	{
		bTextureNeedUpdate = true;
	}
    return true;
}

bool FPicoXRStereoLayer::IfCanReuseLayers(const FPicoXRStereoLayer* InLayer) const
{
	if (!InLayer || !InLayer->PxrLayer.IsValid())
	{
		return false;
	}

#if PLATFORM_ANDROID
	if (PxrLayerCreateParam.width != InLayer->PxrLayerCreateParam.width				||
		PxrLayerCreateParam.height != InLayer->PxrLayerCreateParam.height			||
		PxrLayerCreateParam.layerShape != InLayer->PxrLayerCreateParam.layerShape   ||
		PxrLayerCreateParam.layerLayout != InLayer->PxrLayerCreateParam.layerLayout ||
		PxrLayerCreateParam.mipmapCount != InLayer->PxrLayerCreateParam.mipmapCount ||
		PxrLayerCreateParam.sampleCount != InLayer->PxrLayerCreateParam.sampleCount ||
		PxrLayerCreateParam.format != InLayer->PxrLayerCreateParam.format)
	{
		return false;
	}
#endif

	return true;
}

void FPicoXRStereoLayer::IncrementSwapChainIndex_RHIThread(FPicoXRRenderBridge* RenderBridge)
{
    if ((LayerDesc.Flags & IStereoLayers::LAYER_FLAG_HIDDEN) != 0)
	{
		return;
	}

	if (SwapChain && SwapChain.IsValid())
	{
		int32 index = 0;
#if PLATFORM_ANDROID
		Pxr_GetLayerNextImageIndex(LayerId, &index);
#endif
		while (index != SwapChain->GetSwapChainIndex_RHIThread())
		{
#if ENGINE_MINOR_VERSION > 26||ENGINE_MAJOR_VERSION>4
			SwapChain->IncrementSwapChainIndex_RHIThread();
#else
			SwapChain->IncrementSwapChainIndex_RHIThread(0);
#endif
		}

		if (LeftSwapChain && LeftSwapChain.IsValid())
		{
			while (index != LeftSwapChain->GetSwapChainIndex_RHIThread())
			{
#if ENGINE_MINOR_VERSION > 26||ENGINE_MAJOR_VERSION>4
				LeftSwapChain->IncrementSwapChainIndex_RHIThread();
#else
				LeftSwapChain->IncrementSwapChainIndex_RHIThread(0);
#endif
			}
		}
	}
}

void FPicoXRStereoLayer::SubmitCompositionLayerRenderMatrix_RHIThread(APlayerController* PlayerController, FQuat& CurrentOrientation, FVector& CurrentPosition, FTransform& CurrentTrackingToWorld)
{
#if PLATFORM_ANDROID
	PXR_LOGV(PxrUnreal, "Layer Submit=%d", LayerId);
    if (LayerId == 0 || ((LayerDesc.Flags & IStereoLayers::LAYER_FLAG_HIDDEN) != 0))
    {
        return;
    }
	int32 ShapeType = GetShapeType();
	FQuat CameraRotation = FQuat::Identity;
	FVector CameraLocation = FVector::ZeroVector;
	FTransform TrackingToWorld =  FTransform::Identity;

	if (LayerDesc.PositionType != IStereoLayers::ELayerType::FaceLocked)
	{
		CameraLocation = CurrentPosition;
		CameraRotation = CurrentOrientation;
		TrackingToWorld = CurrentTrackingToWorld;
		if (PlayerController)
		{
			if (PlayerController->PlayerCameraManager)
			{
				CameraLocation += HMDDevice->InitCamPos;
			}
		}
	}

	FVector LayerPosition = GetLayerLocation();
	LayerPosition = TrackingToWorld.InverseTransformPosition(LayerPosition);
	LayerPosition = FPicoXRUtils::ConvertUnrealVectorToXRVector(LayerPosition,GEngine->XRSystem->GetWorldToMetersScale());
	float ModuleTranslation[3] = { (float)LayerPosition.X,(float)LayerPosition.Y,(float)LayerPosition.Z };
	FQuat LayerOrientation = GetLayerOrientation();
	FRotator LayerRotation = TrackingToWorld.InverseTransformRotation(LayerOrientation).Rotator();

	LayerOrientation = LayerRotation.Quaternion();
	LayerOrientation = FPicoXRUtils::ConvertUnrealQuatToXRQuat(LayerOrientation);
	float ModuleOrientation[4] = { (float)LayerOrientation.X,(float)LayerOrientation.Y,(float)LayerOrientation.Z,(float)LayerOrientation.W };
	FIntPoint TextureSize = LayerDesc.Texture.IsValid() ? LayerDesc.Texture->GetTexture2D()->GetSizeXY() : LayerDesc.LayerSize;
	float Aspect = 1.0f;
	if (TextureSize.X > 0)
		Aspect = (float)TextureSize.Y / (float)TextureSize.X;

	CameraRotation = FPicoXRUtils::ConvertUnrealQuatToXRQuat(CameraRotation);
	float CameraOrientation[4] = { (float)CameraRotation.X,(float)CameraRotation.Y,(float)CameraRotation.Z,(float)CameraRotation.W };
	CameraLocation = FPicoXRUtils::ConvertUnrealVectorToXRVector(CameraLocation,GEngine->XRSystem->GetWorldToMetersScale());
	float CameraTranslation[3] = { (float)CameraLocation.X,(float)CameraLocation.Y,(float)CameraLocation.Z };

    if (ShapeType == (int32)PxrLayerShape::PXR_LAYER_QUAD) {
        PxrLayerQuad layerSubmit = {};
        layerSubmit.header.layerId = LayerId;
        if (LayerDesc.PositionType != IStereoLayers::ELayerType::FaceLocked)
            layerSubmit.header.layerFlags = PXR_LAYER_FLAG_LAYER_POSE_NOT_IN_TRACKING_SPACE | PXR_LAYER_FLAG_USE_EXTERNAL_HEAD_POSE;
        else
            layerSubmit.header.layerFlags = PXR_LAYER_FLAG_LAYER_POSE_NOT_IN_TRACKING_SPACE | PXR_LAYER_FLAG_HEAD_LOCKED;
		
        if (bMRCLayer)
        {
            layerSubmit.header.layerFlags |= 1 << 30;
        }

		bool bAdjustLayersColor = HMDDevice->GbApplyToAllLayers && (!bSplashLayer);
  
		layerSubmit.header.colorScale[0] = bAdjustLayersColor ? HMDDevice->GColorScale.R : 1.0f;
		layerSubmit.header.colorScale[1] = bAdjustLayersColor ? HMDDevice->GColorScale.G : 1.0f;
		layerSubmit.header.colorScale[2] = bAdjustLayersColor ? HMDDevice->GColorScale.B : 1.0f;
		layerSubmit.header.colorScale[3] = bAdjustLayersColor ? HMDDevice->GColorScale.A : 1.0f;

		layerSubmit.header.colorBias[0] = bAdjustLayersColor ? HMDDevice->GColorOffset.R : 0.0f;
		layerSubmit.header.colorBias[1] = bAdjustLayersColor ? HMDDevice->GColorOffset.G : 0.0f;
		layerSubmit.header.colorBias[2] = bAdjustLayersColor ? HMDDevice->GColorOffset.B : 0.0f;
		layerSubmit.header.colorBias[3] = bAdjustLayersColor ? HMDDevice->GColorOffset.A : 0.0f;


        layerSubmit.header.compositionDepth = 0;
        layerSubmit.header.headPose.orientation.x = CameraRotation.X;
        layerSubmit.header.headPose.orientation.y = CameraRotation.Y;
        layerSubmit.header.headPose.orientation.z = CameraRotation.Z;
        layerSubmit.header.headPose.orientation.w = CameraRotation.W;
        layerSubmit.header.headPose.position.x = CameraLocation.X;
        layerSubmit.header.headPose.position.y = CameraLocation.Y;
        layerSubmit.header.headPose.position.z = CameraLocation.Z;

        layerSubmit.pose.orientation.x = LayerOrientation.X;
        layerSubmit.pose.orientation.y = LayerOrientation.Y;
        layerSubmit.pose.orientation.z = LayerOrientation.Z;
        layerSubmit.pose.orientation.w = LayerOrientation.W;
        layerSubmit.pose.position.x = LayerPosition.X;
        layerSubmit.pose.position.y = LayerPosition.Y;
        layerSubmit.pose.position.z = LayerPosition.Z;

        float QuadSizeX = LayerDesc.QuadSize.X;
        float QuadSizeY = LayerDesc.QuadSize.Y;
        if (LayerDesc.Flags & IStereoLayers::LAYER_FLAG_QUAD_PRESERVE_TEX_RATIO)
        {
            QuadSizeY = QuadSizeX * Aspect;
        }
        FVector QuadScale = GetLayerScale();
        FVector QuadSize = FVector(-GEngine->XRSystem->GetWorldToMetersScale(), QuadSizeX, QuadSizeY);
        FVector Scale = QuadScale * QuadSize;
        Scale = FPicoXRUtils::ConvertUnrealVectorToXRVector(Scale, GEngine->XRSystem->GetWorldToMetersScale());
        layerSubmit.size[0] = (float)Scale.X;
        layerSubmit.size[1] = (float)Scale.Y;

        Pxr_SubmitLayer((PxrLayerHeader*)&layerSubmit);
    }
    else if (ShapeType == (int32)PxrLayerShape::PXR_LAYER_CYLINDER) {
        PxrLayerCylinder layerSubmit = {};
        layerSubmit.header.layerId = LayerId;
        if (LayerDesc.PositionType != IStereoLayers::ELayerType::FaceLocked)
            layerSubmit.header.layerFlags = PXR_LAYER_FLAG_LAYER_POSE_NOT_IN_TRACKING_SPACE | PXR_LAYER_FLAG_USE_EXTERNAL_HEAD_POSE;
        else
            layerSubmit.header.layerFlags = PXR_LAYER_FLAG_LAYER_POSE_NOT_IN_TRACKING_SPACE | PXR_LAYER_FLAG_HEAD_LOCKED;

        bool bAdjustLayersColor = HMDDevice->GbApplyToAllLayers && (!bSplashLayer);

		layerSubmit.header.colorScale[0] = bAdjustLayersColor ? HMDDevice->GColorScale.R : 1.0f;
		layerSubmit.header.colorScale[1] = bAdjustLayersColor ? HMDDevice->GColorScale.G : 1.0f;
		layerSubmit.header.colorScale[2] = bAdjustLayersColor ? HMDDevice->GColorScale.B : 1.0f;
		layerSubmit.header.colorScale[3] = bAdjustLayersColor ? HMDDevice->GColorScale.A : 1.0f;

		layerSubmit.header.colorBias[0] = bAdjustLayersColor ? HMDDevice->GColorOffset.R : 0.0f;
		layerSubmit.header.colorBias[1] = bAdjustLayersColor ? HMDDevice->GColorOffset.G : 0.0f;
		layerSubmit.header.colorBias[2] = bAdjustLayersColor ? HMDDevice->GColorOffset.B : 0.0f;
		layerSubmit.header.colorBias[3] = bAdjustLayersColor ? HMDDevice->GColorOffset.A : 0.0f;

        layerSubmit.header.compositionDepth = 0;
        layerSubmit.header.headPose.orientation.x = CameraRotation.X;
        layerSubmit.header.headPose.orientation.y = CameraRotation.Y;
        layerSubmit.header.headPose.orientation.z = CameraRotation.Z;
        layerSubmit.header.headPose.orientation.w = CameraRotation.W;
        layerSubmit.header.headPose.position.x = CameraLocation.X;
        layerSubmit.header.headPose.position.y = CameraLocation.Y;
        layerSubmit.header.headPose.position.z = CameraLocation.Z;

        layerSubmit.pose.orientation.x = LayerOrientation.X;
        layerSubmit.pose.orientation.y = LayerOrientation.Y;
        layerSubmit.pose.orientation.z = LayerOrientation.Z;
        layerSubmit.pose.orientation.w = LayerOrientation.W;
        layerSubmit.pose.position.x = LayerPosition.X;
        layerSubmit.pose.position.y = LayerPosition.Y;
        layerSubmit.pose.position.z = LayerPosition.Z;

        FVector Scale;
#if ENGINE_MINOR_VERSION > 24||ENGINE_MAJOR_VERSION>4
        const FCylinderLayer& CylinderProps = LayerDesc.GetShape<FCylinderLayer>();
        Scale.X = CylinderProps.OverlayArc;
        Scale.Y = (LayerDesc.Flags & IStereoLayers::LAYER_FLAG_QUAD_PRESERVE_TEX_RATIO) ? CylinderProps.OverlayArc * Aspect : CylinderProps.Height;
        Scale.Z = CylinderProps.Radius;
#else
        Scale.X = LayerDesc.CylinderOverlayArc;
        Scale.Y = (LayerDesc.Flags & IStereoLayers::LAYER_FLAG_QUAD_PRESERVE_TEX_RATIO) ? LayerDesc.CylinderOverlayArc * Aspect : LayerDesc.CylinderHeight;
        Scale.Z = LayerDesc.CylinderRadius;
#endif
        Scale = Scale / GEngine->XRSystem->GetWorldToMetersScale();
        layerSubmit.centralAngle = Scale.X / Scale.Z;
        layerSubmit.height = Scale.Y;
        layerSubmit.radius = Scale.Z;

        Pxr_SubmitLayer((PxrLayerHeader*)&layerSubmit);
    }
#endif
}

int32 FPicoXRStereoLayer::GetShapeType()
{
	int32 ShapeType = 0;
#if PLATFORM_ANDROID
#if ENGINE_MINOR_VERSION>24||ENGINE_MAJOR_VERSION>4
	if (LayerDesc.HasShape<FQuadLayer>())
	{
		ShapeType = 1;
	}
	else if (LayerDesc.HasShape<FCylinderLayer>())
	{
		ShapeType = 2;
	}
	else if (LayerDesc.HasShape<FCubemapLayer>())
	{
		ShapeType = 4;
	}
	else if (LayerDesc.HasShape<FEquirectLayer>())
	{
		ShapeType = 3;
	}

#else
	switch (LayerDesc.ShapeType)
	{
	case IStereoLayers::QuadLayer:
		ShapeType = 1;
		break;
	case IStereoLayers::CylinderLayer:
		ShapeType = 2;
		break;
	case IStereoLayers::CubemapLayer:
		ShapeType = 4;
		break;
	default:
		ShapeType = 0;
		break;
	}
#endif
#endif
	return ShapeType;
}


void FPicoXRStereoLayer::SetEyeLayerParams(uint32 SizeX, uint32 SizeY,uint32 ArraySize, uint32 NumMips, uint32 NumSamples)
{
#if PLATFORM_ANDROID
	PxrLayerCreateParam.layerId = LayerId;
	PxrLayerCreateParam.layerShape = PXR_LAYER_PROJECTION;
	PxrLayerCreateParam.width = SizeX;
	PxrLayerCreateParam.height = SizeY;
	PxrLayerCreateParam.faceCount = 1;
	PxrLayerCreateParam.mipmapCount = NumMips;
	PxrLayerCreateParam.sampleCount = NumSamples;
	PxrLayerCreateParam.arraySize = ArraySize;
	PxrLayerCreateParam.layerLayout = (ArraySize == 2 ? PXR_LAYER_LAYOUT_ARRAY : PXR_LAYER_LAYOUT_DOUBLE_WIDE);
#endif
}

