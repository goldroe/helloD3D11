
#ifndef GRID_H
#define GRID_H

#include <DirectXMath.h>
#include <vector>
#include <stdint.h>

struct MeshData {
    DirectX::XMFLOAT3 *vertices;
    uint32_t *indices;
    uint32_t vertex_count;
    uint32_t index_count;
};

void
generate_grid(float width, float height, uint32_t m, uint32_t n, MeshData *mesh_data);

float
get_height(float x, float z);

#endif
