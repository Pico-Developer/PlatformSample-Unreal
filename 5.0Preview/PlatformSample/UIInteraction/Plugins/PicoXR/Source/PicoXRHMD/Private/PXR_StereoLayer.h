//Unreal® Engine, Copyright 1998 – 2022, Epic Games, Inc. All rights reserved.

#pragma once
#include "ProceduralMeshComponent.h"
#include "IStereoLayers.h"
#include "PXR_HMDRenderBridge.h"
#include "XRSwapChain.h"
#include "GameFramework/PlayerController.h"

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#include "PxrApi.h"
#endif

class FPxrLayer : public TSharedFromThis<FPxrLayer, ESPMode::ThreadSafe>
{
public:
	FPxrLayer(uint32 InPxrLayerId);
	~FPxrLayer();

protected:
	uint32 PxrLayerId;
};

typedef TSharedPtr<FPxrLayer, ESPMode::ThreadSafe> FPxrLayerPtr;

class FPicoXRStereoLayer : public TSharedFromThis<FPicoXRStereoLayer, ESPMode::ThreadSafe>
{
public:
	FPicoXRStereoLayer(FPicoXRHMD* InHMDDevice,uint32 InPXRLayerId, const IStereoLayers::FLayerDesc& InLayerDesc);
	FPicoXRStereoLayer(const FPicoXRStereoLayer& InPXRLayer);
	~FPicoXRStereoLayer();

	TSharedPtr<FPicoXRStereoLayer, ESPMode::ThreadSafe> CloneMyself() const;
	void SetPXRLayerDesc(const IStereoLayers::FLayerDesc& InDesc);
	const IStereoLayers::FLayerDesc& GetPXRLayerDesc() const { return LayerDesc; }
	const uint32& GetPXRLayerID()const{return LayerId;}

	bool IsLayerSupportDepth() { return (LayerDesc.Flags & IStereoLayers::LAYER_FLAG_SUPPORT_DEPTH) != 0; }
	void ManageUnderlayComponent();
	void CreateQuadUnderlayMesh(TArray<FVector>& VerticePos, TArray<int32>& TriangleIndics, TArray<FVector2D>& TexUV);

	const FXRSwapChainPtr& GetSwapChain() const { return SwapChain; }
	const FXRSwapChainPtr& GetLeftSwapChain() const { return LeftSwapChain; }
	const FXRSwapChainPtr& GetFoveationSwapChain() const { return FoveationSwapChain; }
	void IncrementSwapChainIndex_RHIThread(FPicoXRRenderBridge* RenderBridge);
	void SubmitCompositionLayerRenderMatrix_RHIThread(APlayerController* PlayerController, FQuat& CurrentOrientation, FVector& CurrentPosition, FTransform& CurrentTrackingToWorld);
	int32 GetShapeType();
	void SetEyeLayerParams(uint32 SizeX, uint32 SizeY, uint32 ArraySize, uint32 NumMips, uint32 NumSamples);
    void PXRLayersCopy_RenderThread(FPicoXRRenderBridge* RenderBridge, FRHICommandListImmediate& RHICmdList);
	void MarkTextureForUpdate(bool bUpdate = true) { bTextureNeedUpdate = bUpdate; }
	bool InitPXRLayer_RenderThread(FPicoXRRenderBridge* CustomPresent, FRHICommandListImmediate& RHICmdList, const FPicoXRStereoLayer* InLayer = nullptr);
	bool IfCanReuseLayers(const FPicoXRStereoLayer* InLayer) const;

	bool bSplashLayer = false;
	bool bMRCLayer = false;

protected:
	FVector GetLayerLocation() const { return LayerDesc.Transform.GetLocation(); };
	FQuat GetLayerOrientation() const { return LayerDesc.Transform.GetRotation(); };
	FVector GetLayerScale() const { return LayerDesc.Transform.GetScale3D(); };
	FPicoXRHMD* HMDDevice;
	uint32 LayerId;
	IStereoLayers::FLayerDesc LayerDesc;
	FXRSwapChainPtr SwapChain;
	FXRSwapChainPtr LeftSwapChain;
	FXRSwapChainPtr FoveationSwapChain;
    bool bTextureNeedUpdate;
	UProceduralMeshComponent* UnderlayMeshComponent;
	AActor* UnderlayActor;

	FPxrLayerPtr PxrLayer;
#if PLATFORM_ANDROID
	PxrLayerParam PxrLayerCreateParam;
#endif

};

typedef TSharedPtr<FPicoXRStereoLayer, ESPMode::ThreadSafe> FPicoLayerPtr;

struct FPicoLayerPtr_SortByPriority
{
	FORCEINLINE bool operator()(const FPicoLayerPtr&A,const FPicoLayerPtr&B)const
	{
		int32 nPriorityA = A->GetPXRLayerDesc().Priority;
		int32 nPriorityB = B->GetPXRLayerDesc().Priority;
		if (nPriorityA < nPriorityB)
		{
			return true;
		}
		else if (nPriorityA > nPriorityB)
		{
			return false;
		}
		else
		{
			return A->GetPXRLayerID() < B->GetPXRLayerID();
		}
	}
};

struct FPicoLayerPtr_SortById
{
	FORCEINLINE bool operator()(const FPicoLayerPtr& A, const FPicoLayerPtr& B) const
	{
		return A->GetPXRLayerID() < B->GetPXRLayerID();
	}
};

struct FLayerPtr_CompareByAll
{
	FORCEINLINE bool operator()(const FPicoLayerPtr& A, const FPicoLayerPtr& B) const
	{
		int32 OrderA = (A->GetPXRLayerID() == 0) ? 0 : A->IsLayerSupportDepth() ? -1 : 1;
		int32 OrderB = (B->GetPXRLayerID() == 0) ? 0 : B->IsLayerSupportDepth() ? -1 : 1;

		if (OrderA != OrderB)
		{
			return OrderA < OrderB;
		}

		const IStereoLayers::FLayerDesc& DescA = A->GetPXRLayerDesc();
		const IStereoLayers::FLayerDesc& DescB = B->GetPXRLayerDesc();

		bool bFaceLockedA = (DescA.PositionType == IStereoLayers::ELayerType::FaceLocked);
		bool bFaceLockedB = (DescB.PositionType == IStereoLayers::ELayerType::FaceLocked);

		if (bFaceLockedA != bFaceLockedB)
		{
			return bFaceLockedB;
		}

		if (DescA.Priority != DescB.Priority)
		{
			return DescA.Priority < DescB.Priority;
		}

		return A->GetPXRLayerID() < B->GetPXRLayerID();
	}
};
