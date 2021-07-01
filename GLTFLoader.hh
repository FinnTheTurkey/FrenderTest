#ifndef GLTFLOADER_HH
#define GLTFLOADER_HH

#include "Frender/Frender.hh"

std::vector<Frender::RenderObjectRef> loadModel(Frender::Renderer* renderer, const std::string& filename);

#endif