/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#include "ptCudaCommon.h"
#include <cuda.h>
#include <iostream>
#include <fstream>
#include <cfloat>
#include <vector>
#include "ptAABB.h"
#include "ptRectangle.h"
#include "ptSphere.h"
#include "ptHitableList.h"
#include "ptAmbientLight.h"
#include "ptRay.h"
#include "ptCamera.h"
#include "ptMaterial.h"
#include "ptProgress.h"
#include "cxxopts.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

const int MAX_DEPTH = 25;

#ifdef __CUDA_ARCH__
    __device__ AmbientLight* g_ambientLight = NULL;
    __device__ Camera g_cam;
#else
    AmbientLight* g_ambientLight = NULL;
    Camera g_cam;
#endif

COMMON_FUNC Vector3f color_nr(const Ray<float>& r, Hitable* world, Hitable* lightShape, RNG* rng)
{
    Vector3f accumCol(1, 1, 1);

    Ray<float> currentRay(r);

    for (int depth = 0; depth < MAX_DEPTH; depth++)
    {
        HitRecord rec;
        if (world->hit(r, 0.001f, FLT_MAX, rec))
        {
            ScatterRecord srec;
            auto emitted = rec.material->emitted(r, rec, rec.uv, rec.p);
            if (rec.material->scatter(r, rec, srec, rng))
            {
                if (srec.isSpecular)
                {
                    accumCol *= srec.attenuation;
                    currentRay = srec.specularRay;
                }
                else
                {
                    if (lightShape != nullptr)
                    {
                        HitablePdf plight(lightShape, rec.p);
                        MixturePdf p(&plight, srec.pdf);
                        auto scattered = Ray<float>(rec.p, p.generate(rng), r.time());
                        float pdfValue = p.value(scattered.direction());
                        delete srec.pdf;
                        accumCol *= (emitted + (srec.attenuation * rec.material->scatteringPdf(currentRay, rec, scattered)) / pdfValue);
                        currentRay = scattered;
                    }
                    else
                    {
                        auto scattered = Ray<float>(rec.p, srec.pdf->generate(rng), r.time());
                        float pdfValue = srec.pdf->value(scattered.direction());
                        delete srec.pdf;
                        accumCol *= (emitted + (srec.attenuation * rec.material->scatteringPdf(currentRay, rec, scattered)) / pdfValue);
                        currentRay = scattered;
                    }
                }
            }
            else
            {
                accumCol *= emitted;
                break;
            }
        }
        else
        {
            accumCol *= g_ambientLight->emitted(r);
            break;
        }
    }
    return accumCol;
}

__global__ void render_kernel(float3* pOutImage, Hitable** world, Hitable** lightShape, int nx, int ny, int ns)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= nx || y >= ny) return;

    unsigned int i = (ny - y - 1) * nx + x; // index of current pixel (calculated using thread index)

    unsigned int seed0 = x;  // seeds for random number generator
    unsigned int seed1 = y;
    SimpleRng rng(seed0, seed1);

    Vector3<float> accumCol(0, 0, 0);
    for (int s = 0; s < ns; s++)
    {
        float u = (x + rng.rand()) / float(nx);
        float v = (y + rng.rand()) / float(ny);
        Ray<float> r = g_cam.getRay(u, v, &rng);
        accumCol += color_nr(r, *world, *lightShape, &rng);
    }
    accumCol /= float(ns);

    pOutImage[i] = make_float3(sqrtf(accumCol[0]), sqrtf(accumCol[1]), sqrtf(accumCol[2]));
}

__global__ void allocate_world_kernel(Hitable** world, Hitable** lightShape, float aspect)
{
    int i = 0;
    Hitable** list = new Hitable*[4];
    list[i++] = new Sphere(Vector3f(0.0f, 0.0f, -1.0f), 0.5f, new Lambertian(new ConstantTexture(Vector3f(0.1, 0.2, 0.5))));
    list[i++] = new Sphere(Vector3f(0.0f, -100.5f, -1.0f), 100.0f, new Lambertian(new ConstantTexture(Vector3f(0.8, 0.8, 0.0))));
    list[i++] = new Sphere(Vector3f(1, 0, -1), 0.5, new Metal(Vector3f(0.8, 0.6, 0.2), 0.3));
    list[i++] = new Sphere(Vector3f(-1, 0, -1), 0.5, new Dielectric(1.5));

    *world = new HitableList(i, list);

    g_cam = Camera(Vector3f(-2, 2, 1), Vector3f(0, 0, -1), Vector3f(0, 1, 0), 90, aspect, 0.0f, 10.0f);

    g_ambientLight = new SkyAmbient();

    *lightShape = NULL;
}

__global__ void cornell_box_kernel(Hitable **world, Hitable** lightShape, float aspect)
{
    int i = 0;
    Hitable **list = new Hitable*[8];
    Material *red = new Lambertian( new ConstantTexture(Vector3f(0.65f, 0.05f, 0.05f)) );
    Material *white = new Lambertian( new ConstantTexture(Vector3f(0.73, 0.73, 0.73)) );
    Material *green = new Lambertian( new ConstantTexture(Vector3f(0.12, 0.45, 0.15)) );
    Material *light = new DiffuseLight( new ConstantTexture(Vector3f(15, 15, 15)) );
    Material* aluminum = new Metal(Vector3f(0.8, 0.85, 0.88), 0.0);

    list[i++] = new FlipNormals(new YZRectangle(0, 555, 0, 555, 555, green));
    list[i++] = new YZRectangle(0, 555, 0, 555, 0, red);
    list[i++] = new FlipNormals(new XZRectangle(213, 343, 227, 332, 554, light));
    list[i++] = new FlipNormals(new XZRectangle(0, 555, 0, 555, 555, white));
    list[i++] = new XZRectangle(0, 555, 0, 555, 0, white);
    list[i++] = new FlipNormals(new XYRectangle(0, 555, 0, 555, 555, white));

    list[i++] = new Translate(new RotateY(new Box(Vector3f(0, 0, 0), Vector3f(165, 165, 165), white), -18), Vector3f(130, 0, 65));
    list[i++] = new Translate(new RotateY(new Box(Vector3f(0, 0, 0), Vector3f(165, 330, 165), aluminum), 15), Vector3f(265, 0, 295));

    *world = new HitableList(i, list);

    const Vector3f lookFrom(278, 278, -800);
    const Vector3f lookAt(278, 278, 0);
    const double dist_to_focus = 10.0;
    const double aperture = 0.0;
    g_cam = Camera(lookFrom, lookAt, Vector3f(0, 1, 0), 40, aspect, aperture, dist_to_focus);

    g_ambientLight = new ConstantAmbient();

    *lightShape = new XZRectangle(213, 343, 227, 332, 554, NULL);
}

void writeImage(const std::string& outFile, const Vector3f* outImage, int nx, int ny)
{
    auto extStart = outFile.rfind('.');
    if (extStart != std::string::npos)
    {
        extStart++;
        std::string ext = outFile.substr(extStart);
        if (ext == "ppm")
        {
            std::ofstream of(outFile.c_str());
            if (of.is_open())
            {
                of << "P3\n" << nx << " " << ny << "\n255\n";

                for (int i = 0; i < nx * ny; i++)
                {
                    Vector3f col = outImage[i];

                    int ir = int(255.99 * col[0]);
                    int ig = int(255.99 * col[1]);
                    int ib = int(255.99 * col[2]);

                    of << ir << " " << ig << " " << ib << "\n";
                }
            }
            of.close();
        }
        else if (ext == "hdr")
        {
            stbi_write_hdr(outFile.c_str(), nx, ny, 3, (const float*)outImage);
        }
        else
        {
            unsigned char* outBytes = new unsigned char[nx * ny * 3];
            unsigned char* currentOut = outBytes;
            for (int i = 0; i < nx * ny; i++)
            {
                const Vector3f& col = outImage[i];
                int ir = Clamp(int(255.99 * col[0]), 0, 255);
                int ig = Clamp(int(255.99 * col[1]), 0, 255);
                int ib = Clamp(int(255.99 * col[2]), 0, 255);
                *currentOut++ = (unsigned char)ir;
                *currentOut++ = (unsigned char)ig;
                *currentOut++ = (unsigned char)ib;
            }
            if (ext == "png")
                stbi_write_png(outFile.c_str(), nx, ny, 3, outBytes, nx * 3);
            else if (ext == "tga")
                stbi_write_tga(outFile.c_str(), nx, ny, 3, outBytes);
            else if (ext == "bmp")
                stbi_write_bmp(outFile.c_str(), nx, ny, 3, outBytes);

            delete[] outBytes;
        }
    }
}

inline Vector3f deNan(const Vector3f& c)
{
    Vector3f temp = c;
    if (!(temp[0] == temp[0])) temp[0] = 0;
    if (!(temp[1] == temp[1])) temp[1] = 0;
    if (!(temp[2] == temp[2])) temp[2] = 0;
    return temp;
}

void renderLine(int line, Vector3f* outLine, int nx, int ny, int ns, Camera& cam, Hitable* world, Hitable* lightShapes, RNG* rng)
{
    for (int x = 0; x < nx; x++)
    {
        Vector3f col(0, 0, 0);
        for (int s = 0; s<ns; s++)
        {
            auto u = (x + rng->rand()) / float(nx);
            auto v = (line + rng->rand()) / float(ny);
            Rayf r = cam.getRay(u, v, rng);
            col += deNan(color_nr(r, world, lightShapes, rng));
        }
        col /= double(ns);
        outLine[x] = Vector3f(sqrt(std::max(0.0f, col[0])), sqrt(std::max(0.0f, col[1])), sqrt(std::max(0.0f, col[2])));
    }
}

Hitable* cornellBox(float aspect, Camera& camera, std::vector<Hitable*>& lights)
{
    const Vector3f lookFrom(278, 278, -800);
    const Vector3f lookAt(278, 278, 0);
    const float dist_to_focus = 10.0;
    const float aperture = 0.0;
    camera = Camera(lookFrom, lookAt, Vector3f(0, 1, 0), 40, aspect, aperture, dist_to_focus);

    std::vector<Hitable*> list;

    Material* red = new Lambertian(new ConstantTexture(Vector3f(0.65, 0.05, 0.05)));
    Material* white = new Lambertian(new ConstantTexture(Vector3f(0.73, 0.73, 0.73)));
    Material* green = new Lambertian(new ConstantTexture(Vector3f(0.12, 0.45, 0.15)));
    Material* light = new DiffuseLight(new ConstantTexture(Vector3f(15, 15, 15)));
    Material* aluminum = new Metal(Vector3f(0.8, 0.85, 0.88), 0.0);
    Material* glass = new Dielectric(1.5);

    list.push_back(new FlipNormals(new YZRectangle(0, 555, 0, 555, 555, green)));
    list.push_back(new YZRectangle(0, 555, 0, 555, 0, red));
    list.push_back(new FlipNormals(new XZRectangle(213, 343, 227, 332, 554, light)));
    list.push_back(new FlipNormals(new XZRectangle(0, 555, 0, 555, 555, white)));
    list.push_back(new XZRectangle(0, 555, 0, 555, 0, white));
    list.push_back(new FlipNormals(new XYRectangle(0, 555, 0, 555, 555, white)));

    list.push_back(new Translate(new RotateY(new Box(Vector3f(0, 0, 0), Vector3f(165, 165, 165), white), -18), Vector3f(130, 0, 65)));
    list.push_back(new Translate(new RotateY(new Box(Vector3f(0, 0, 0), Vector3f(165, 330, 165), aluminum), 15), Vector3f(265, 0, 295)));
    //list.push_back(new Translate(new Box(Vector3(0, 0, 0), Vector3(165, 330, 165), aluminum), Vector3(265, 0, 295)));
    //list.push_back(new Sphere(Vector3(190, 90, 190), 90, glass));

    //list.push_back(new Translate(new Box(Vector3(0, 0, 0), Vector3(165, 165, 165), white), Vector3(130, 0, 65)));
    //list.push_back(new Translate(new Box(Vector3(0, 0, 0), Vector3(165, 330, 165), white), Vector3(265, 0, 295)));

    //Hitable* b1 = new Translate(new RotateY(new Box(Vector3(0, 0, 0), Vector3(165, 165, 165), white), -18), Vector3(130, 0, 65));
    //Hitable* b2 = new Translate(new RotateY(new Box(Vector3(0, 0, 0), Vector3(165, 330, 165), white), 15), Vector3(265, 0, 295));

    //list.push_back(new ConstantMedium(b1, 0.01, new ConstantTexture(Vector3(1, 1, 1))));
    //list.push_back(new ConstantMedium(b2, 0.01, new ConstantTexture(Vector3(0, 0, 0))));

    lights.push_back(new XZRectangle(213, 343, 227, 332, 554, nullptr));

    g_ambientLight = new SkyAmbient();

    return new HitableList(list.size(), list.data());
}

int main(int argc, char** argv)
{
    cxxopts::Options options("pathtracer", "Implementation of Peter Shirley's Raytracing in One Weekend book series.");
    options.add_options()
        ("q,quick", "Quick render.")
        ("c,cpu", "Render on CPU.")
        ("w,width", "Output width.", cxxopts::value<int>())
        ("h,height", "Output height.", cxxopts::value<int>())
        ("n,numsamples", "Number of sample rays per pixel.", cxxopts::value<int>())
        ("t,threads", "Number of render threads.", cxxopts::value<int>())
        ("f,file", "Output filename.", cxxopts::value<std::string>());

    options.parse(argc, argv);

    bool quick = options.count("quick") > 0;
    int ns = 100;
    int nx = 128 * 4;
    int ny = 128 * 4;
    bool cpu = options.count("cpu") > 0;
    int numThreads = 1;

    std::string outFile("outputImage.ppm");

    if (options.count("width"))
        nx = options["width"].as<int>();
    if (options.count("height"))
        ny = options["height"].as<int>();
    if (options.count("numsamples"))
        ns = options["numsamples"].as<int>();
    if (options.count("file"))
        outFile = options["file"].as<std::string>();
    if (options.count("threads"))
        numThreads = options["numthreads"].as<int>();

    if (quick)
    {
        nx /= 8;
        ny /= 8;
        ns /= 16;
    }

    const float aspect = float(nx)/float(ny);

    if (!cpu)
    {
        float3* pOutImage = NULL;
        cudaMalloc(&pOutImage, nx * ny * sizeof(float3));

        Hitable** world = NULL;
        cudaMalloc(&world, sizeof(Hitable**));

        Hitable** lightShape = NULL;
        cudaMalloc(&lightShape, sizeof(Hitable**));

        std::cerr << "Allocating world...";
        allocate_world_kernel<<<1, 1>>>(world, lightShape, aspect);
        cudaError_t err = cudaDeviceSynchronize();
        std::cerr << "done" << std::endl;
        if (err != cudaSuccess)
        {
            std::cerr << "Failed to allocate world in GPU memory.  Error: " << cudaGetErrorName(err) << " Desc: " << cudaGetErrorString(err) << std::endl;
            return EXIT_FAILURE;
        }

        dim3 block(8, 8, 1);
        dim3 grid(IDIVUP(nx, block.x), IDIVUP(ny, block.y), 1);
        std::cerr << "Rendering world...";
        render_kernel<<<grid, block>>>(pOutImage, world, lightShape, nx, ny, ns);
        err = cudaDeviceSynchronize();
        std::cerr << "done" << std::endl;
        if (err != cudaSuccess)
        {
            std::cerr << "Failed to render on GPU.  Error: " << cudaGetErrorName(err) << " Desc: " << cudaGetErrorString(err) << std::endl;
            return EXIT_FAILURE;
        }

        Vector3f* pTemp = new Vector3f[nx * ny];
        cudaMemcpy(pTemp, pOutImage, nx*ny*sizeof(float3), cudaMemcpyDeviceToHost);
        cudaFree(pOutImage);

        writeImage(outFile, pTemp, nx, ny);
        delete[] pTemp;
    }
    else
    {
        Camera cam;
        const double aspect = double(nx)/double(ny);
        std::vector<Hitable*> lights;
        Hitable* world = cornellBox(aspect, cam, lights);// cornellBox(); // simpleLight(); //randomScene(); //
        HitableList* lightShapes = nullptr;
        if (!lights.empty())
            lightShapes = new HitableList(lights.size(), lights.data());
        //Hitable* lightShape = new XZRectangle(213, 343, 227, 332, 554, nullptr);
        //Hitable* glassSphere = new Sphere(Vector3(190, 90, 190), 90, nullptr);
        //lights.push_back(lightShape);
        //lights.push_back(glassSphere);
        //HitableList* lightShapes = new HitableList(lights);

        Vector3f* outImage = new Vector3f[nx * ny];

        Progress progress(nx*ny, "PathTracers");

        DRandRng* rng = new DRandRng(42);

        int index = 0;
        #pragma omp parallel for if(numThreads)
        for (int j = 0; j < ny; j++)
        {
            Vector3f* outLine = outImage + (nx * j);
            const int line = ny - j - 1;
            renderLine(line, outLine, nx, ny, ns, cam, world, lightShapes, rng);

            #pragma omp critical(progress)
            {
                progress.update(nx);
            }
        }

        progress.completed();

        writeImage(outFile, outImage, nx, ny);

        delete rng;
        delete[] outImage;
    }

    std::cerr << "Done." << std::endl;

    return EXIT_SUCCESS;
}
