/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#include "ptMaterial.h"


Material* Material::Create(Stream* pStream)
{
    if (pStream == nullptr )
        return nullptr;

    Material* material = nullptr;

    int typeId;
    bool ok = pStream->read(&typeId, sizeof(typeId));
    if (!ok) return nullptr;

    switch (typeId)
    {
        case LambertianTypeId:
            material = new Lambertian();
            break;
        case MetalTypeId:
            material = new Metal();
            break;
        case DielectricTypeId:
            material = new Dielectric();
            break;
        case DiffuseLightTypeId:
            material = new DiffuseLight();
            break;
        case IsotropicTypeId:
            material = new Isotropic();
            break;
        default:
            return nullptr;
    }

    ok = material->unserialize(pStream);
    if (!ok)
    {
        delete material;
        material = nullptr;
    }
    return material;
}
