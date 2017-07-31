/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#include "ptTexture.h"

Texture* Texture::Create(Stream* pStream)
{
    if (pStream == nullptr )
        return nullptr;

    Texture* texture = nullptr;

    int typeId;
    bool ok = pStream->read(&typeId, sizeof(typeId));
    switch (typeId)
    {
        case ConstantTextureTypeId:
            break;
        case CheckerTextureTypeId:
            break;
        case NoiseTextureTypeId:
            break;
        case ImageTextureTypeId:
            break;
        default:
            break;
    }
    return texture;
}
