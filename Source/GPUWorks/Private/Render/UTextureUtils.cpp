#include "Render/UTextureUtils.h"

#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"

#include "Render/BlitTextureShaders.h"
#include "TextureResource.h"
#include "RHIStaticStates.h"
#include "Async/Async.h"

namespace RenderUtils
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

	template<typename V, typename C>
	void GenerateMip(std::vector<Mip>& output,
				     V* const src,
				     size_t srcWidth, 
				     size_t srcHeight,
				     size_t srcLayers,
				     uint8_t srcChannels)
	{
		output.clear();

		size_t currentWidth = srcWidth;
		size_t currentHeight = srcHeight;
		V* currentData = src;

		// Initial Mip
		output.push_back(Mip{ currentWidth,
						      currentHeight,
						      srcLayers,
						      srcChannels,
						      src });

		while (currentWidth > 1 || currentHeight > 1)
		{
			size_t nextWidth	= std::max(static_cast<size_t>(1), static_cast<size_t>(std::roundf(currentWidth * 0.5f)));
			size_t nextHeight	= std::max(static_cast<size_t>(1), static_cast<size_t>(std::roundf(currentHeight * 0.5f)));
			V* nextData = new V[nextWidth * nextHeight * srcChannels * srcLayers];

			for (size_t layer = 0; layer < srcLayers; ++layer)
			{
				size_t currentLayerOffset = layer * currentWidth * currentHeight * srcChannels;

				for (size_t y = 0; y < nextHeight; ++y)
				{
					for (size_t x = 0; x < nextWidth; ++x)
					{
						std::vector<C> channelPixel(srcChannels, 0);

						float count = 0;
						for (size_t dy = 0; dy < 2; ++dy)
						{
							for (size_t dx = 0; dx < 2; ++dx)
							{
								size_t srcX = std::min(currentWidth - 1, x * 2 + dx);
								size_t srcY = std::min(currentHeight - 1, y * 2 + dy);
								size_t srcIndex = ((srcY * currentWidth + srcX) * srcChannels) + currentLayerOffset;

								for (int c = 0; c < srcChannels; ++c)
								{
									channelPixel[c] += currentData[srcIndex + c];
								}

								++count;
							}
						}

						size_t nextLayerOffset = layer * nextWidth * nextHeight * srcChannels;
						const size_t dstIndex = ((y * nextWidth + x) * srcChannels) + nextLayerOffset;
						for (int c = 0; c < srcChannels; ++c)
						{
							nextData[dstIndex + c] = static_cast<V>(std::floor(channelPixel[c] / count));
						}
					}
				}
			}

			output.push_back({ nextWidth, 
							   nextHeight, 
							   srcLayers, 
							   srcChannels, 
							   nextData });

			currentData = std::move(nextData);
			currentWidth = nextWidth;
			currentHeight = nextHeight;
		}
	}

	void GenerateMipsUInt8(std::vector<Mip>& output,
						   uint8_t* const src,
						   size_t srcWidth, 
						   size_t srcHeight,
						   size_t srcLayers,
						   uint8_t srcChannels)
	{
		GenerateMip<uint8_t, size_t>(output,
									 src,
									 srcWidth,
									 srcHeight,
									 srcLayers,
									 srcChannels);
	}

	void GenerateMipsUInt32(std::vector<Mip>& output, 
							uint32_t* const src, 
							size_t srcWidth, 
							size_t srcHeight, 
							size_t srcLayers, 
							uint8_t srcChannels)
	{
		GenerateMip<uint32_t, size_t>(output,
									  src,
									  srcWidth,
									  srcHeight,
									  srcLayers,
									  srcChannels);
	}

	void GenerateMipsInt32(std::vector<Mip>& output, 
						   int32_t* const src, 
						   size_t srcWidth, 
						   size_t srcHeight, 
						   size_t srcLayers, 
						   uint8_t srcChannels)
	{
		GenerateMip<int32_t, size_t>(output,
									 src,
									 srcWidth,
									 srcHeight,
									 srcLayers,
									 srcChannels);
	}

	void GenerateMipsFloat16(std::vector<Mip>& output,
							 FFloat16* const src,
						     size_t srcWidth, 
						     size_t srcHeight, 
							 size_t srcLayers,
						     uint8_t srcChannels)
	{
		GenerateMip<FFloat16, float>(output,
									 src,
									 srcWidth,
									 srcHeight,
									 srcLayers,
									 srcChannels);
	}

	void GenerateMipsFloat(std::vector<Mip>& output,
						   float* const src,
						   size_t srcWidth, 
						   size_t srcHeight, 
						   size_t srcLayers,
						   uint8_t srcChannels)
	{
		GenerateMip<float, float>(output,
								  src,
								  srcWidth,
								  srcHeight,
								  srcLayers,
								  srcChannels);
	}
};