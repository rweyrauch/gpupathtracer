cmake_minimum_required(VERSION 3.7)

project(gpupathtracer)

find_package(CUDA REQUIRED)
find_package(OpenMP)

set(CMAKE_CXX_STANDARD 11)

if (OpenMP_FOUND)
    list(APPEND CMAKE_CXX_FLAGS ${OpenMP_CXX_FLAGS})
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)
set(GPU_SOURCE_FILES
        include/ptAABB.h
        include/ptAmbientLight.h
        include/ptBVH.h
        include/ptCamera.h
        include/ptHitable.h
        include/ptHitableList.h
        include/ptMaterial.h
        include/ptMath.h
        include/ptMedium.h
        include/ptONB.h
        include/ptPDF.h
        include/ptQuickSort.h
        include/ptRay.h
        include/ptRectangle.h
        include/ptRNG.h
        include/ptSphere.h
        include/ptTexture.h
        include/ptVector2.h
        include/ptVector3.h
        include/ptProgress.h
        src/ptProgress.cpp
        src/cxxopts.hpp
        src/ptNoise.cu
        src/ptBVH.cu
        src/ptQuickSort.cu
        src/ptMain.cu)

set(CUDA_NVCC_FLAGS "-use_fast_math")
set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode arch=compute_52,code=sm_52)
set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode arch=compute_61,code=sm_61)
set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-std=c++11)
SET(CUDA_SEPARABLE_COMPILATION ON)

cuda_add_executable(gpupathtracer ${GPU_SOURCE_FILES})
