#include "Frender/Frender.hh"

#include <cstdio>
#include <iostream>

#include "GLTFLoader.hh"
#include "assimp/types.h"
#include "glm/gtc/type_ptr.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>


// Sooooo much assimp
#include <assimp/matrix4x4.h>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include <assimp/pbrmaterial.h>
#include <assimp/light.h>
#include <assimp/material.h>


#include <unordered_map>
#include <queue>
#include <filesystem>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Fun fact: This isn't actually a GLTF loader
// It loads all the filetypes!

glm::mat4 matrixConvert(aiMatrix4x4 mat)
{
    // Find transform
    aiVector3t scaling(0.0f);
    aiVector3t rot_axis(0.0f);
    aiVector3t position(0.0f);
    float rot_angle;

    mat.Decompose(scaling, rot_axis, rot_angle, position);

    float aaa[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    // Add transform
    glm::mat4 transform = glm::make_mat4(aaa);
    transform = glm::translate(transform, glm::vec3(position.x, position.y, position.z));

    // Predictably, rotating around an axis that doesn't exist causes major problems
    if (!(rot_axis.x == 0 && rot_axis.y == 0 && rot_axis.z == 0))
    {
        transform = glm::rotate(transform, rot_angle, glm::vec3(rot_axis.x, rot_axis.y, rot_axis.z));
    }
    
    transform = glm::scale(transform, glm::vec3(scaling.x, scaling.y, scaling.z));

    return transform;
}

glm::mat4 getNodeTransform(aiNode* node)
{
    // Recursivly search backwards
    if (!node->mParent)
    {
        return matrixConvert(node->mTransformation);
    }

    return getNodeTransform(node->mParent) * matrixConvert(node->mTransformation);
}

Frender::MeshRef loadMesh(unsigned int id, const aiScene* scene, Frender::Renderer* renderer)
{
    std::vector<Frender::Vertex> vertices;
    std::vector<uint32_t> indices;

    auto mesh = scene->mMeshes[id];
    
    // std::cout << "{";
    for (int i = 0; i < mesh->mNumVertices; i++)
    {
        Frender::Vertex vert(
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z,
            mesh->mNormals[i].x,
            mesh->mNormals[i].y,
            mesh->mNormals[i].z,
            0,
            0,
            mesh->mTangents[i].x,
            mesh->mTangents[i].y,
            mesh->mTangents[i].z,
            mesh->mBitangents[i].x,
            mesh->mBitangents[i].y,
            mesh->mBitangents[i].z
        );

        // TODO: Deal with multiple UV layers
        if (mesh->mTextureCoords[0] != nullptr)
        {
            vert.tex_coords.x = mesh->mTextureCoords[0][i].x;
            vert.tex_coords.y = mesh->mTextureCoords[0][i].y;
        }
        else
        {
            vert.tex_coords = {0, 0};
        }

        vertices.push_back(vert);

        // If you need a mesh in C++
        // std::printf("{%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f},\n",
                // mesh->mVertices[i].x,
            // mesh->mVertices[i].y,
            // mesh->mVertices[i].z,
            // mesh->mNormals[i].x,
            // mesh->mNormals[i].y,
            // mesh->mNormals[i].z,
            // vert.tex_coords.x,
            // vert.tex_coords.y,
            // mesh->mTangents[i].x,
            // mesh->mTangents[i].y,
            // mesh->mTangents[i].z,
            // mesh->mBitangents[i].x,
            // mesh->mBitangents[i].y,
            // mesh->mBitangents[i].z);
    }
    // std::cout << "}\n{";

    // Indices
    for (int i = 0; i < mesh->mNumFaces; i++)
    {
        for (int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
        {
            indices.push_back(mesh->mFaces[i].mIndices[j]);
            // std::cout << mesh->mFaces[i].mIndices[j] << ", ";
        }
    }

    // std::cout << "}\n";

    return renderer->createMesh(vertices, indices);
}

Frender::Texture loadTexture(std::string name, const aiScene* scene, Frender::Renderer* renderer)
{
    if (name[0] == '*')
    {
        // Internal texture :(
        int index = std::stoi(name.substr(1));

        auto tex = scene->mTextures[index];

        if (tex->mHeight == 0)
        {
            // It's in a normal image format
            // Use stb_image to decode it
            int x, y, channels;

            stbi_set_flip_vertically_on_load(true);
            auto data = (unsigned char*)stbi_load_from_memory((unsigned char*)tex->pcData, tex->mWidth, &x, &y, &channels, 4);

            if (!data)
            {
                std::cerr << "Failed to load internal image\n";
            }

            auto tx = renderer->createTexture(x, y, data);

            stbi_image_free(data);

            return tx;
        }
        else
        {
            std::cerr << "Internal images in generic format not implemented yet\n";
            return Frender::Texture();
        }
    }
    else
    {
        // External texture
        std::filesystem::path input(name);
        std::filesystem::path og = name;
        std::filesystem::path new_g;
        if (og.is_absolute() == true)
        {
            // Why do programs do this?
            // Move the textures here
            if (!std::filesystem::exists(input.parent_path() / (input.filename().string() + "_external_textures")))
            {
                std::filesystem::create_directory(input.parent_path() / (input.filename().string() + "_external_textures"));
            }

            if (std::filesystem::exists(input.parent_path() / (input.filename().string() + "_external_textures") / og.filename()))
            {
                // Delete it
                std::filesystem::remove(input.parent_path() / (input.filename().string() + "_external_textures") / og.filename());
            }

            std::filesystem::copy_file(og, input.parent_path() / (input.filename().string() + "_external_textures") / og.filename());
            new_g = input.parent_path() / (input.filename().string() + "_external_textures") / og.filename();
        }
        else
        {
            // Nice people who use relative paths get
            // to keep their own folder structure
            new_g = input.parent_path() / og;
        }

        // Use stb image to load
        // Texture
        int width, height, channels;
        stbi_set_flip_vertically_on_load(true);  
        unsigned char* data = stbi_load(new_g.c_str(), &width, &height, &channels, 4);
        std::cout << channels << "\n";

        Frender::Texture tex;

        if (data)
        {
            tex = renderer->createTexture(width, height, data);
        }
        else
        {
            std::cerr << "Waaaaaaaaaaaaaa image not found\n";
        }

        stbi_image_free(data);

        return tex;
    }
}

uint32_t loadMaterial(unsigned int id, const aiScene* scene, Frender::Renderer* renderer, std::map<std::string, Frender::Texture>& texes)
{
    auto mat = scene->mMaterials[id];

    // Create material with default shaders
    auto new_mat = renderer->createMaterial();
    auto m = renderer->getMaterial(new_mat);

    // Get attributes
    aiColor3D color(0, 0, 0);

    if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, color) != aiReturn_SUCCESS)
    {
        // Make the color purple
        color.r = 0.5;
        color.g = 0;
        color.b = 0.5;
    }

    m->uniforms.set("color", glm::vec3(color.r, color.g, color.b));

    // Check for diffuse texture
    if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        aiString path;
        mat->GetTexture(aiTextureType_DIFFUSE, 0, &path);

        if (texes.find(path.C_Str()) == texes.end())
        {
            texes[path.C_Str()] = loadTexture(path.C_Str(), scene, renderer);
        }

        m->textures.set("diffuse_map", texes[path.C_Str()]);
        m->uniforms.set("has_diffuse_map", 1);
    }
    else
    {
        m->uniforms.set("has_diffuse_map", 0);
    }

    // Deal with roughness and metalness
    float roughness = 0.4;
    float metallic = 0.0;
    
    if (mat->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, roughness) != aiReturn_SUCCESS)
    {
        // Not GLTF :(
        // That makes everything more annoying
    }

    if (mat->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, roughness) != aiReturn_SUCCESS)
    {
        // Not GLTF :(
        // That makes everything more annoying
    }

    m->uniforms.set("roughness", roughness);
    m->uniforms.set("metallic", metallic);

    // Check for normal map
    if (mat->GetTextureCount(aiTextureType_NORMALS) > 0)
    {
        aiString path;
        mat->GetTexture(aiTextureType_NORMALS, 0, &path);

        if (texes.find(path.C_Str()) == texes.end())
        {
            texes[path.C_Str()] = loadTexture(path.C_Str(), scene, renderer);
        }

        m->textures.set("normal_map", texes[path.C_Str()]);
        m->uniforms.set("has_normal_map", 1);
    }
    else
    {
        m->uniforms.set("has_normal_map", 0);
    }

    // Check for roughness map
    if (mat->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0)
    {
        aiString path;
        mat->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &path);

        if (texes.find(path.C_Str()) == texes.end())
        {
            texes[path.C_Str()] = loadTexture(path.C_Str(), scene, renderer);
        }

        m->textures.set("roughness_map", texes[path.C_Str()]);
        m->uniforms.set("has_roughness_map", 1);
    }
    else
    {
        m->uniforms.set("has_roughness_map", 0);
    }

    // Check for metalness map
    if (mat->GetTextureCount(aiTextureType_METALNESS) > 0)
    {
        aiString path;
        mat->GetTexture(aiTextureType_METALNESS, 0, &path);

        if (texes.find(path.C_Str()) == texes.end())
        {
            texes[path.C_Str()] = loadTexture(path.C_Str(), scene, renderer);
        }

        m->textures.set("metal_map", texes[path.C_Str()]);
        m->uniforms.set("has_metal_map", 1);
    }
    else
    {
        m->uniforms.set("has_metal_map", 0);
    }

    std::cout << "Roughness: " << roughness << " Metalness: " << metallic << "\n";

    return new_mat;
}

std::vector<Frender::RenderObjectRef> loadModel(Frender::Renderer* renderer, const std::string& filename)
{
    Assimp::Importer importer;

    const struct aiScene* scene = importer.ReadFile(filename, aiProcess_CalcTangentSpace |
                aiProcess_Triangulate            |
                aiProcess_JoinIdenticalVertices  |
                // aiProcess_FlipUVs                |
                aiProcess_CalcTangentSpace       |
                aiProcess_GenNormals             |
                aiProcess_OptimizeMeshes         | // This may break things
                aiProcess_SortByPType);
    
    if (!scene)
    {
        // :(
        std::cerr << "Failed to load asset: " << std::string(importer.GetErrorString()) << "\n";
    }

    // Iterate through node list
    std::map<unsigned int, Frender::MeshRef> meshes;
    std::map<unsigned int, uint32_t> materials;
    std::map<std::string, Frender::Texture> texes;

    std::queue<aiNode*> nodes;
    nodes.emplace(scene->mRootNode);

    std::vector<Frender::RenderObjectRef> render_objects;

    while (nodes.size() != 0)
    {
        auto node = nodes.front();
        nodes.pop();

        auto global_transform = getNodeTransform(node);

        // Process mesh
        if (node->mNumMeshes > 0)
        {
            for (int i = 0; i < node->mNumMeshes; i++)
            {
                if (meshes.find(node->mMeshes[i]) == meshes.end())
                {
                    meshes[node->mMeshes[i]] = loadMesh(node->mMeshes[i], scene, renderer);
                }

                auto mesh = scene->mMeshes[node->mMeshes[i]];
                if (materials.find(mesh->mMaterialIndex) == materials.end())
                {
                    materials[mesh->mMaterialIndex] = loadMaterial(mesh->mMaterialIndex, scene, renderer, texes);
                }

                auto fmesh = meshes[node->mMeshes[i]];
                auto fmat = materials[mesh->mMaterialIndex];

                auto obj = renderer->createRenderObject(fmesh, fmat, global_transform);
                render_objects.push_back(obj);
                std::cout << "Made object\n";
            }
        }

        // Add children to the queue
        for (int i = 0; i < node->mNumChildren; i++)
        {
            nodes.emplace(node->mChildren[i]);
        }
    }

    return render_objects;
}