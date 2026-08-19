#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "GPUObjectDefines.generated.h"

/// <summary>
/// Enum representing the type of GPU access for a resource.
/// This determines how the resource can be accessed and used in GPU operations.
/// </summary>
UENUM(BlueprintType)
enum class UGPUAccessType : uint8
{
	READ_ONLY		UMETA(DisplayName = "Read-Only"),
	WRITE_ONLY		UMETA(DisplayName = "Write-Only"),
	READ_WRITE		UMETA(DisplayName = "Read-Write"),
};

/// <summary>
/// Enum representing the memory strategy for GPU resources.
/// This determines how the resource is allocated and accessed in GPU memory.
/// </summary>
UENUM(BlueprintType)
enum class UGPUMemoryStrategy : uint8
{
	COPY_ONCE		UMETA(DisplayName = "Upload-Only"),
	STREAM			UMETA(DisplayName = "Stream"),
	ZERO_COPY		UMETA(DisplayName = "Zero-Copy"),
};

/// <summary>
/// Enum representing the type of GPU image resource.
/// This determines the dimensionality and structure of the image data,
/// which affects how it can be used in GPU operations.
/// </summary>
UENUM(BlueprintType)
enum class UGPUImageType : uint8
{
	Texture2D		UMETA(DisplayName = "2D"),
	Texture2DArray	UMETA(DisplayName = "2DArray"),
	Texture3D		UMETA(DisplayName = "3D"),
};

/// <summary>
/// Enum representing the pixel format of a GPU image resource.
/// This determines how the pixel data is stored and interpreted in GPU memory.
/// </summary>
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
