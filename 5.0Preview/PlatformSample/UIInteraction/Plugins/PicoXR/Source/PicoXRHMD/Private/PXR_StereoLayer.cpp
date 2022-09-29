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
#include "PXR_GameFrame.h"

#if PLATFORM_ANDROID
#include "OpenGLDrvPrivate.h"
#include "OpenGLResources.h"
#include "VulkanRHIPrivate.h"
#include "VulkanResources.h"
#endif

FPxrLayer::FPxrLayer(uint32 InPxrLayerId,FDelayDeleteLayerManager* InDelayDeletion) :
	PxrLayerId(InPxrLayerId),
	DelayDeletion(InDelayDeletion)
{
}

FPxrLayer::~FPxrLayer()
{
	if (IsInGameThread())
	{
		ExecuteOnRenderThread([PxrLayerId = this->PxrLayerId, DelayDeletion = this->DelayDeletion]()
		{
			DelayDeletion->AddPxrLayerToDeferredDeletionQueue(PxrLayerId);
		});
	}
	else
	{
		DelayDeletion->AddPxrLayerToDeferredDeletionQueue(PxrLayerId);
	}
}

uint64_t OverlayImages[2] = {};
uint64_t OverlayNativeImages[2][3] = {};

uint32 FPicoXRStereoLayer::PxrLayerIDCounter = 0;

FPicoXRStereoLayer::FPicoXRStereoLayer(FPicoXRHMD* InHMDDevice, uint32 InPXRLayerId, const IStereoLayers::FLayerDesc& InDesc)
	: bSplashLayer(false)
	, bSplashBlackProjectionLayer(false)
	, bMRCLayer(false)
    , HMDDevice(InHMDDevice)
	, ID(InPXRLayerId)
	, PxrLayerID(0)
    , bTextureNeedUpdate(false)
    , UnderlayMeshComponent(NULL)
    , UnderlayActor(NULL)
    , PxrLayer(nullptr)
{
    PXR_LOGD(PxrUnreal, "FPicoXRStereoLayer with ID=%d", ID);

#if PLATFORM_ANDROID
	FMemory::Memzero(PxrLayerCreateParam);
#endif

    SetPXRLayerDesc(InDesc);
}

FPicoXRStereoLayer::FPicoXRStereoLayer(const FPicoXRStereoLayer& InPXRLayer)
    : bSplashLayer(InPXRLayer.bSplashLayer)
	, bSplashBlackProjectionLayer(InPXRLayer.bSplashBlackProjectionLayer)
    , bMRCLayer(InPXRLayer.bMRCLayer)
    , HMDDevice(InPXRLayer.HMDDevice)
	, ID(InPXRLayer.ID)
	, PxrLayerID(InPXRLayer.PxrLayerID)
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
		const FString UnderlayNameStr = FString::Printf(TEXT("PicoUnderlay_%d"), ID);
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

	PXR_LOGV(PxrUnreal, "ID=%d, bTextureNeedUpdate=%d, IsVisible:%d, SwapChain.IsValid=%d, LayerDesc.Texture.IsValid=%d", ID, bTextureNeedUpdate, IsVisible(), SwapChain.IsValid(), LayerDesc.Texture.IsValid());

	if (bTextureNeedUpdate && IsVisible())
	{
        // Copy textures
        if (LayerDesc.Texture.IsValid() && SwapChain.IsValid())
        {
            bool bNoAlpha = (LayerDesc.Flags & IStereoLayers::LAYER_FLAG_TEX_NO_ALPHA_CHANNEL) != 0;

            // Mono
            FRHITexture* SrcTexture = LayerDesc.Texture;
            FRHITexture* DstTexture = SwapChain->GetTexture();
            RenderBridge->TransferImage_RenderThread(RHICmdList, DstTexture, SrcTexture, FIntRect(), FIntRect(), true, bNoAlpha, bMRCLayer);

            // Stereo
            if (LayerDesc.LeftTexture.IsValid() && LeftSwapChain.IsValid())
            {
                FRHITexture* LeftSrcTexture = LayerDesc.LeftTexture;
                FRHITexture* LeftDstTexture = LeftSwapChain->GetTexture();
                RenderBridge->TransferImage_RenderThread(RHICmdList, LeftDstTexture, LeftSrcTexture, FIntRect(), FIntRect(), true, bNoAlpha, bMRCLayer);
            }
			
			bTextureNeedUpdate = false;
        }
    }
}

bool FPicoXRStereoLayer::InitPXRLayer_RenderThread(FPicoXRRenderBridge* CustomPresent, FDelayDeleteLayerManager* DelayDeletion, FRHICommandListImmediate& RHICmdList, const FPicoXRStereoLayer* InLayer)
{
	check(IsInRenderingThread());

	int32 MSAAValue = 1;
	bool bNeedFFRSwapChain = false;
	if (ID == 0)
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
	else if (!bSplashBlackProjectionLayer)
	{
		MSAAValue = 1;
#if PLATFORM_ANDROID
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
#if ENGINE_MINOR_VERSION > 24||ENGINE_MAJOR_VERSION>4
				PxrLayerCreateParam.mipmapCount = Texture2D->GetNumMips();
#else
				PxrLayerCreateParam.mipmapCount = 1;
#endif
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

		if (CustomPresent->RHIString == TEXT("OpenGL"))
		{
			PxrLayerCreateParam.format = IsMobileColorsRGB() ? GL_SRGB8_ALPHA8 : GL_RGBA8;
		}
		else if (CustomPresent->RHIString == TEXT("Vulkan"))
		{
			PxrLayerCreateParam.format = IsMobileColorsRGB() ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
		}
#endif
	}

	if (IfCanReuseLayers(InLayer))
	{
		//GameThread = RenderThread
		PxrLayerID = InLayer->PxrLayerID;
		PxrLayer = InLayer->PxrLayer;
		SwapChain = InLayer->SwapChain;
		LeftSwapChain = InLayer->LeftSwapChain;
        FoveationSwapChain =InLayer->FoveationSwapChain;
		bTextureNeedUpdate |= InLayer->bTextureNeedUpdate;
	}
    else
	{
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
				PxrLayerCreateParam.layerId = PxrLayerID = PxrLayerIDCounter;
				if (Pxr_IsInitialized() && (Pxr_CreateLayer(&PxrLayerCreateParam) == 0))
				{
					PxrLayerIDCounter++;
					uint32_t ImageCounts = 0;
					uint64_t LayerImages[2][3] = {};
					Pxr_GetLayerImageCount(PxrLayerID, PXR_EYE_RIGHT, &ImageCounts);
					ensure(ImageCounts != 0);
					for (uint32_t i = 0; i < ImageCounts; i++)
					{
						Pxr_GetLayerImage(PxrLayerID, PXR_EYE_RIGHT, i, &LayerImages[1][i]);
						PXR_LOGI(PxrUnreal, "Pxr_GetLayerImage Right LayerImages[1][%d]=u_%u", i, (uint32_t)LayerImages[1][i]);
						TextureResources.Add(LayerImages[1][i]);
					}

					if (PxrLayerCreateParam.layerLayout == PXR_LAYER_LAYOUT_STEREO)
					{
						Pxr_GetLayerImageCount(PxrLayerID, PXR_EYE_LEFT, &ImageCounts);
						ensure(ImageCounts != 0);
						for (uint32_t i = 0; i < ImageCounts; i++)
						{
							Pxr_GetLayerImage(PxrLayerID, PXR_EYE_LEFT, i, &LayerImages[0][i]);
							PXR_LOGI(PxrUnreal, "Pxr_GetLayerImage Left LayerImages[0][%d]=u_%u", i, (uint32_t)LayerImages[0][i]);
							LeftTextureResources.Add(LayerImages[0][i]);
						}
					}

					if (bNeedFFRSwapChain)
					{
						uint64_t FoveationImage;
						Pxr_GetLayerFoveationImage(PxrLayerID, PXR_EYE_RIGHT, &FoveationImage, &FoveationWidth, &FoveationHeight);
						FFRTextureResources.Add(FoveationImage);
					}

					bNativeTextureCreated = true;
				}
				else
				{
					PXR_LOGE(PxrUnreal, "Create native texture failed!");
				}
			});

		if (bNativeTextureCreated)
		{
			PxrLayer = MakeShareable<FPxrLayer>(new FPxrLayer(PxrLayerID, DelayDeletion));

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
			SwapChain = CustomPresent->CreateSwapChain(PxrLayerID, TextureResources, PF_R8G8B8A8, PxrLayerCreateParam.width, PxrLayerCreateParam.height, PxrLayerCreateParam.arraySize, PxrLayerCreateParam.mipmapCount, PxrLayerCreateParam.sampleCount, Flags, TargetableTextureFlags, MSAAValue);
			if (PxrLayerCreateParam.layerLayout == PXR_LAYER_LAYOUT_STEREO)
			{
				LeftSwapChain = CustomPresent->CreateSwapChain(PxrLayerID, LeftTextureResources, PF_R8G8B8A8, PxrLayerCreateParam.width, PxrLayerCreateParam.height, PxrLayerCreateParam.arraySize, PxrLayerCreateParam.mipmapCount, PxrLayerCreateParam.sampleCount, Flags, TargetableTextureFlags, MSAAValue);
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
				FoveationSwapChain = CustomPresent->CreateSwapChain(PxrLayerID, FFRTextureResources, PF_R8G8, FoveationWidth, FoveationHeight, PxrLayerCreateParam.arraySize, 1, 1, Flags, TCF, 1);

				if (FoveationSwapChain)
				{
					HMDDevice->bNeedReAllocateFoveationTexture_RenderThread = true;
				}
			}	
			bTextureNeedUpdate = true;
		}
		else
		{
			PXR_LOGE(PxrUnreal, "Create SwapChain failed!");
			return false;
		}
#endif
	}            
	if ((LayerDesc.Flags & IStereoLayers::LAYER_FLAG_TEX_CONTINUOUS_UPDATE) && LayerDesc.Texture.IsValid() && IsVisible())
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
		Pxr_GetLayerNextImageIndex(PxrLayerID, &index);
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

void FPicoXRStereoLayer::SubmitLayer_RHIThread(FPXRGameFrame* Frame)
{
	PXR_LOGV(PxrUnreal, "Submit Layer:%u", ID);
#if PLATFORM_ANDROID
	if (ID == 0)
	{
		PxrLayerProjection layerProjection = {};
		layerProjection.header.layerId = PxrLayerID;
		layerProjection.header.layerFlags = 0;
		layerProjection.header.sensorFrameIndex = Frame->ViewNumber;
		bool bDrawBlackEye = HMDDevice->bIsSwitchingLevel;
		layerProjection.header.colorScale[0] = bDrawBlackEye ? 0.0f : HMDDevice->GColorScale.R;
		layerProjection.header.colorScale[1] = bDrawBlackEye ? 0.0f : HMDDevice->GColorScale.G;
		layerProjection.header.colorScale[2] = bDrawBlackEye ? 0.0f : HMDDevice->GColorScale.B;
		layerProjection.header.colorScale[3] = bDrawBlackEye ? 0.0f : HMDDevice->GColorScale.A;
		layerProjection.header.colorBias[0] = bDrawBlackEye ? 0.0f : HMDDevice->GColorOffset.R;
		layerProjection.header.colorBias[1] = bDrawBlackEye ? 0.0f : HMDDevice->GColorOffset.G;
		layerProjection.header.colorBias[2] = bDrawBlackEye ? 0.0f : HMDDevice->GColorOffset.B;
		layerProjection.header.colorBias[3] = bDrawBlackEye ? 0.0f : HMDDevice->GColorOffset.A;
		Pxr_SubmitLayer((PxrLayerHeader*)&layerProjection);
	}
	else if (bSplashBlackProjectionLayer)
	{
		PxrLayerProjection layerProjection = {};
		layerProjection.header.layerId = PxrLayerID;
		layerProjection.header.layerFlags = 0;
		layerProjection.header.sensorFrameIndex = Frame->ViewNumber;
		layerProjection.header.colorScale[0] = 0.0f;
		layerProjection.header.colorScale[1] = 0.0f;
		layerProjection.header.colorScale[2] = 0.0f;
		layerProjection.header.colorScale[3] = 0.0f;
		Pxr_SubmitLayer((PxrLayerHeader*)&layerProjection);
	}
	else
	{
		FTransform BaseTransform = FTransform::Identity;
		uint32 Flags = 0;
		Flags |= bMRCLayer ? (1 << 30) : 0;
		switch (LayerDesc.PositionType)
		{
		case IStereoLayers::WorldLocked:
			BaseTransform.SetRotation(Frame->TrackingToWorld.GetRotation());
			BaseTransform.SetLocation(Frame->TrackingToWorld.GetTranslation());
			break;

		case IStereoLayers::TrackerLocked:
			break;

		case IStereoLayers::FaceLocked:
			Flags |= PXR_LAYER_FLAG_HEAD_LOCKED;
			break;
		}

		FVector LayerPosition = GetLayerLocation();
		LayerPosition = BaseTransform.InverseTransformPosition(LayerPosition);
		LayerPosition = FPicoXRUtils::ConvertUnrealVectorToXRVector(LayerPosition, Frame->WorldToMetersScale);

		FQuat LayerOrientation = GetLayerOrientation();
		LayerOrientation = BaseTransform.InverseTransformRotation(LayerOrientation);
		LayerOrientation = FPicoXRUtils::ConvertUnrealQuatToXRQuat(LayerOrientation);

		int SizeX = PxrLayerCreateParam.width;
		int SizeY = PxrLayerCreateParam.height;
		float AspectRatio = SizeX ? (float)SizeY / (float)SizeX : 3.0f / 4.0f;
		FVector Scale = FPicoXRUtils::ConvertUnrealVectorToXRVector(LayerDesc.Transform.GetScale3D(), Frame->WorldToMetersScale);

		bool bAdjustLayersColor = HMDDevice->GbApplyToAllLayers && (!bSplashLayer);

		int32 ShapeType = GetShapeType();
		if (ShapeType == (int32)PxrLayerShape::PXR_LAYER_QUAD)
		{
			PxrLayerQuad layerSubmit = {};
			layerSubmit.header.layerId = PxrLayerID;
			layerSubmit.header.compositionDepth = 0;
			layerSubmit.header.sensorFrameIndex = Frame->ViewNumber;
			layerSubmit.header.layerFlags = Flags;

			layerSubmit.header.colorScale[0] = bAdjustLayersColor ? HMDDevice->GColorScale.R : 1.0f;
			layerSubmit.header.colorScale[1] = bAdjustLayersColor ? HMDDevice->GColorScale.G : 1.0f;
			layerSubmit.header.colorScale[2] = bAdjustLayersColor ? HMDDevice->GColorScale.B : 1.0f;
			layerSubmit.header.colorScale[3] = bAdjustLayersColor ? HMDDevice->GColorScale.A : 1.0f;

			layerSubmit.header.colorBias[0] = bAdjustLayersColor ? HMDDevice->GColorOffset.R : 0.0f;
			layerSubmit.header.colorBias[1] = bAdjustLayersColor ? HMDDevice->GColorOffset.G : 0.0f;
			layerSubmit.header.colorBias[2] = bAdjustLayersColor ? HMDDevice->GColorOffset.B : 0.0f;
			layerSubmit.header.colorBias[3] = bAdjustLayersColor ? HMDDevice->GColorOffset.A : 0.0f;

			layerSubmit.pose.orientation.x = LayerOrientation.X;
			layerSubmit.pose.orientation.y = LayerOrientation.Y;
			layerSubmit.pose.orientation.z = LayerOrientation.Z;
			layerSubmit.pose.orientation.w = LayerOrientation.W;
			layerSubmit.pose.position.x = LayerPosition.X;
			layerSubmit.pose.position.y = LayerPosition.Y;
			layerSubmit.pose.position.z = LayerPosition.Z;

			float QuadSizeY = (LayerDesc.Flags & IStereoLayers::LAYER_FLAG_QUAD_PRESERVE_TEX_RATIO) ? LayerDesc.QuadSize.X * AspectRatio : LayerDesc.QuadSize.Y;
			layerSubmit.size[0] = (float)(LayerDesc.QuadSize.X * Scale.X);
			layerSubmit.size[1] = (float)(QuadSizeY * Scale.Y);

			Pxr_SubmitLayer((PxrLayerHeader*)&layerSubmit);
		}
		else if (ShapeType == (int32)PxrLayerShape::PXR_LAYER_CYLINDER)
		{
			PxrLayerCylinder layerSubmit = {};
			layerSubmit.header.layerId = PxrLayerID;
			layerSubmit.header.compositionDepth = 0;
			layerSubmit.header.sensorFrameIndex = Frame->ViewNumber;
			layerSubmit.header.layerFlags = Flags;

			layerSubmit.header.colorScale[0] = bAdjustLayersColor ? HMDDevice->GColorScale.R : 1.0f;
			layerSubmit.header.colorScale[1] = bAdjustLayersColor ? HMDDevice->GColorScale.G : 1.0f;
			layerSubmit.header.colorScale[2] = bAdjustLayersColor ? HMDDevice->GColorScale.B : 1.0f;
			layerSubmit.header.colorScale[3] = bAdjustLayersColor ? HMDDevice->GColorScale.A : 1.0f;

			layerSubmit.header.colorBias[0] = bAdjustLayersColor ? HMDDevice->GColorOffset.R : 0.0f;
			layerSubmit.header.colorBias[1] = bAdjustLayersColor ? HMDDevice->GColorOffset.G : 0.0f;
			layerSubmit.header.colorBias[2] = bAdjustLayersColor ? HMDDevice->GColorOffset.B : 0.0f;
			layerSubmit.header.colorBias[3] = bAdjustLayersColor ? HMDDevice->GColorOffset.A : 0.0f;

			layerSubmit.pose.orientation.x = LayerOrientation.X;
			layerSubmit.pose.orientation.y = LayerOrientation.Y;
			layerSubmit.pose.orientation.z = LayerOrientation.Z;
			layerSubmit.pose.orientation.w = LayerOrientation.W;
			layerSubmit.pose.position.x = LayerPosition.X;
			layerSubmit.pose.position.y = LayerPosition.Y;
			layerSubmit.pose.position.z = LayerPosition.Z;

#if ENGINE_MINOR_VERSION > 24||ENGINE_MAJOR_VERSION>4
			const FCylinderLayer& CylinderProps = LayerDesc.GetShape<FCylinderLayer>();
			float CylinderHeight = (LayerDesc.Flags & IStereoLayers::LAYER_FLAG_QUAD_PRESERVE_TEX_RATIO) ? CylinderProps.OverlayArc * AspectRatio : CylinderProps.Height;
			layerSubmit.centralAngle = CylinderProps.OverlayArc * Scale.X;
			layerSubmit.height = CylinderHeight * Scale.X;
			layerSubmit.radius = CylinderProps.Radius * Scale.X;
#else
			float CylinderHeight = (LayerDesc.Flags & IStereoLayers::LAYER_FLAG_QUAD_PRESERVE_TEX_RATIO) ? LayerDesc.CylinderOverlayArc * AspectRatio : LayerDesc.CylinderHeight;
			layerSubmit.centralAngle = LayerDesc.CylinderOverlayArc * Scale.X;
			layerSubmit.height = CylinderHeight * Scale.X;
			layerSubmit.radius = LayerDesc.CylinderRadius * Scale.X;
#endif
			Pxr_SubmitLayer((PxrLayerHeader*)&layerSubmit);
		}
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

void FPicoXRStereoLayer::SetProjectionLayerParams(uint32 SizeX, uint32 SizeY, uint32 ArraySize, uint32 NumMips, uint32 NumSamples, FString RHIString)
{
#if PLATFORM_ANDROID
	PxrLayerCreateParam.layerShape = PXR_LAYER_PROJECTION;
	PxrLayerCreateParam.width = SizeX;
	PxrLayerCreateParam.height = SizeY;
	PxrLayerCreateParam.faceCount = 1;
	PxrLayerCreateParam.mipmapCount = NumMips;
	PxrLayerCreateParam.sampleCount = NumSamples;
	PxrLayerCreateParam.arraySize = ArraySize;
	PxrLayerCreateParam.layerLayout = (ArraySize == 2 ? PXR_LAYER_LAYOUT_ARRAY : PXR_LAYER_LAYOUT_DOUBLE_WIDE);
	if (RHIString == TEXT("OpenGL"))
	{
		PxrLayerCreateParam.format = IsMobileColorsRGB() ? GL_SRGB8_ALPHA8 : GL_RGBA8;
	}
	else if (RHIString == TEXT("Vulkan"))
	{
		PxrLayerCreateParam.format = IsMobileColorsRGB() ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
	}
	if (bSplashBlackProjectionLayer)
	{
		PxrLayerCreateParam.layerFlags |= PXR_LAYER_FLAG_STATIC_IMAGE;
	}
#endif
}

