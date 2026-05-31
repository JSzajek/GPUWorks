#include "Interops/UE/GPUBufferObject.h"
#include "Interops/UE/GPUContextObject.h"

bool UGPUBufferObject::Initialize(UGPUContextObject* contextObject,
                                  int64 sizeBytes,
                                  bool bReadOnly,
                                  bool bWriteOnly)
{
    if (!contextObject || !contextObject->GetContext() || sizeBytes <= 0)
    {
        return false;
    }

    Gpu::BufferDescription desc;
    desc.SizeBytes = static_cast<size_t>(sizeBytes);
    desc.Usage = Gpu::MemoryUsage::Default;

    if (bReadOnly)
    {
        desc.AccessMode = Gpu::Access::ReadOnly;
    }
    else if (bWriteOnly)
    {
        desc.AccessMode = Gpu::Access::WriteOnly;
    }
    else
    {
        desc.AccessMode = Gpu::Access::ReadWrite;
    }

    Buffer = contextObject->GetContext()->CreateBuffer(desc);
    return Buffer != nullptr;
}

bool UGPUBufferObject::IsValidBuffer() const
{
    return Buffer != nullptr;
}

int64 UGPUBufferObject::GetSizeBytes() const
{
    return Buffer ? static_cast<int64>(Buffer->GetSize()) : 0;
}

void UGPUBufferObject::CopyBuffer(UGPUContextObject* contextObject,
                                  UGPUBufferObject* otherBuffer)
{
    if (!Buffer || !contextObject || !contextObject->GetDefaultQueue())
    {
        return;
    }

    Buffer->Copy(*contextObject->GetDefaultQueue(),
                 *otherBuffer->GetBuffer());
}

bool UGPUBufferObject::UploadRaw(UGPUContextObject* contextObject,
                                 const void* bytes,
                                 int64 sizeBytes,
                                 int64 offset)
{
    if (!Buffer || !contextObject || !contextObject->GetDefaultQueue() || offset < 0)
    {
        return false;
    }

    return Buffer->Upload(*contextObject->GetDefaultQueue(),
                          bytes,
                          static_cast<size_t>(sizeBytes),
                          static_cast<size_t>(offset));
}

bool UGPUBufferObject::UploadBytes(UGPUContextObject* contextObject,
                                   const TArray<uint8>& bytes,
                                   int64 offset)
{
    if (!Buffer || !contextObject || !contextObject->GetDefaultQueue() || offset < 0)
    {
        return false;
    }

    return Buffer->Upload(*contextObject->GetDefaultQueue(),
                          bytes.GetData(),
                          static_cast<size_t>(bytes.Num()),
                          static_cast<size_t>(offset));
}

bool UGPUBufferObject::UploadFloatArray(UGPUContextObject* contextObject,
                                        const TArray<float>& values,
                                        int64 offsetBytes)
{
    if (!Buffer || !contextObject || !contextObject->GetDefaultQueue() || offsetBytes < 0)
    {
        return false;
    }

    const size_t byteCount = static_cast<size_t>(values.Num()) * sizeof(float);
    return Buffer->Upload(*contextObject->GetDefaultQueue(),
                          values.GetData(),
                          byteCount,
                          static_cast<size_t>(offsetBytes));
}

bool UGPUBufferObject::DownloadRaw(UGPUContextObject* contextObject,
                                   void* outBytes,
                                   int64 bytesToRead,
                                   int64 offset)
{
    if (!Buffer || !contextObject || !contextObject->GetDefaultQueue() || offset < 0)
    {
        return false;
    }

    const int64 available = static_cast<int64>(Buffer->GetSize()) - offset;
	if (available <= 0)
	{
		return false;
	}

    const int64 readSize = (bytesToRead < 0) ? available : FMath::Min(bytesToRead, available);
    return Buffer->Download(*contextObject->GetDefaultQueue(),
                            outBytes,
                            static_cast<size_t>(readSize),
                            static_cast<size_t>(offset));
}

bool UGPUBufferObject::DownloadBytes(UGPUContextObject* contextObject,
                                     TArray<uint8>& outBytes,
                                     int64 bytesToRead,
                                     int64 offset)
{
    const int64 available = static_cast<int64>(Buffer->GetSize()) - offset;
    if (available <= 0)
    {
        return false;
    }

    const int64 readSize = (bytesToRead < 0) ? available : FMath::Min(bytesToRead, available);

    outBytes.SetNumUninitialized(static_cast<int32>(readSize));
	return DownloadRaw(contextObject, outBytes.GetData(), bytesToRead, offset);
}

bool UGPUBufferObject::DownloadFloatArray(UGPUContextObject* contextObject,
                                          TArray<float>& outValues,
                                          int32 floatCount,
                                          int64 offsetBytes)
{
    const int64 available = static_cast<int64>(Buffer->GetSize()) - offsetBytes;
    if (available <= 0)
    {
        return false;
    }

    const int64 maxFloatCount = available / static_cast<int64>(sizeof(float));
    const int64 resolvedCount = (floatCount < 0) ? maxFloatCount : FMath::Min<int64>(floatCount, maxFloatCount);

    outValues.SetNumUninitialized(static_cast<int32>(resolvedCount));
    return DownloadRaw(contextObject,
                       outValues.GetData(),
                       static_cast<size_t>(resolvedCount * sizeof(float)),
                       static_cast<size_t>(offsetBytes));
}