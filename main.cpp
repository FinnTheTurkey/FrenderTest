#include <ios>
#include <iostream>
#include "Frender/Frender.hh"
#include "GLTFLoader.hh"
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
    
    angle += ((3.14/2) * delta);
    camera = glm::mat4();
    camera = glm::rotate(camera, angle, glm::vec3(0, 1, 0));
    camera = glm::translate(camera, glm::vec3(0, 0, 2.5));
    // camera = glm::translate(camera, glm::vec3(0, 10, 0));

    renderer->setCamera(camera);
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
    unsigned char* data = stbi_load("container.jpg", &width, &height, &channels, 4);
    std::cout << channels << "\n";

    if (data)
    {
        auto tex = r.createTexture(width, height, data);
        r.getMaterial(mat)->textures.set("tex", tex);
    }
    else
    {
        std::cerr << "Waaaaaaaaaaaaaa image not found\n";
    }

    stbi_image_free(data);

    // auto objs = loadModel(&r, "Assets/jmodl.glb");

    for (int x = -24; x < 24; x+=4)
    {
        for (int y = -24; y < 24; y+=4)
        {
            for (int z = -24; z < 24; z+=4)
            {
                auto objs = loadModel(&r, "Assets/HighPolySphere.obj");
                // auto objs = loadModel(&r, "Assets/jmodl.glb");
                for (auto i : objs)
                {
                    i.setTransform(glm::translate(glm::mat4(), glm::vec3(x, y, z)));
                }

                auto light = r.createPointLight(glm::vec3(x, y, z), glm::vec3(((x+24.0)/48.0), ((y+24.0)/48.0), ((z+24.0)/48.0)), 6);

            }
        }
    }
    // auto objs = loadModel(&r, "Assets/Sphere.obj");

    // auto objs = loadModel(&r, "Assets/Sponza/sponza.obj");

    // auto light = r.createPointLight(glm::vec3(0.120, -0.870, 2.030), glm::vec3(1, 0, 0), 4);
    // auto light2 = r.createPointLight(glm::vec3(0.120, -0.870, -2.030), glm::vec3(0, 1, 0), 4);
    // auto light3 = r.createPointLight(glm::vec3(0, 2, 0), glm::vec3(0, 0, 1), 4);
    // auto dlight = r.createDirectionalLight(glm::vec3(1, 1, 1), glm::vec3(0.2f, -1.0f, 0.3f));
    // auto dlight0 = r.createDirectionalLight(glm::vec3(1, 1, 1), glm::vec3(-0.2f, -1.0f, -0.3f));
    // auto dlight1 = r.createDirectionalLight(glm::vec3(1, 1, 1), glm::vec3(0.2f, -1.0f, -0.3f));
    // auto dlight2 = r.createDirectionalLight(glm::vec3(1, 1, 1), glm::vec3(-0.2f, -1.0f, 0.3f));

    w.mainloop(&r, loop);
}