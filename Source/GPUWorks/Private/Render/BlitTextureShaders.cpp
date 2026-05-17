#include "BlitTextureShaders.h"

IMPLEMENT_GLOBAL_SHADER(FBlitTextureShadersCS, "/GPUShaders/BlitTexture_CS.usf", "MainCS", SF_Compute);