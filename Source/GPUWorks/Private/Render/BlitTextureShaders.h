#pragma once

#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "ShaderParameterMacros.h"

/// <summary>
/// Blit texture shader that can copy from one texture to another with different channel formats.
/// </summary>
class FBlitTextureShadersCS : public FGlobalShader
{
public:
	/// <summary>
	/// Enum to specify the channel format for the blit texture shader.
	/// This will determine how the shader reads from the input texture and writes to the output texture.
	/// </summary>
	enum class ECopyChannelFormat : uint8
	{
		Float,
		Float2,
		Float4,

		UInt,
		UInt2,
		UInt4,

		SInt,

		MAX
	};
public:
	DECLARE_GLOBAL_SHADER(FBlitTextureShadersCS);
	
	SHADER_USE_PARAMETER_STRUCT(FBlitTextureShadersCS, FGlobalShader);

	class FChannelFormatPermutation : SHADER_PERMUTATION_ENUM_CLASS("CHANNEL_FORMAT", ECopyChannelFormat);
	using FPermutationDomain = TShaderPermutationDomain<FChannelFormatPermutation>;

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FIntPoint, Resolution)
		SHADER_PARAMETER_SAMPLER(SamplerState, Sampler)

		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, Input)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, Output)
	END_SHADER_PARAMETER_STRUCT()
public:
	/// <summary>
	/// Whether the shader permutation should be compiled for the given parameters.
	/// </summary>
	/// <param name="Parameters">The parameters for the shader permutation.</param>
	/// <returns>True if the permutation should be compiled, false otherwise.</returns>
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM6);
	}
};