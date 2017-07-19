/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#include "ptQuickSort.h"

COMMON_FUNC void swap(Hitable** a, Hitable** b)
{
    Hitable* temp = *a;
    *a = *b;
    *b = temp;
}

COMMON_FUNC int partition(Hitable** list, int l, int h, int index)
{
    auto x = list[h];
    int i = (l - 1);

    for (int j = l; j <= h- 1; j++)
    {
        AABB<float> boxLeft, boxRight;
        list[j]->bounds(0, 0, boxLeft);
        x->bounds(0, 0, boxRight);

        if (boxLeft.min()[index] < boxRight.min()[index])
        {
            i++;
            swap(&list[i], &list[j]);
        }
    }
    swap(&list[i + 1], &list[h]);
    return (i + 1);
}

COMMON_FUNC void quickSort(Hitable** list, int l, int h, int index)
{
    // Create an auxiliary stack
    int* stack = new int[h - l + 1];

    // initialize top of stack
    int top = -1;

    // push initial values of l and h to stack
    stack[++top] = l;
    stack[++top] = h;

    // Keep popping from stack while is not empty
    while (top >= 0)
    {
        // Pop h and l
        h = stack[top--];
        l = stack[top--];

        // Set pivot element at its correct position
        // in sorted array
        int p = partition(list, l, h, index);

        // If there are elements on left side of pivot,
        // then push left side to stack
        if (p-1 > l)
        {
            stack[++top] = l;
            stack[++top] = p - 1;
        }

        // If there are elements on right side of pivot,
        // then push right side to stack
        if (p+1 < h)
        {
            stack[++top] = p + 1;
            stack[++top] = h;
        }
    }

    delete[] stack;
}