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
            texture = new ConstantTexture();
            break;
        case CheckerTextureTypeId:
            texture = new CheckerTexture();
            break;
        case NoiseTextureTypeId:
            texture = new NoiseTexture();
            break;
        case ImageTextureTypeId:
            texture = new ImageTexture();
            break;
        default:
            return nullptr;
    }

    ok = texture->unserialize(pStream);
    if (!ok)
    {
        delete texture;
        texture = nullptr;
    }
    return texture;
}
