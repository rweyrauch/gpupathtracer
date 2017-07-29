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

#include "ptCudaCommon.h"

class Stream
{
public:
    COMMON_FUNC Stream();
    virtual ~Stream();

    bool create(size_t size);
    bool close();

    COMMON_FUNC bool write(const void* pData, size_t size);
    COMMON_FUNC bool read(void* pData, size_t size);

private:

    void* pBuffer = nullptr;
    size_t bufferSize = 0;
    size_t writeOffset = 0;
    size_t readOffset = 0;
};

#endif //PATHTRACER_PTSTREAM_H
