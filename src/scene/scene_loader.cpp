// Copyright 2018 Mihail Mladenov
//
// This file is part of bpmap.
//
// bpmap is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// bpmap is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with bpmap.  If not, see <http://www.gnu.org/licenses/>.

#include <cctype>
#include <cstring>
#include <cstdlib>
#include <sstream>

#include <io.hpp>
#include <algebra.hpp>

#define INI_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION

#include <ini.h>
#include <tiny_obj_loader.h>

#include "scene_loader.hpp"


namespace bpmap
{
    darray_t<string_t> split(const string_t& str, char_t delim)
    {
        darray_t<string_t> result;

        std::stringstream ss(str);

        string_t tmp;

        while(std::getline(ss,tmp, delim))
        {
            result.push_back(tmp);
        }

        return result;
    }

    class scene_loader_t
    {
        ini_t* parsed;
        scene_t* scene;
        error_t success;

    public:
        error_t is_loaded() { return success; }

        scene_loader_t(const darray_t<uint8_t>& sd, scene_t& s)
        {
            scene = &s;
            success = error_t::success;

            parsed = ini_load((const char_t*) sd.data(), nullptr);

            if(!parsed)
            {
                success = error_t::scene_settings_read_fail;
                return;
            }

            success = load_settings();

            if(success != error_t::success)
            {
                return;
            }

            success = load_camera();

            if(success != error_t::success)
            {
                return;
            }

            success = load_lights();

            if(success != error_t::success)
            {
                return;
            }

            success = load_objects();

            if(success != error_t::success)
            {
                return;
            }

        }

        ~scene_loader_t()
        {
            ini_destroy(parsed);
        }

    private:

        error_t load_settings()
        {
            static constexpr const char* global_settings_section_name = "settings";

            auto global_settings_section = ini_find_section(
                                                             parsed,
                                                             global_settings_section_name,
                                                             strlen(global_settings_section_name)
                                                           );

            if(global_settings_section == INI_NOT_FOUND)
            {
                return error_t::global_settings_load_fail;
            }

            auto resolution_x = get_value(global_settings_section, "resolution_x");
            auto resolution_y = get_value(global_settings_section, "resolution_y");
            auto samples_per_pixel = get_value(global_settings_section, "samples_per_pixel");
            auto light_samples = get_value(global_settings_section, "light_samples");
            auto max_reflection_bounces = get_value(global_settings_section, "max_reflection_bounces");

            scene->settings.resolution_x = strtof(resolution_x.c_str(), nullptr);
            scene->settings.resolution_y = strtof(resolution_y.c_str(), nullptr);
            scene->settings.samples_per_pixel = strtoul(samples_per_pixel.c_str(), nullptr, 10);
            scene->settings.light_samples =  strtoul(light_samples.c_str(), nullptr, 10);
            scene->settings.max_reflection_bounces =  strtoul(max_reflection_bounces.c_str(), nullptr, 10);

            return error_t::success;
        }


        error_t load_lights()
        {
            static constexpr const char* lightsectn = "lights";

            auto lights_section = ini_find_section(parsed, lightsectn, strlen(lightsectn));

            if(lights_section == INI_NOT_FOUND)
            {
                return error_t::lights_load_fail;
            }

            static constexpr const char* point_of_rect_light = "point";
            static constexpr const char* normal_of_rect_light = "normal";
            static constexpr const char* parameter0_max_of_rect_light = "param0_max";
            static constexpr const char* parameter1_max_of_rect_light = "param1_max";
            static constexpr const char* basis_vec0_of_rect_light = "basis_vec0";
            static constexpr const char* basis_vec1_of_rect_light = "basis_vec1";
            static constexpr const char* power_of_rect_light = "power";


            for(auto i = 0; ; ++i)
            {
                auto light_number = std::to_string(i);

                auto point = get_value(lights_section, point_of_rect_light + light_number);
                auto normal = get_value(lights_section, normal_of_rect_light + light_number);
                auto param0_max = get_value(lights_section, parameter0_max_of_rect_light + light_number);
                auto param1_max = get_value(lights_section, parameter1_max_of_rect_light + light_number);
                auto basis0 = get_value(lights_section, basis_vec0_of_rect_light + light_number);
                auto basis1 = get_value(lights_section, basis_vec1_of_rect_light + light_number);
                auto power = get_value(lights_section, power_of_rect_light + light_number);

                if(
                    point.empty() ||
                    normal.empty() ||
                    param0_max.empty() ||
                    param1_max.empty() ||
                    basis0.empty() ||
                    basis1.empty() ||
                    power.empty()
                  )
                {
                    break;
                }

                light_t light;
                light.point = parse_point(point);
                light.normal = parse_point(normal);
                light.basis_vec0 = parse_point(basis0);
                light.basis_vec1 = parse_point(basis1);
                light.param0_max = strtof(param0_max.c_str(), nullptr);
                light.param1_max = strtof(param1_max.c_str(), nullptr);
                light.power = strtof(power.c_str(), nullptr);

                scene->lights.push_back(light);
            }

            return error_t::success;
        }


        error_t load_objects()
        {
            static constexpr const char* objsectn = "objects";

            auto objects_section = ini_find_section(parsed, objsectn, strlen(objsectn));

            if(objects_section == INI_NOT_FOUND)
            {
                return error_t::objects_load_fail;
            }

            string_t path_of_object = "objp";
            string_t transform_of_object = "objt";

            darray_t<pair_t<string_t, string_t>> objects;

            for(auto i = 0; ; ++i)
            {
                auto obj_number = std::to_string(i);

                auto obj_path_id = ini_find_property(
                                                      parsed,
                                                      objects_section,
                                                      (path_of_object + obj_number).c_str(),
                                                      path_of_object.size() + obj_number.size()
                                                    );

                if(obj_path_id == INI_NOT_FOUND)
                {
                    break;
                }

                string_t obj_path = ini_property_value(parsed, objects_section, obj_path_id);

                auto obj_transform_id = ini_find_property(
                                                           parsed,
                                                           objects_section,
                                                           (transform_of_object + obj_number).c_str(),
                                                           transform_of_object.size() + obj_number.size()
                                                         );

                auto transform = ini_property_value(parsed, objects_section, obj_transform_id);
                string_t obj_transform = (transform)? transform : "";

                objects.push_back(std::make_pair(obj_path, obj_transform));
            }

            for(auto& path_transform_pair: objects)
            {
                auto status = parse_object(path_transform_pair.first, path_transform_pair.second);

                if(status != error_t::success)
                {
                    return status;
                }
            }

            return error_t::success;
        }

        error_t parse_object(const string_t& path, const string_t& transform)
        {
            string_t err;

            tinyobj::attrib_t attributes;
            darray_t<tinyobj::material_t> materials;
            darray_t<tinyobj::shape_t> shapes;

            if(!tinyobj::LoadObj(&attributes, &shapes, &materials, &err, path.c_str()))
            {
                return error_t::objects_load_fail;
            }

            auto vertex_offset = scene->vertices.size();
            auto normal_offset = scene->normals.size();
            auto texcoord_offset = scene->texcoords.size();

            scene->vertices.resize(vertex_offset + attributes.vertices.size());
            scene->normals.resize(normal_offset + attributes.normals.size());
            scene->texcoords.resize(texcoord_offset + attributes.texcoords.size());

            memcpy(
                    scene->vertices.data() + vertex_offset,
                    attributes.vertices.data(),
                    attributes.vertices.size()
                   );
            memcpy(
                    scene->normals.data() + normal_offset,
                    attributes.normals.data(),
                    attributes.normals.size()
                  );
            memcpy(
                    scene->texcoords.data() + texcoord_offset,
                    attributes.texcoords.data(),
                    attributes.texcoords.size()
                  );

            for(auto& shape: shapes)
            {
                for(auto i = 0; i + 3 < shape.mesh.indices.size(); i+=3)
                {
                    triangle_t t;
                    t.vertices[0].vertex_index = shape.mesh.indices[i].vertex_index + vertex_offset;
                    t.vertices[0].normal_index = shape.mesh.indices[i].normal_index + normal_offset;
                    t.vertices[0].texcoord_index = shape.mesh.indices[i].texcoord_index + texcoord_offset;
                    t.vertices[1].vertex_index = shape.mesh.indices[i + 1].vertex_index + vertex_offset;
                    t.vertices[1].normal_index = shape.mesh.indices[i + 1].normal_index + normal_offset;
                    t.vertices[1].texcoord_index = shape.mesh.indices[i + 1].texcoord_index + texcoord_offset;
                    t.vertices[2].vertex_index = shape.mesh.indices[i + 2].vertex_index + vertex_offset;
                    t.vertices[2].normal_index = shape.mesh.indices[i + 2].normal_index + normal_offset;
                    t.vertices[2].texcoord_index = shape.mesh.indices[i + 2].texcoord_index + texcoord_offset;
                    t.material_id = shape.mesh.material_ids[i/3];

                    scene->triangles.push_back(t);
                }
            }

            for(auto& m: materials)
            {
                material_t material;
                material.base_color[0] = m.diffuse[0];
                material.base_color[1] = m.diffuse[1];
                material.base_color[2] = m.diffuse[2];
                material.metallic = m.metallic;
                material.roughness = m.roughness;

                scene->materials.push_back(material);
            }

            return error_t::success;
        }

        error_t load_camera()
        {
            static constexpr const char* camerasectn = "camera";
            static constexpr const char* uppropname = "up";
            static constexpr const char* leftpropname = "left";
            static constexpr const char* frontpropname = "lookat";
            static constexpr const char* originpropname = "origin";
            static constexpr const char* arpropname = "aspect_ratio";
            static constexpr const char* nearpropname = "near";
            static constexpr const char* farpropname = "far";
            static constexpr const char* fovpropname = "far";

            auto camera_section = ini_find_section(parsed, camerasectn, strlen(camerasectn));

            if(camera_section == INI_NOT_FOUND)
            {
                return error_t::objects_load_fail;
            }


            auto up = get_value(camera_section, uppropname);
            auto left = get_value(camera_section, leftpropname);
            auto front = get_value(camera_section, frontpropname);
            auto origin = get_value(camera_section, originpropname);
            auto aspect_ratio = get_value(camera_section, arpropname);
            auto near = get_value(camera_section, nearpropname);
            auto far = get_value(camera_section, farpropname);
            auto fov = get_value(camera_section, fovpropname);

            scene->settings.camera.up = parse_point(up);
            scene->settings.camera.left = parse_point(left);
            scene->settings.camera.front = parse_point(front);
            scene->settings.camera.origin = parse_point(origin);
            scene->settings.camera.aspect_ratio = strtof(aspect_ratio.c_str(), nullptr);
            scene->settings.camera.near = strtof(near.c_str(), nullptr);
            scene->settings.camera.far = strtof(far.c_str(), nullptr);
            scene->settings.camera.field_of_view = strtof(fov.c_str(), nullptr);

            return error_t::success;
        }

        string_t get_value(int32_t section, const string_t& property)
        {
           auto property_id = ini_find_property(parsed, section, property.c_str(), property.size());
           auto value = ini_property_value(parsed, section, property_id);

           return (value)? string_t(value) : string_t();
        }

        point3d_t parse_point(const string_t& p)
        {
            point3d_t result;
            auto tokens = split(p, ' ');

            if(tokens.size() < 3)
            {
                result.components[0] = 0;
                result.components[1] = 0;
                result.components[2] = 0;

                return result;
            }

            auto x = strtof(tokens[0].c_str(), nullptr);
            auto y = strtof(tokens[1].c_str(), nullptr);
            auto z = strtof(tokens[2].c_str(), nullptr);

            result.components[0] = x;
            result.components[1] = y;
            result.components[2] = z;

            return result;
        }
    };

    error_t load_scene(const string_t& path, scene_t& scene)
    {
        darray_t<uint8_t> scene_description;

        if(!read_whole_file(path, scene_description))
        {
            return error_t::scene_settings_read_fail;
        }

        scene_description.push_back('\0');

        return scene_loader_t(scene_description, scene).is_loaded();
    }
}
