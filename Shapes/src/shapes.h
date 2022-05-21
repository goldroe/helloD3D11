
#ifndef SHAPES_H
#define SHAPES_H

#include <DirectXMath.h>
#include <vector>
#include <stdint.h>

struct MeshData {
    std::vector<DirectX::XMFLOAT3> vertices;
    std::vector<uint32_t> indices;
};

void
generate_cylinder(float height, float top_radius, float bottom_radius, uint32_t stack_count, uint32_t slice_count, MeshData *mesh_data);

#endif
