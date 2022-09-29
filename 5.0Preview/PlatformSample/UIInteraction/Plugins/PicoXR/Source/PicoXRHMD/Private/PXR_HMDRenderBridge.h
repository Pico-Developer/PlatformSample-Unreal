//Unreal® Engine, Copyright 1998 – 2022, Epic Games, Inc. All rights reserved.

#pragma once
#include "CoreMinimal.h"
#include "XRRenderBridge.h"
#include "XRSwapChain.h"
#include "RHI.h"
#include "RendererInterface.h"
#include "IStereoLayers.h"
#include "XRRenderBridge.h"

class FPicoXRHMD;
class FPicoXRRenderBridge : public FXRRenderBridge
{
public:
    FPicoXRRenderBridge(FPicoXRHMD* HMD);
    /** FRHICustomPresent */
    virtual bool NeedsNativePresent() override;
    virtual bool Present(int32& InOutSyncInterval)override;
#if ENGINE_MINOR_VERSION >25||ENGINE_MAJOR_VERSION>4
    virtual FXRSwapChainPtr CreateSwapChain(uint32 LayerID, TArray<uint64>& NativeTextures, uint8 Format, uint32 SizeX, uint32 SizeY, uint32 ArraySize, uint32 NumMips, uint32 NumSamples, ETextureCreateFlags Flags, ETextureCreateFlags TargetableTextureFlags,uint32 MSAAValue) = 0;
#else
    virtual FXRSwapChainPtr CreateSwapChain(uint32 LayerID, TArray<uint64>& NativeTextures, uint8 Format, uint32 SizeX, uint32 SizeY, uint32 ArraySize, uint32 NumMips, uint32 NumSamples, uint32 Flags, uint32 TargetableTextureFlags,uint32 MSAAValue) = 0;
#endif
    virtual void GetVulkanGraphics();
    virtual int GetSystemRecommendedMSAA() const;
	void TransferImage_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture* DstTexture, FRHITexture* SrcTexture, FIntRect DstRect = FIntRect(), FIntRect SrcRect = FIntRect(), bool bAlphaPremultiply = false, bool bNoAlphaWrite = false, bool bIsMRCLayer = false, bool bInvertY = false, bool sRGBSource = false, bool bInvertAlpha = false) const;
	void SubmitGPUCommands_RenderThread(FRHICommandListImmediate& RHICmdList);

    FPicoXRHMD* PicoXRHMD;
    FXRSwapChainPtr SwapChain;
    FString RHIString;
    IRendererModule* RendererModule;
};
FPicoXRRenderBridge* CreateRenderBridge_OpenGL(FPicoXRHMD* HMD);
FPicoXRRenderBridge* CreateRenderBridge_Vulkan(FPicoXRHMD* HMD);

