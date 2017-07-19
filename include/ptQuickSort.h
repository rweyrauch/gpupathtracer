/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_QUICKSORT_H
#define PATHTRACER_QUICKSORT_H

#include "ptCudaCommon.h"
#include "ptHitable.h"

COMMON_FUNC void quickSort(Hitable** list, int l, int h, int index);

#endif //PATHTRACER_QUICKSORT_H
