//Unreal® Engine, Copyright 1998 – 2022, Epic Games, Inc. All rights reserved.
#include "PXR_HMDRenderBridge.h"
#include "PXR_HMD.h"
#include "OpenGLDrv.h"

class FPicoXRRenderBridge_OpenGL : public FPicoXRRenderBridge
{
public:
	FPicoXRRenderBridge_OpenGL(FPicoXRHMD* HMD) :FPicoXRRenderBridge(HMD)
	{
		RHIString = HMD->GetRHIString();
	}
#if ENGINE_MINOR_VERSION>25
	virtual FTextureRHIRef CreateTexture_RenderThread(ERHIResourceType ResourceType, uint64 InTexture, uint8 Format, uint32 SizeX, uint32 SizeY, uint32 NumMips, uint32 NumSamples, ETextureCreateFlags TargetableTextureFlags, uint32 MSAAValue)override
#else
	virtual FTextureRHIRef CreateTexture_RenderThread(ERHIResourceType ResourceType, uint64 InTexture, uint8 Format, uint32 SizeX, uint32 SizeY, uint32 NumMips, uint32 NumSamples, uint32 TargetableTextureFlags, uint32 MSAAValue)override
#endif
	{
		FOpenGLDynamicRHI* DynamicRHI = static_cast<FOpenGLDynamicRHI*>(GDynamicRHI);
		switch (ResourceType)
		{
		case RRT_Texture2D:
			return DynamicRHI->RHICreateTexture2DFromResource((EPixelFormat)Format, SizeX, SizeY, NumMips, NumSamples, MSAAValue, FClearValueBinding::Black, (GLuint)InTexture, TargetableTextureFlags).GetReference();

		case RRT_Texture2DArray:
			return DynamicRHI->RHICreateTexture2DArrayFromResource((EPixelFormat)Format, SizeX, SizeY, 2, NumMips, NumSamples, MSAAValue, FClearValueBinding::Black, (GLuint)InTexture, TargetableTextureFlags).GetReference();

		case RRT_TextureCube:
			return DynamicRHI->RHICreateTextureCubeFromResource((EPixelFormat)Format, SizeX, false, 1, NumMips, NumSamples, MSAAValue, FClearValueBinding::Black, (GLuint)InTexture, TargetableTextureFlags).GetReference();

		default:
			return nullptr;
		}
	}
};

FPicoXRRenderBridge* CreateRenderBridge_OpenGL(FPicoXRHMD* HMD)
{
	return new FPicoXRRenderBridge_OpenGL(HMD);
}
