#include "grid.h"

float
get_height(float x, float z) {
    return 0.3f * (z*sinf(0.1f*x) + x*cosf(0.1f * z));
}

void
generate_grid(float width, float depth, uint32_t m, uint32_t n, MeshData *mesh_data) {
    uint32_t vertex_count = m * n;
    uint32_t face_count = (m-1) * (n-1) * 2;

    float dx = width / (m - 1);
    float dz = depth / (n - 1);

    // Generate vertex buffer
    mesh_data->vertices = new DirectX::XMFLOAT3[vertex_count];
    mesh_data->vertex_count = vertex_count;
    for (uint32_t i = 0; i < m; i++) {
        float z = 0.5f * depth - i * dz;
        for (uint32_t j = 0; j < n; j++) {
            float x = -0.5f * width + j * dx;
            mesh_data->vertices[i*n + j] = DirectX::XMFLOAT3(x, 0.0f, z);
        }
    }

    // Generate index buffer
    mesh_data->index_count = face_count * 3;
    mesh_data->indices = new uint32_t[face_count * 3];
    uint32_t k = 0;
    for (uint32_t i = 0; i < m - 1; i++) {
        for (uint32_t j = 0; j < n - 1; j++) {
            mesh_data->indices[k] = i * n + j;
            mesh_data->indices[k + 1] = i * n + j + 1;
            mesh_data->indices[k + 2] = (i + 1) * n + j;
            mesh_data->indices[k + 3] = (i + 1) * n + j;
            mesh_data->indices[k + 4] = i * n + j + 1;
            mesh_data->indices[k + 5] = (i + 1) * n + j + 1;
            k += 6;
        }
    }
}
