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

    auto t = ro.getTransform();
    t = glm::rotate(t, 3.14f * delta, glm::vec3(0, 1, 0));
    ro.setTransform(t);
    
    angle += ((3.14/2) * delta);
    camera = glm::mat4();
    camera = glm::rotate(camera, angle, glm::vec3(0, 1, 0));
    camera = glm::translate(camera, glm::vec3(0, 0, 2.5));

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
        {0.5f,  0.5f, 0.0f , 1, 0}, // top right
        {0.5f, -0.5f, 0.0f , 1, 1}, // bottom right
        {-0.5f, -0.5f, 0.0f, 0, 1},  // bottom left
        {-0.5f,  0.5f, 0.0f, 0, 0}  // top left 
    };
    std::vector<uint32_t> indices = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    auto mesh = r.createMesh(vertices, indices);
    auto mat = r.createMaterial();

    ro = r.createRenderObject(mesh, mat, glm::mat4());

    // Camera
    glm::mat4 cam_mat;
    cam_mat = glm::translate(cam_mat, glm::vec3(0, 0, 1));
    r.setCamera(cam_mat);
    camera = cam_mat;

    r.getMaterial(mat)->uniforms.set("color", glm::vec3(0.6, 1, 0.6));
    r.getMaterial(mat)->uniforms.set("has_texture", 1);

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

    auto objs = loadModel(&r, "Assets/jmodl.glb");

    w.mainloop(&r, loop);
}