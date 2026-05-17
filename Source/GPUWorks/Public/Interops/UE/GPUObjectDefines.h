#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "GPUObjectDefines.generated.h"

UENUM(BlueprintType)
enum class UGPUAccessType : uint8
{
	READ_ONLY		UMETA(DisplayName = "Read-Only"),
	WRITE_ONLY		UMETA(DisplayName = "Write-Only"),
	READ_WRITE		UMETA(DisplayName = "Read-Write"),
};

UENUM(BlueprintType)
enum class UGPUMemoryStrategy : uint8
{
	COPY_ONCE		UMETA(DisplayName = "Upload-Only"),
	STREAM			UMETA(DisplayName = "Stream"),
	ZERO_COPY		UMETA(DisplayName = "Zero-Copy"),
};

UENUM(BlueprintType)
enum class UGPUImageType : uint8
{
	Texture2D		UMETA(DisplayName = "2D"),
	Texture2DArray	UMETA(DisplayName = "2DArray"),
	Texture3D		UMETA(DisplayName = "3D"),
};

UENUM(BlueprintType)
enum class UGPUImageFormat : uint8
{
	R8				UMETA(DisplayName = "8-bit Red"),
	RGBA8			UMETA(DisplayName = "8-bit RGBA"),

	R16				UMETA(DisplayName = "16F Red"),
	RGBA16			UMETA(DisplayName = "16F RGBA"),

	R32				UMETA(DisplayName = "32F Red"),
	RGBA32			UMETA(DisplayName = "32F RGBA"),
};
