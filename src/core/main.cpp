/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <core/parser.h>
#include <objects/scene.h>
#include <objects/camera.h>
#include <core/block.h>
#include <tools/timer.h>
#include <core/bitmap.h>
#include <objects/sampler.h>
#include <objects/integrator.h>
#include <core/gui.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/task_scheduler_init.h>
#include <filesystem/resolver.h>
#include <thread>

using namespace nori;

static int threadCount = -1;
static bool gui = true;
static std::string savePath = "../scenes/output";

static void renderBlock(const Scene *scene, Sampler *sampler, ImageBlock &block) {
    const Camera *camera = scene->getCamera();
    const Integrator *integrator = scene->getIntegrator();

    Point2i offset = block.getOffset();
    Vector2i size  = block.getSize();

    /* Clear the block contents */
    block.clear();

    /* For each pixel and pixel sample sample */
    for (int y = 0; y < size.y(); ++y) {
        for (int x = 0; x < size.x(); ++x) {
            for (uint32_t i = 0; i < sampler->getSampleCount(); ++i) {
                Point2f pixelSample = Point2f((float) (x + offset.x()), (float) (y + offset.y())) + sampler->next2D();
                Point2f apertureSample = sampler->next2D();

                /* Sample a ray from the camera */
                Ray3f ray;
                Color3f value = camera->sampleRay(ray, pixelSample, apertureSample);

                /* Compute the incident radiance */
                value *= integrator->Li(scene, sampler, ray);

                /* Store in the image block */
                block.put(pixelSample, value);
            }
        }
    }
}

static void render(Scene *scene, const std::string &filename) {
    const Camera *camera = scene->getCamera();
    Vector2i outputSize = camera->getOutputSize();
    scene->getIntegrator()->preprocess(scene);

    /* Create a block generator (i.e. a work scheduler) */
    BlockGenerator blockGenerator(outputSize, NORI_BLOCK_SIZE);

    /* Allocate memory for the entire output image and clear it */
    ImageBlock result(outputSize, camera->getReconstructionFilter());
    result.clear();

    /* Create a window that visualizes the partially rendered result */
    NoriScreen *screen = nullptr;
    if (gui) {
        nanogui::init();
        screen = new NoriScreen(result);
    }

    /* Do the following in parallel and asynchronously */
    std::thread render_thread([&] {
        tbb::task_scheduler_init init(threadCount);

        cout << "Rendering .. ";
        cout.flush();
        Timer timer;

        tbb::blocked_range<int> range(0, blockGenerator.getBlockCount());

        auto map = [&](const tbb::blocked_range<int> &range) {
            /* Allocate memory for a small image block to be rendered
               by the current thread */
            ImageBlock block(Vector2i(NORI_BLOCK_SIZE),
                camera->getReconstructionFilter());

            /* Create a clone of the sampler for the current thread */
            std::unique_ptr<Sampler> sampler(scene->getSampler()->clone());

            for (int i = range.begin(); i < range.end(); ++i) {
                /* Request an image block from the block generator */
                blockGenerator.next(block);

                /* Inform the sampler about the block to be rendered */
                sampler->prepare(block);

                /* Render all contained pixels */
                renderBlock(scene, sampler.get(), block);

                /* The image block has been processed. Now add it to
                   the "big" block that represents the entire image */
                result.put(block);
            }
        };

        /// Default: parallel rendering
        tbb::parallel_for(range, map);

        /// (equivalent to the following single-threaded call)
        // map(range);

        cout << "done. (took " << timer.elapsedString() << ")" << endl;
    });

    /* Enter the application main loop */
    if (gui)
        nanogui::mainloop(50.f);

    /* Shut down the user interface */
    render_thread.join();

    if (gui) {
        delete screen;
        nanogui::shutdown();
    }

    /* Now turn the rendered image block into
       a properly normalized bitmap */
    std::unique_ptr<Bitmap> bitmap(result.toBitmap());

    /* Determine the filename of the output bitmap */
    std::string outputName = filename;
    size_t lastdot = filename.find_last_of(".");
    size_t lastslash = filename.find_last_of("/");
    if (lastdot != std::string::npos)
        outputName.erase(lastdot, std::string::npos);
    if (lastslash != std::string::npos)
        outputName.erase(0, lastslash);
    outputName = savePath + outputName; 
    
    /* Save using the OpenEXR format */
    bitmap->saveEXR(outputName);
    /* Save tonemapped (sRGB) output using the PNG format */
    bitmap->savePNG(outputName);
}

int main(int argc, char **argv) {
    cmdline::parser a;
    a.add<std::string>("path", 'p', "XML file path", true, "");
    a.add<int>("threads", 't', "render threads number", false, -1, cmdline::range(1, 65535));
    a.add("gui", '\0', "graphics interface");
    a.parse_check(argc, argv);

    threadCount = a.get<int>("threads");
    gui = a.exist("gui");

    std::string sceneName = a.get<std::string>("path");
    filesystem::path path(sceneName);
    if (path.extension() == "xml") {
        /* Add the parent directory of the scene file to the
        file resolver. That way, the XML file can reference
        resources (OBJ files, textures) using relative paths */
        getFileResolver()->prepend(path.parent_path());
    } else {
        cerr << "Error while parsing \"" << sceneName
        << "\": expected an extension of type .xml" << endl;
        return -1;
    }

    if (threadCount < 0) {
        threadCount = tbb::task_scheduler_init::automatic;
    }
    try {
        std::unique_ptr<NoriObject> root(loadFromXML(sceneName));
        /* When the XML root object is a scene, start rendering it .. */
        if (root->getClassType() == NoriObject::EScene)
            render(static_cast<Scene *>(root.get()), sceneName);
    } catch (const std::exception &e) {
        cerr << e.what() << endl;
        return -1;
    }
    return 0;
}
