#ifndef SCENE_LOADER_HPP
#define SCENE_LOADER_HPP

#include "scene.hpp"
#include <common.hpp>


namespace bpmap
{
    bool load_scene(const string_t& path, scene_t& scene);
}

#endif // SCENE_LOADER_HPP
