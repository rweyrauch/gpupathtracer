/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#include "ptAmbientLight.h"

AmbientLight* AmbientLight::Create(Stream* pStream)
{
    if (pStream == nullptr )
        return nullptr;

    AmbientLight* light = nullptr;

    int typeId;
    bool ok = pStream->read(&typeId, sizeof(typeId));
    if (!ok) return nullptr;

    switch (typeId)
    {
        case ConstantAmbientTypeId:
            light = new ConstantAmbient();
            break;
        case SkyAmbientTypeId:
            light = new SkyAmbient();
            break;
        default:
            return nullptr;
    }

    ok = light->deserialize(pStream);
    if (!ok)
    {
        delete light;
        light = nullptr;
    }
    return light;
}
