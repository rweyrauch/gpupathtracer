/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#include <cuda_runtime.h>
#include <cstring>
#include <cstdint>
#include "ptStream.h"

Stream::Stream()
{
}

Stream::~Stream()
{
#ifndef __CUDA_ARCH__
    if (ownBuffer)
        close();
#endif
}

bool Stream::create(size_t size)
{
    if (pBuffer != nullptr)
        return false;

    cudaError_t err = cudaMallocManaged(&pBuffer, size);
    if (err != cudaSuccess)
    {
        return false;
    }

    bufferSize = size;

    return true;
}

bool Stream::close()
{
    if (pBuffer != nullptr)
    {
        cudaError_t err = cudaFree(pBuffer);
        pBuffer = nullptr;
        bufferSize = 0;

        if (err != cudaSuccess) return false;
    }
    return true;
}

bool Stream::write(const void* pData, size_t size)
{
    if (pBuffer == nullptr)
        return false;

    if (writeOffset + size >= bufferSize)
        return false;

    uint8_t* pDest = (uint8_t*)pBuffer + writeOffset;
    memcpy(pDest, pData, size);
    writeOffset += size;

    return true;
}

bool Stream::writeNull()
{
    int nullId = -1;
    return write(&nullId, sizeof(nullId));
}

bool Stream::read(void* pData, size_t size)
{
    if (pBuffer == nullptr)
        return false;

    if (readOffset + size >= bufferSize)
        return false;

    const uint8_t* pSrc = (uint8_t*)pBuffer + readOffset;
    memcpy(pData, pSrc, size);
    readOffset += size;

    return true;
}
