#include "Render/UTextureUtils.h"

#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"

#include "Render/BlitTextureShaders.h"
#include "TextureResource.h"
#include "RHIStaticStates.h"
#include "Async/Async.h"

namespace UTextureUtils
{
	bool BlitTextureToRenderTarget(const TObjectPtr<UTexture2D> source, 
								   TObjectPtr<UTextureRenderTarget2D>& output, 
								   FIntPoint subRect,
								   const std::function<void()>& onWriteComplete)
	{
		if (!source || !output)
			return false;
		
		ENQUEUE_RENDER_COMMAND(CopyTextureCommand)([source, output, subRect, onWriteComplete](FRHICommandListImmediate& RHICmdList)
		{
			FBlitTextureShadersCS::ECopyChannelFormat format = FBlitTextureShadersCS::ECopyChannelFormat::MAX;

			switch (source->GetPixelFormat())
			{
				case EPixelFormat::PF_R8:
				case EPixelFormat::PF_R16F:
				case EPixelFormat::PF_R32_FLOAT:
					format = FBlitTextureShadersCS::ECopyChannelFormat::Float;
					break;
				case EPixelFormat::PF_R8G8:
				case EPixelFormat::PF_G16R16F:
				case EPixelFormat::PF_G32R32F:
					format = FBlitTextureShadersCS::ECopyChannelFormat::Float2;
					break;
				case EPixelFormat::PF_R8G8B8A8:
				case EPixelFormat::PF_FloatRGBA:
				case EPixelFormat::PF_A32B32G32R32F:
					format = FBlitTextureShadersCS::ECopyChannelFormat::Float4;
					break;

				case EPixelFormat::PF_R32_UINT:
					format = FBlitTextureShadersCS::ECopyChannelFormat::UInt;
					break;
				case EPixelFormat::PF_R32G32_UINT:
					format = FBlitTextureShadersCS::ECopyChannelFormat::UInt2;
					break;
				case EPixelFormat::PF_R32G32B32A32_UINT:
					format = FBlitTextureShadersCS::ECopyChannelFormat::UInt4;
					break;

				case EPixelFormat::PF_R32_SINT:
					format = FBlitTextureShadersCS::ECopyChannelFormat::SInt;
					break;
			}

			if (format == FBlitTextureShadersCS::ECopyChannelFormat::MAX)
				return;


			FRDGBuilder GraphBuilder(RHICmdList);

			TRefCountPtr<IPooledRenderTarget> sourceRT = CreateRenderTarget(source->GetResource()->TextureRHI, TEXT("Source"));
			FRDGTextureRef source_rdg = GraphBuilder.RegisterExternalTexture(sourceRT);

			TRefCountPtr<IPooledRenderTarget> outputRT = CreateRenderTarget(output->GetRenderTargetResource()->GetRenderTargetTexture(), TEXT("Output"));
			FRDGTextureRef output_rdg = GraphBuilder.RegisterExternalTexture(outputRT);

			FBlitTextureShadersCS::FParameters* parameters = GraphBuilder.AllocParameters<FBlitTextureShadersCS::FParameters>();
			parameters->Input = source_rdg;
			parameters->Sampler = TStaticSamplerState<SF_Point, AM_Wrap, AM_Wrap>::GetRHI();
			parameters->Output = GraphBuilder.CreateUAV(output_rdg);

			FBlitTextureShadersCS::FPermutationDomain Permutation;
			Permutation.Set<FBlitTextureShadersCS::FChannelFormatPermutation>(format);

			const FGlobalShaderMap* globalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
			auto blitShader = globalShaderMap->GetShader<FBlitTextureShadersCS>(Permutation);


			FIntPoint Size = subRect != FIntPoint::ZeroValue ? subRect : output->GetRenderTargetResource()->GetSizeXY();

			FIntVector GroupCount = FComputeShaderUtils::GetGroupCount(Size, FIntPoint(16, 16));

			FComputeShaderUtils::AddPass(GraphBuilder,
										 RDG_EVENT_NAME("BlitTextureToRT_CS"),
										 blitShader,
										 parameters,
										 GroupCount);

			GraphBuilder.AddPostExecuteCallback([onWriteComplete]()
			{
				AsyncTask(ENamedThreads::GameThread, [onWriteComplete]
				{
					if (onWriteComplete)
						onWriteComplete();
				});
			});

			GraphBuilder.Execute();
		});

		return true;
	}

	bool BlitTextureToRenderTarget_Immediate(const TObjectPtr<UTexture2D> source,
											 TObjectPtr<UTextureRenderTarget2D>& output,
											 FIntPoint subRect)
	{
		FRenderCommandFence resourceFence;
		resourceFence.BeginFence();
		if (!BlitTextureToRenderTarget(source, output, subRect))
		{
			resourceFence.Wait(true);
			return false;
		}
		resourceFence.Wait(true);
		return true;
	}
};