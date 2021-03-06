/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#include "ptCudaCommon.h"
#include <cuda_runtime_api.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include "ptAABB.h"
#include "ptRectangle.h"
#include "ptRNG.h"
#include "ptSphere.h"
#include "ptHitableList.h"
#include "ptAmbientLight.h"
#include "ptRay.h"
#include "ptBVH.h"
#include "ptCamera.h"
#include "ptMaterial.h"
#include "ptMedium.h"
#include "ptProgress.h"
#include "cxxopts.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef __CUDA_ARCH__
    __device__ AmbientLight* g_ambientLight = nullptr;
    __device__ Camera* g_cam = nullptr;
#else
    AmbientLight* g_ambientLight = nullptr;
    Camera* g_cam = nullptr;
#endif

COMMON_FUNC Vector3f deNan(const Vector3f& c)
{
    Vector3f temp = c;
    if (!(temp[0] == temp[0])) temp[0] = 0;
    if (!(temp[1] == temp[1])) temp[1] = 0;
    if (!(temp[2] == temp[2])) temp[2] = 0;
    return temp;
}

/*
COMMON_FUNC Vector3f color(const Rayf& r, Hitable* world, RNG& rng, int maxDepth)
{
    Vector3f accumCol(1, 1, 1);

    Rayf currentRay(r);

    for (int depth = 0; depth < maxDepth; depth++)
    {
        HitRecord rec;
        if (world->hit(currentRay, 0.001f, FLT_MAX, rec))
        {
            Rayf scattered;
            Vector3f attenuation;
            Vector3f emitted = rec.material->emitted(rec.uv, rec.p);
            if (rec.material->scatter(currentRay, rec, attenuation, scattered, rng))
            {
                accumCol *= (emitted + attenuation);
                currentRay = scattered;
            }
            else
            {
                accumCol *= emitted;
                break;
            }
        }
        else
        {
            if (g_ambientLight)
                accumCol *= g_ambientLight->emitted(currentRay);
            else
                accumCol = Vector3f(0.0f, 0.0f, 0.0f);

            break;
        }
    }
    return accumCol;
}
*/

COMMON_FUNC Vector3f color(const Rayf& r_in, Hitable* world, Hitable* lightShape, RNG& rng, int maxDepth)
{
    Vector3f accumCol(1, 1, 1);

    Rayf currentRay(r_in);

    for (int depth = 0; depth < maxDepth; depth++)
    {
        HitRecord rec;
        if (world->hit(currentRay, 0.001f, FLT_MAX, rec, rng))
        {
            ScatterRecord srec;
            auto emitted = rec.material->emitted(currentRay, rec, rec.uv, rec.p);
            if (rec.material->scatter(currentRay, rec, srec, rng))
            {
                if (srec.isSpecular)
                {
                    accumCol *= srec.attenuation;
                    currentRay = srec.specularRay;
                }
                else
                {
                    CosinePdf pdf(rec.normal);
                    ConstPdf pdf2;
                    if (lightShape != nullptr)
                    {
                        HitablePdf plight(lightShape, rec.p);
                        MixturePdf p(&plight, &pdf);
                        auto scattered = Rayf(rec.p, p.generate(rng), currentRay.time());
                        float pdfValue = p.value(scattered.direction(), rng);
                        accumCol *= (emitted + (srec.attenuation * rec.material->scatteringPdf(currentRay, rec, scattered)) / pdfValue);
                        currentRay = scattered;
                    }
                    else
                    {
                        auto scattered = Rayf(rec.p, srec.cosinePdf ? pdf.generate(rng) : pdf2.generate(rng), currentRay.time());
                        float pdfValue = srec.cosinePdf ? pdf.value(scattered.direction(), rng) : pdf2.value(scattered.direction(), rng);
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
            if (g_ambientLight != nullptr)
                accumCol *= g_ambientLight->emitted(currentRay);
            break;
        }
    }
    return accumCol;
}

COMMON_FUNC Vector3f render_pixel(Hitable** world, Hitable** lightShapes, int x, int y, int nx, int ny, int ns, RNG& rng, int maxDepth)
{
    Vector3f accumCol(0, 0, 0);
    for (int s = 0; s < ns; s++)
    {
        float u = (x + rng.rand()) / float(nx);
        float v = (y + rng.rand()) / float(ny);
        Rayf r = g_cam->getRay(u, v, rng);
        accumCol += deNan(color(r, *world, *lightShapes, rng, maxDepth));
    }
    accumCol /= float(ns);
    accumCol[0] = sqrtf(fmaxf(0.0f, accumCol[0]));
    accumCol[1] = sqrtf(fmaxf(0.0f, accumCol[1]));
    accumCol[2] = sqrtf(fmaxf(0.0f, accumCol[2]));

    return accumCol;
}

__global__ void render_kernel(float3* pOutImage, Hitable** world, Hitable** lightShapes, int nx, int ny, int ns, int maxDepth, int* progress)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= nx || y >= ny) return;

    unsigned int i = (ny - y - 1) * nx + x; // index of current pixel (calculated using thread index)

    unsigned int seed0 = x;  // seeds for random number generator
    unsigned int seed1 = y;
    //SimpleRng rng(seed0, seed1);
    PcgRng rng(i);
    Vector3f accumCol = render_pixel(world, lightShapes, x, y, nx, ny, ns, rng, maxDepth);

    pOutImage[i] = make_float3(accumCol[0], accumCol[1], accumCol[2]);

    if (progress != nullptr)
        atomicAdd(progress, 1);
}

COMMON_FUNC void simple_spheres(float aspect, Hitable **world, Hitable** lightShapes, Camera** camera, AmbientLight** ambientLight)
{
    int i = 0;
    Hitable** list = new Hitable*[4];
    list[i++] = new Sphere(Vector3f(0.0f, 0.0f, -1.0f), 0.5f, new Lambertian(new ConstantTexture(Vector3f(0.1, 0.2, 0.5))));
    list[i++] = new Sphere(Vector3f(0.0f, -100.5f, -1.0f), 100.0f, new Lambertian(new ConstantTexture(Vector3f(0.8, 0.8, 0.0))));
    list[i++] = new Sphere(Vector3f(1, 0, -1), 0.5, new Metal(Vector3f(0.8, 0.6, 0.2), 0.3));
    list[i++] = new Sphere(Vector3f(-1, 0, -1), 0.5, new Dielectric(1.5));

    *world = new HitableList(i, list);
    *lightShapes = nullptr;

    *camera = new Camera(Vector3f(-2, 2, 1), Vector3f(0, 0, -1), Vector3f(0, 1, 0), 90, aspect, 0.0f, 10.0f);

    *ambientLight = new SkyAmbient();
}

COMMON_FUNC void simple_light(float aspect, Hitable **world, Hitable** lightShapes, Camera** camera, AmbientLight** ambientLight)
{
    const Vector3f lookFrom(13, 2, 3);
    const Vector3f lookAt(0, 0, 0);
    const double dist_to_focus = 10.0;
    const double aperture = 0.0;
    *camera = new Camera(lookFrom, lookAt, Vector3f(0, 1, 0), 40, aspect, aperture, dist_to_focus);

    Texture* noise = new NoiseTexture(1.0f);
    int i = 0;
    Hitable** list = new Hitable*[4];
    list[i++] = new Sphere(Vector3f(0,-1000, 0), 1000, new Lambertian(noise));
    list[i++] = new Sphere(Vector3f(0, 2, 0), 2, new Lambertian(noise));

    list[i++] = new Sphere(Vector3f(0, 7, 0), 2, new DiffuseLight(new ConstantTexture(Vector3f(4, 4, 4))));
    list[i++] = new XYRectangle(3, 5, 1, 3, -2, new DiffuseLight(new ConstantTexture(Vector3f(4, 4, 4))));

    Hitable** lights = new Hitable*[2];
    lights[0] = new Sphere(Vector3f(0, 7, 0), 2, nullptr);
    lights[1] = new XYRectangle(3, 5, 1, 3, -2, nullptr);

    *ambientLight = new ConstantAmbient();

    *world = new HitableList(i, list);
    *lightShapes = new HitableList(2, lights);
}

COMMON_FUNC void random_scene(float aspect, Hitable **world, Hitable** lightShapes, Camera** camera, AmbientLight** ambientLight)
{
    const Vector3f lookFrom(13, 2, 3);
    const Vector3f lookAt(0, 0, 0);
    const double dist_to_focus = 10.0;
    const double aperture = 0.0;
    *camera = new Camera(lookFrom, lookAt, Vector3f(0, 1, 0), 20, aspect, aperture, dist_to_focus, 0.0, 1.0);

    SimpleRng rng(42, 13);

    int n = 500;
    Hitable** list = new Hitable*[n];

    int i = 0;
    Texture* checker = new CheckerTexture(new ConstantTexture(Vector3f(0.2, 0.3, 0.1)), new ConstantTexture(Vector3f(0.9, 0.9, 0.9)));
    list[i++] = new Sphere(Vector3f(0,-1000,0), 1000, new Lambertian(checker));
    for (int a = -11; a < 11; a++)
    {
        for (int b = -11; b < 11; b++)
        {
            double choose_mat = rng.rand();
            Vector3f center(a+0.9*rng.rand(),0.2,b+0.9*rng.rand());
            if ((center-Vector3f(4,0.2,0)).length() > 0.9)
            {
                if (choose_mat < 0.8) // diffuse
                {
                    list[i++] = new MovingSphere(center, center+Vector3f(0, 0.5*rng.rand(),0), 0, 1, 0.2, new Lambertian(new ConstantTexture(Vector3f(rng.rand()*rng.rand(), rng.rand()*rng.rand(), rng.rand()*rng.rand()))));
                }
                else if (choose_mat < 0.95) // metal
                {
                    list[i++] = new Sphere(center, 0.2, new Metal(Vector3f(0.5*(1+rng.rand()), 0.5*(1+rng.rand()), 0.5*rng.rand()), 0.3));
                }
                else // glass
                {
                    list[i++] = new Sphere(center, 0.2, new Dielectric(1.5));
                }
            }
        }
    }

    list[i++] = new Sphere(Vector3f(0,1,0), 1.0, new Dielectric(1.5));
    list[i++] = new Sphere(Vector3f(-4, 1, 0), 1.0, new Lambertian(new ConstantTexture(Vector3f(0.4, 0.2, 0.1))));
    list[i++] = new Sphere(Vector3f(4, 1, 0), 1.0, new Metal(Vector3f(0.7, 0.6, 0.5), 0.0));

    *ambientLight = new SkyAmbient();

    *world = new HitableList(i, list);
    *lightShapes = nullptr;
}

COMMON_FUNC void cornell_box(float aspect, Hitable **world, Hitable** lightShapes, Camera** camera, AmbientLight** ambientLight)
{
    int i = 0;
    Hitable **list = new Hitable*[8];
    Material *red = new Lambertian( new ConstantTexture(Vector3f(0.65f, 0.05f, 0.05f)) );
    Material *white = new Lambertian( new ConstantTexture(Vector3f(0.73, 0.73, 0.73)) );
    Material *green = new Lambertian( new ConstantTexture(Vector3f(0.12, 0.45, 0.15)) );
    Material *light = new DiffuseLight( new ConstantTexture(Vector3f(15, 15, 15)) );

    list[i++] = new FlipNormals(new YZRectangle(0, 555, 0, 555, 555, green));
    list[i++] = new YZRectangle(0, 555, 0, 555, 0, red);
    list[i++] = new FlipNormals(new XZRectangle(213, 343, 227, 332, 554, light));
    list[i++] = new FlipNormals(new XZRectangle(0, 555, 0, 555, 555, white));
    list[i++] = new XZRectangle(0, 555, 0, 555, 0, white);
    list[i++] = new FlipNormals(new XYRectangle(0, 555, 0, 555, 555, white));

    //list[i++] = new Sphere(Vector3f(160, 100, 145), 100, new Dielectric(1.5));
    list[i++] = new Translate(new RotateY(new Box(Vector3f(0, 0, 0), Vector3f(165, 165, 165), white), -18), Vector3f(130, 0, 65));
    list[i++] = new Translate(new RotateY(new Box(Vector3f(0, 0, 0), Vector3f(165, 330, 165), white), 15), Vector3f(265, 0, 295));

    *world = new HitableList(i, list);

    const Vector3f lookFrom(278, 278, -800);
    const Vector3f lookAt(278, 278, 0);
    const double dist_to_focus = 10.0;
    const double aperture = 0.0;
    *camera = new Camera(lookFrom, lookAt, Vector3f(0, 1, 0), 40, aspect, aperture, dist_to_focus);

    *ambientLight = new ConstantAmbient();

    *lightShapes = new XZRectangle(213, 343, 227, 332, 554, NULL);
}

COMMON_FUNC void cornell_box_spheres(float aspect, Hitable **world, Hitable** lightShapes, Camera** camera, AmbientLight** ambientLight)
{
    int i = 0;
    Hitable **list = new Hitable*[8];
    Material *red = new Lambertian( new ConstantTexture(Vector3f(0.65f, 0.05f, 0.05f)) );
    Material *white = new Lambertian( new ConstantTexture(Vector3f(0.73, 0.73, 0.73)) );
    Material *green = new Lambertian( new ConstantTexture(Vector3f(0.12, 0.45, 0.15)) );

    list[i++] = new Sphere(Vector3f(1e5f+1.0f, 40.8f, 81.6f), 1e5f, red);
    list[i++] = new Sphere(Vector3f(-1e5f+99.0f, 40.8f, 81.6f), 1e5f, red);

    list[i++] = new Sphere(Vector3f(50.0f, 40.8f, 1e5f), 1e5f, green);

    list[i++] = new Sphere(Vector3f(50.0f, 1e5f, 81.6f), 1e5f, white);
    list[i++] = new Sphere(Vector3f(50.0f, -1e5f + 81.6f, 81.6f), 1e5f, white);

    list[i++] = new Sphere(Vector3f(27.0f, 16.5f, 47.0f), 16.5f, white);
    list[i++] = new Sphere(Vector3f(73.0f, 16.5f, 78.0f), 16.5f, white);

    *world = new HitableList(i, list);

    const Vector3f lookFrom(278, 278, -800);
    const Vector3f lookAt(278, 278, 0);
    const double dist_to_focus = 10.0;
    const double aperture = 0.0;
    *camera = new Camera(lookFrom, lookAt, Vector3f(0, 1, 0), 40, aspect, aperture, dist_to_focus);

    *ambientLight = new ConstantAmbient();
    *lightShapes = nullptr;
}

COMMON_FUNC void final(float aspect, Hitable **world, Hitable** lightShapes, Camera** camera, AmbientLight** ambientLight)
{
    const Vector3f lookFrom(478, 278, -600); //(278, 278, -800); //(13, 2, 3);
    const Vector3f lookAt(278, 278, 0); //(0, 1, 0);
    const float dist_to_focus = 10.0f;
    const float aperture = 0.0f;
    *camera = new Camera(lookFrom, lookAt, Vector3f(0, 1, 0), 40, aspect, aperture, dist_to_focus);

    int nb = 20;

    Material* white = new Lambertian(new ConstantTexture(Vector3f(0.73, 0.73, 0.73)));
    Material* ground = new Lambertian(new ConstantTexture(Vector3f(0.48, 0.83, 0.53)));
    Hitable** boxList = new Hitable*[nb*nb];

    SimpleRng rng(42, 13);

    int bi = 0;
    for (int i = 0; i < nb; i++)
    {
        for (int j = 0; j < nb; j++)
        {
            float w = 100;
            float x0 = -1000 + i*w;
            float z0 = -1000 + j*w;
            float y0 = 0;
            float x1 = x0 + w;
            float y1 = 100*(rng.rand()+0.01f);
            float z1 = z0 + w;
            boxList[bi++] = new Box(Vector3f(x0, y0, z0), Vector3f(x1, y1, z1), ground);
        }
    }

    int i = 0;
    Hitable **list = new Hitable*[12];
    list[i++] = new BVH(boxList, bi, 0, 1, rng);
    Material* light = new DiffuseLight(new ConstantTexture(Vector3f(6, 6, 6)));
    list[i++] = new FlipNormals(new XZRectangle(123, 423, 147, 412, 554, light));
    Vector3f center(400, 400, 200);
    list[i++] = new MovingSphere(center, center+Vector3f(30, 0, 0), 0, 1, 50, new Lambertian(new ConstantTexture(Vector3f(0.7, 0.3, 0.1))));
    list[i++] = new Sphere(Vector3f(260, 150, 45), 50, new Dielectric(1.5));
    list[i++] = new Sphere(Vector3f(0, 150, 145), 50, new Metal(Vector3f(0.8, 0.8, 0.9), 10));
    Hitable* boundary = new Sphere(Vector3f(360, 150, 145), 70, new Dielectric(1.5));
    list[i++] = boundary;
    list[i++] = new ConstantMedium(boundary, 0.02, new ConstantTexture(Vector3f(0.2, 0.4, 0.9)));
    boundary = new Sphere(Vector3f(0, 0, 0), 5000, new Dielectric(1.5));
    list[i++] = new ConstantMedium(boundary, 0.0001, new ConstantTexture(Vector3f(1.0, 1.0, 1.0)));
#ifdef __CUDA_ARCH__
#else
    int nx, ny, nz;
    unsigned char* tex_data = stbi_load("earthmap.jpg", &nx, &ny, &nz, 0);
    Material* emat = new Lambertian(new ImageTexture(tex_data, nx, ny));
    list[i++] = new Sphere(Vector3f(400, 200, 400), 100, emat);
#endif
    Texture* pertext = new NoiseTexture(0.1);
    list[i++] = new Sphere(Vector3f(220, 280, 300), 80, new Lambertian(pertext));
    int ns = 1000;
    Hitable** boxList2 = new Hitable*[ns];

    for (int j = 0; j < ns; j++)
    {
        boxList2[j] = new Sphere(Vector3f(165*rng.rand(), 165*rng.rand(), 165*rng.rand()), 10, white);
    }
    //list[i++] = new Translate(new RotateY(new BVH(boxList2, ns, 0.0f, 1.0f, rng), 15), Vector3f(-100, 270, 395));

    *lightShapes = new XZRectangle(123, 423, 147, 412, 554, nullptr);
    //lights.push_back(new Sphere(Vector3(360, 150, 145), 70, nullptr));
    //lights.push_back(new Sphere(Vector3(0, 0, 0), 5000, nullptr));

    *ambientLight = new ConstantAmbient();

    *world = new HitableList(i, list);
}

__global__ void allocate_world_kernel(Hitable** world, Hitable** lightShapes, void* pData, size_t dataSize)
{
    Stream stream(pData, dataSize);
    *world = Hitable::Create(&stream);
    *lightShapes = Hitable::Create(&stream);
    g_cam = Camera::Create(&stream);
    g_ambientLight = AmbientLight::Create(&stream);
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

                    int ir = Clamp(int(255.99 * col[0]), 0, 255);
                    int ig = Clamp(int(255.99 * col[1]), 0, 255);
                    int ib = Clamp(int(255.99 * col[2]), 0, 255);

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

void renderLine(int line, Vector3f* outLine, int nx, int ny, int ns, Hitable* world, Hitable* lightShapes, RNG& rng, int maxDepth)
{
    for (int x = 0; x < nx; x++)
    {
        outLine[x] = render_pixel(&world, &lightShapes, x, line, nx, ny, ns, rng, maxDepth);
    }
}

void medianFilter3x3(Vector3f*, const Vector3f *, int, int);
void medianFilter2x2(Vector3f*, const Vector3f *, int, int);

int main(int argc, char** argv)
{
    cxxopts::Options options("pathtracer", "Implementation of Peter Shirley's Raytracing in One Weekend book series.");
    options.add_options()
        ("q,quick", "Quick render.")
        ("c,cpu", "Render on CPU.")
        ("m,median", "Apply median filter to output.")
        ("w,width", "Output width.", cxxopts::value<int>())
        ("h,height", "Output height.", cxxopts::value<int>())
        ("n,numsamples", "Number of sample rays per pixel.", cxxopts::value<int>())
        ("t,threads", "Number of render threads.", cxxopts::value<int>())
        ("d,maxdepth", "Maximum ray bounces.", cxxopts::value<int>())
        ("s,stacksize", "Size of GPU thread stack (bytes)", cxxopts::value<int>())
        ("f,file", "Output filename.", cxxopts::value<std::string>());

    options.parse(argc, argv);

    bool quick = options.count("quick") > 0;
    int ns = 100;
    int nx = 128 * 4;
    int ny = 128 * 4;
    bool cpu = options.count("cpu") > 0;
    bool filter = options.count("median") > 0;
    int numThreads = 1;
    int maxDepth = 25;
    int threadStackSize = -1; // default

    std::string outFile("outputImage.ppm");

    if (options.count("width"))
        nx = options["width"].as<int>();
    if (options.count("height"))
        ny = options["height"].as<int>();
    if (options.count("numsamples"))
        ns = options["numsamples"].as<int>();
    if (options.count("maxdepth"))
        maxDepth = options["maxdepth"].as<int>();
    if (options.count("file"))
        outFile = options["file"].as<std::string>();
    if (options.count("threads"))
        numThreads = options["numthreads"].as<int>();
    if (options.count("stacksize"))
        threadStackSize = options["stacksize"].as<int>();

    if (quick)
    {
        nx /= 8;
        ny /= 8;
        ns /= 16;
    }

    const float aspect = float(nx)/float(ny);

    std::vector<Vector3f> outImage(nx*ny);

    Hitable* world = nullptr;
    Hitable* lightShapes = nullptr;
    Camera* camera = nullptr;
    AmbientLight* ambientLight = nullptr;
    random_scene(aspect, &world, &lightShapes, &camera, &ambientLight);// cornellBox(); // simpleLight(); //randomScene(); //
    Stream* pStream = new Stream();
    pStream->create(1024 * 1024 * 16);

    bool ok = world->serialize(pStream);
    if (lightShapes != nullptr)
        ok |= lightShapes->serialize(pStream);
    else
        ok |= pStream->writeNull();
    ok |= camera->serialize(pStream);
    if (ambientLight != nullptr)
        ok |= ambientLight->serialize(pStream);
    else
        ok |= pStream->writeNull();

    if (!ok)
    {
        std::cerr << "Failed to serialize world to GPU memory." << std::endl;
        return EXIT_FAILURE;
    }
    if (!cpu)
    {
        size_t stackSize;
        cudaDeviceGetLimit(&stackSize, cudaLimitStackSize);
        std::cout << "Max stack size: " << stackSize << std::endl;

        if (threadStackSize > 0)
        {
            cudaDeviceSetLimit(cudaLimitStackSize, threadStackSize);

            cudaDeviceGetLimit(&stackSize, cudaLimitStackSize);
            std::cout << "New Max stack size: " << stackSize << std::endl;
        }

        float3* pOutImage = nullptr;
        cudaMalloc(&pOutImage, nx * ny * sizeof(float3));

        Hitable** world = nullptr;
        cudaMalloc(&world, sizeof(Hitable**));

        Hitable** lightShapes = NULL;
        cudaMalloc(&lightShapes, sizeof(Hitable**));

        std::cerr << "Allocating world...";
        allocate_world_kernel<<<1, 1>>>(world, lightShapes, pStream->data(), pStream->size());
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

        Progress progress(nx*ny, "PathTracers");

        int* progressCounter;
        cudaMallocManaged(&progressCounter, 4);
        *progressCounter = 0;

        auto progressFunc = [&progress, progressCounter](){
            int previousValue = *progressCounter;
            int delta = 0;
            do
            {
                sleep(2);
                int currentValue = *progressCounter;
                delta = currentValue - previousValue;
                previousValue = currentValue;
            } while (!progress.update(delta));
            std::cout << "Progress thread exited." << std::endl;
        };
        //std::thread progressThread(progressFunc);

        render_kernel<<<grid, block>>>(pOutImage, world, lightShapes, nx, ny, ns, maxDepth, progressCounter);
        err = cudaDeviceSynchronize();
        std::cerr << "done" << std::endl;
        //progressThread.join();
        progress.completed();

        if (err != cudaSuccess)
        {
            std::cerr << "Failed to render on GPU.  Error: " << cudaGetErrorName(err) << " Desc: " << cudaGetErrorString(err) << std::endl;
            return EXIT_FAILURE;
        }

        cudaFree(progressCounter);
        cudaMemcpy(outImage.data(), pOutImage, nx*ny*sizeof(Vector3f), cudaMemcpyDeviceToHost);
        cudaFree(pOutImage);
        cudaFree(lightShapes);
        cudaFree(world);
    }
    else
    {
        Hitable* clonedWorld = Hitable::Create(pStream);
        g_ambientLight = ambientLight;
        g_cam = camera;
        unsigned int seed0 = 42;
        unsigned int seed1 = 13;
        PcgRng rng(seed0);//, seed1);

        Progress progress(nx*ny, "PathTracers");

        #pragma omp parallel for if(numThreads)
        for (int j = 0; j < ny; j++)
        {
            Vector3f* outLine = outImage.data() + (nx * j);
            const int line = ny - j - 1;
            renderLine(line, outLine, nx, ny, ns, clonedWorld, lightShapes, rng, maxDepth);

            #pragma omp critical(progress)
            {
                progress.update(nx);
            }
        }

        progress.completed();
    }

    pStream->close();
    delete pStream;

    if (filter)
    {
        std::vector<Vector3f> filteredImage(nx * ny);
        medianFilter3x3(filteredImage.data(), outImage.data(), nx, ny);
        writeImage(outFile, filteredImage.data(), nx, ny);
    }
    else
    {
        writeImage(outFile, outImage.data(), nx, ny);
    }

    std::cerr << "Done." << std::endl;

    return EXIT_SUCCESS;
}

int compare(const void* pArg0, const void* pArg1)
{
    Vector3f val0 = *((Vector3f*)pArg0);
    Vector3f val1 = *((Vector3f*)pArg1);

    int result = 0;
    auto v0 = val0.squared_length();
    auto v1 = val1.squared_length();
    if (v0 < v1)
    {
        result = -1;
    }
    else if (v0 > v1)
    {
        result = 1;
    }
    return result;
}

Vector3f median(Vector3f* pValues, size_t count)
{
    qsort(pValues, count, sizeof(Vector3f), compare);
    return pValues[(count-1)/2];
}

void medianFilter3x3(Vector3f* output, const Vector3f *input, int nx, int ny)
{
    Vector3f hood[9];
    Vector3f med(0, 0, 0);

    for (int l = 0; l < ny; l++)
    {
        int l0 = std::max(0, l-1);
        int l1 = l;
        int l2 = std::min(ny-1, l+1);

        for (int s = 0; s < nx; s++)
        {
            int s0 = std::max(0, s-1);
            int s1 = s;
            int s2 = std::min(nx-1, s+1);

            hood[0] = input[l0*nx+s0]; //src(s0, l0);
            hood[1] = input[l1*nx+s0]; //src(s0, l1);
            hood[2] = input[l2*nx+s0]; //src(s0, l2);

            hood[3] = input[l0*nx+s1]; //src(s1, l0);
            hood[4] = input[l1*nx+s1]; //src(s1, l1);
            hood[5] = input[l2*nx+s1]; //src(s1, l2);

            hood[6] = input[l0*nx+s2]; //src(s2, l0);
            hood[7] = input[l1*nx+s2]; //src(s2, l1);
            hood[8] = input[l2*nx+s2]; //src(s2, l2);

            med = median(hood, 9);

            output[l*nx+s] = med;
        }
    }
}


void medianFilter2x2(Vector3f* output, const Vector3f *input, int nx, int ny)
{
    Vector3f hood[4];
    Vector3f med(0, 0, 0);

    for (int l = 0; l < ny; l++)
    {
        int l0 = std::max(0, l-1);
        int l1 = l;

        for (int s = 0; s < nx; s++)
        {
            int s0 = std::max(0, s-1);
            int s1 = s;

            hood[0] = input[l0*nx+s0]; //src(s0, l0);
            hood[1] = input[l1*nx+s0]; //src(s0, l1);

            hood[2] = input[l0*nx+s1]; //src(s1, l0);
            hood[3] = input[l1*nx+s1]; //src(s1, l1);

            med = median(hood, 4);

            output[l*nx+s] = med;
        }
    }
}
