/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_PTSTREAM_H
#define PATHTRACER_PTSTREAM_H

#include <cinttypes>
#include "ptCudaCommon.h"

#define MakeFourCC(ch0, ch1, ch2, ch3) \
   ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) |         \
   ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24 ))

class Stream
{
public:
    COMMON_FUNC Stream();
    COMMON_FUNC Stream(void* pData, size_t size) :
        pBuffer(pData),
        bufferSize(size),
        ownBuffer(false) {}

    COMMON_FUNC ~Stream();

    bool create(size_t size);
    bool close();

    COMMON_FUNC bool write(const void* pData, size_t size);
    COMMON_FUNC bool writeNull();

    COMMON_FUNC bool read(void* pData, size_t size);

    COMMON_FUNC void* data() { return pBuffer; }
    COMMON_FUNC size_t size() const { return bufferSize; }

private:

    void* pBuffer = nullptr;
    size_t bufferSize = 0;
    bool ownBuffer = true;
    size_t writeOffset = 0;
    size_t readOffset = 0;
};

#endif //PATHTRACER_PTSTREAM_H
