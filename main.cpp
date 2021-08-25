#include <ios>
#include <iostream>
#include "Frender/Frender.hh"
#include "Frender/Keys.hh"
#include "GLTFLoader.hh"
#include "glm/gtc/quaternion.hpp"
// #define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <sstream>

Frender::RenderObjectRef ro;
Frender::Window* window;
Frender::Renderer* renderer;
glm::mat4 camera;
float angle = 0;

double elapsed_time;

glm::mat4 camera_position;
glm::mat4 camera_rotationa;
glm::mat4 camera_rotationb;

void loop(float delta)
{
    // Show FPS
    // elapsed_time += delta;

    // if (elapsed_time > 0.5)
    // {
    //     std::ostringstream outs;
    //     outs.precision(3); // Set precision of numbers
    //     // outs.fill(3);

    //     outs << std::fixed << "FPS: " << 1; // << " EFT: " << window->time_time << " EFPS: " << 1/window->time_time;

    //     window->setWindowTitle(outs.str());
    //     elapsed_time = 0;
    // }

    // auto t = ro.getTransform();
    // t = glm::rotate(t, 3.14f * delta, glm::vec3(0, 1, 0));
    // ro.setTransform(t);
    
    // angle += ((3.14/4) * delta);
    // camera = glm::mat4();
    // camera = glm::rotate(camera, angle, glm::vec3(0, 1, 0));
    // // camera = glm::translate(camera, glm::vec3(0, 20, 2.5));
    // camera = glm::translate(camera, glm::vec3(0, 0, 2.5));

    if (window->isKeyDown(FRENDER_KEY_S))
    {
        // std::cout << "WDown\n";
        camera_position = glm::translate(camera_position, glm::vec3(0, 0, 10) * delta);
    }
    if (window->isKeyDown(FRENDER_KEY_W))
    {
        camera_position = glm::translate(camera_position, glm::vec3(0, 0, -10) * delta);
        // camera_position = glm::translate(camera_position, glm::vec3(glm::inverse(camera_position) * glm::vec4(0, 0, -10, 1)) * delta);
    }

    if (window->isKeyDown(FRENDER_KEY_A))
    {
        // std::cout << "WDown\n";
        camera_position = glm::translate(camera_position, glm::vec3(10, 0, 0) * delta);
    }
    if (window->isKeyDown(FRENDER_KEY_D))
    {
        camera_position = glm::translate(camera_position, glm::vec3(-10, 0, 0) * delta);
    }

    if (window->isKeyJustPressed(FRENDER_KEY_ESCAPE))
    {
        std::cout << "Releasing\n";
        window->setMouseMode(Frender::Regular);
    }

    if (window->isKeyJustPressed(FRENDER_KEY_ENTER))
    {
        std::cout << "Capturing\n";
        window->setMouseMode(Frender::Captured);
    }

    // Looking around
    camera_position = glm::rotate(camera_position, window->getMouseOffset().y * 0.005f, glm::vec3(1, 0, 0));
    camera_position = glm::rotate(camera_position, window->getMouseOffset().x * -0.005f, glm::vec3(glm::inverse(camera_position) * glm::vec4(0, 1, 0, 1)));

    // glm::mat4 camera_complete = camera_rotation;
    // camera_complete[3] = camera_position[3];

    renderer->setCamera(camera_position);
}

int main(int, char**)
{
    Frender::Window w({640, 480, "Hello Frender"});
    w.setVsync(false);
    // w.setVsync(true);

    Frender::Renderer r(640, 480);
    renderer = &r;
    window = &w;

    std::vector<Frender::Vertex> vertices = {
        {1.0f,  1.0f, 0.0f , 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0}, // top right
        {1.0f, -1.0f, 0.0f , 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}, // bottom right
        {-1.0f, -1.0f, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // bottom left
        {-1.0f,  1.0f, 0.0f, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0}  // top left 
    };
    std::vector<uint32_t> indices = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    auto mesh = r.createMesh(vertices, indices);
    auto mat = r.createMaterial();

    // ro = r.createRenderObject(mesh, mat, glm::mat4());

    // Camera
    glm::mat4 cam_mat;
    cam_mat = glm::translate(cam_mat, glm::vec3(0, 0, 1));
    r.setCamera(cam_mat);
    camera = cam_mat;

    r.getMaterial(mat)->uniforms.set("color", glm::vec3(0.6, 1, 0.6));
    r.getMaterial(mat)->uniforms.set("has_diffuse_map", 1);

    // Texture
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    float* data = stbi_loadf("Assets/GrandCanyon_C_YumaPoint/GCanyon_C_YumaPoint_3k.hdr", &width, &height, &channels, STBI_rgb_alpha);
    std::cout << channels << "\n";

    if (data)
    {
        // auto tex = r.createTexture(width, height, data);
        // r.getMaterial(mat)->textures.set("tex", tex);
        r.setSkybox(width, height, data);
    }
    else
    {
        std::cerr << "Waaaaaaaaaaaaaa image not found\n";
    }

    stbi_image_free(data);

    // auto objs = loadModel(&r, "Assets/jmodl.glb");
    // auto objs = loadModel(&r, "Assets/HighPolySphere.obj");

    // for (int x = -24; x < 24; x+=4)
    // {
    //     for (int y = -24; y < 24; y+=4)
    //     {
    //         for (int z = -24; z < 24; z+=4)
    //         {
    //             // auto objs = loadModel(&r, "Assets/jmodl.glb");
    //             for (auto i : objs)
    //             {
    //                 auto o = i.duplicate();
    //                 o.setTransform(glm::translate(i.getTransform(), glm::vec3(x, y, z)));
    //             }

    //             auto light = r.createPointLight(glm::vec3(x, y, z), glm::vec3(((x+24.0)/48.0), ((y+24.0)/48.0), ((z+24.0)/48.0)) * 1.0f, 6);

    //         }
    //     }
    // }
    // auto objs = loadModel(&r, "Assets/HighPolySphere.obj");

    // auto objs = loadModel(&r, "Assets/Sponza/sponza.obj");
    ///home/finn/cpp/flux/Frender/Assets/sponza-scene/source/sponza_zip/glTF/Sponza.gltf
    auto objs = loadModel(&r, "Assets/sponza-scene/source/sponza_zip/glTF/Sponza.gltf");


    for (auto i : objs)
    {
        // Scale it up
        i.setTransform(glm::scale(i.getTransform(), glm::vec3(1.5)));
    }

    auto objs2 = loadModel(&r, "Assets/HighPolySphere.obj");
    for (auto i : objs2)
    {
        // TODO: Be able to delete old one
        auto traits = renderer->getRenderObjectTraits(i);
        i.setTransform(glm::scale(i.getTransform(), glm::vec3(0)));

        // Create new, emmisive object
        auto mat = renderer->createMaterial();
        renderer->getMaterial(mat)->uniforms.set("color", glm::vec3(0, 1, 0));
        renderer->createLitRenderObject(traits.mesh, mat, traits.transform);

        renderer->createRenderObject(traits.mesh, mat, glm::translate(traits.transform, glm::vec3(4, 0, 0)));
    }

    auto light = r.createPointLight(glm::vec3(0.120, -0.870, 2.030), glm::vec3(104, 0, 0), 4);
    auto light2 = r.createPointLight(glm::vec3(0.120, -0.870, -2.030), glm::vec3(0, 4, 0), 4);
    auto light3 = r.createPointLight(glm::vec3(0, 2, 0), glm::vec3(0, 0, 4), 4);
    auto dlight = r.createDirectionalLight(glm::vec3(1.5, 1.5, 1.5), glm::vec3(0.2f, -1.0f, 0.3f));
    auto dlight0 = r.createDirectionalLight(glm::vec3(1, 1, 1), glm::vec3(-0.2f, -1.0f, -0.3f));
    auto dlight1 = r.createDirectionalLight(glm::vec3(1, 1, 1), glm::vec3(0.2f, -1.0f, -0.3f));
    auto dlight2 = r.createDirectionalLight(glm::vec3(1, 1, 1), glm::vec3(-0.2f, -1.0f, 0.3f));
    auto dlight3 = r.createDirectionalLight(glm::vec3(0.5, 0.5, 0.5), glm::vec3(0, 1.0f, 0));

    camera_position = glm::translate(glm::mat4(), glm::vec3(0, 5, 0));

    // w.setMouseMode(Frender::Captured);

    w.mainloop(&r, loop);
}