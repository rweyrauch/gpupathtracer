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
        include/ptCudaCommon.h
        include/ptHitable.h
        include/ptHitableList.h
        include/ptMaterial.h
        include/ptMath.h
        include/ptMedium.h
        include/ptNoise.h
        include/ptONB.h
        include/ptPDF.h
        include/ptQuickSort.h
        include/ptRay.h
        include/ptRectangle.h
        include/ptRNG.h
        include/ptSphere.h
        include/ptTexture.h
        include/ptTriangle.h
        include/ptVector2.h
        include/ptVector3.h
        include/ptProgress.h
        include/ptStream.h
        src/ptProgress.cpp
        src/ptStream.cu
        src/stb_image.h
        src/stb_image_write.h
        src/cxxopts.hpp
        src/ptAmbientLight.cu
        src/ptNoise.cu
        src/ptBVH.cu
        src/ptHitable.cu
        src/ptHitableList.cu
        src/ptMaterial.cu
        src/ptQuickSort.cu
        src/ptRectangle.cu
        src/ptSphere.cu
        src/ptTexture.cu
        src/ptTriangle.cu
        src/ptMain.cu)

set(CUDA_NVCC_FLAGS "-use_fast_math")
set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode arch=compute_52,code=sm_52)
set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode arch=compute_61,code=sm_61)
set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-std=c++11)
set(CUDA_SEPARABLE_COMPILATION ON)

#set(CUDA_NVCC_FLAGS_DEBUG "-G")

cuda_add_executable(gpupathtracer ${GPU_SOURCE_FILES})
