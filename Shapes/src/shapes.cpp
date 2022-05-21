
#include "shapes.h"

void
build_cylinder_top(float height, float top_radius, float bottom_radius, uint32_t stack_count, uint32_t slice_count, MeshData *mesh_data) {
    uint32_t base_index = (uint32_t)mesh_data->vertices.size();
    float y = 0.5f * height;
    float dtheta = 2.0f * DirectX::XM_PI / slice_count;

    // duplicate cap ring vertices
    for (uint32_t i = 0; i <= slice_count; i++) {
        float x = top_radius * cosf(i * dtheta);
        float z = top_radius * sinf(i * dtheta);
        mesh_data->vertices.push_back(DirectX::XMFLOAT3(x, y, z));
    }

    // cap center vertex
    mesh_data->vertices.push_back(DirectX::XMFLOAT3(0.0f, y, 0.0f));

    // index of center vertex
    uint32_t center_index = (uint32_t)mesh_data->vertices.size() - 1;

    for (uint32_t i = 0; i < slice_count; i++) {
        mesh_data->indices.push_back(center_index);
        mesh_data->indices.push_back(base_index + i + 1);
        mesh_data->indices.push_back(base_index + i);
    }
}

void
build_cylinder_bottom(float height, float top_radius, float bottom_radius, uint32_t stack_count, uint32_t slice_count, MeshData *mesh_data) {
    uint32_t base_index = (uint32_t)mesh_data->vertices.size();
    float y = -0.5f * height;
    float dtheta = 2.0f * DirectX::XM_PI / slice_count;

    // duplicate cap ring vertices
    for (uint32_t i = 0; i <= slice_count; i++) {
        float x = bottom_radius * cosf(i * dtheta);
        float z = bottom_radius * sinf(i * dtheta);
        mesh_data->vertices.push_back(DirectX::XMFLOAT3(x, y, z));
    }
    // cap center vertex
    mesh_data->vertices.push_back(DirectX::XMFLOAT3(0.0f, y, 0.0f));

    // index of center vertex
    uint32_t center_index = (uint32_t)mesh_data->vertices.size() - 1;

    for (uint32_t i = 0; i < slice_count; i++) {
        mesh_data->indices.push_back(center_index);
        mesh_data->indices.push_back(base_index + i);
        mesh_data->indices.push_back(base_index + i + 1);
    }
}

void
generate_cylinder(float height, float top_radius, float bottom_radius, uint32_t stack_count, uint32_t slice_count, MeshData *mesh_data) {
    mesh_data->vertices.clear();
    mesh_data->indices.clear();

    uint32_t ring_count = stack_count + 1;

    // compute vertices for each ring of the cylinder
    float radius_step = (top_radius - bottom_radius) / stack_count;
    float stack_height = height / stack_count;
    for (uint32_t i = 0; i < ring_count; i++) {
        float y = -0.5f * height + stack_height * i;
        float r = bottom_radius - radius_step * i;
        float dtheta = 2.0f*DirectX::XM_PI / slice_count;
        for (uint32_t j = 0; j <= slice_count; j++) {
            float x = r * cosf(j * dtheta);
            float z = r * sinf(j * dtheta);
            DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(x, y, z);
            mesh_data->vertices.push_back(position);
        }
    }

    uint32_t ring_vertex_count = slice_count + 1;
    // compute indices
    for (uint32_t i = 0; i < stack_count; i++) {
        for (uint32_t j = 0; j < slice_count; j++) {
            mesh_data->indices.push_back(i * ring_vertex_count + j);
            mesh_data->indices.push_back((i + 1) * ring_vertex_count + j);
            mesh_data->indices.push_back((i + 1) * ring_vertex_count + j + 1);
            mesh_data->indices.push_back(i * ring_vertex_count + j);
            mesh_data->indices.push_back((i + 1) * ring_vertex_count + j + 1);
            mesh_data->indices.push_back(i * ring_vertex_count + j + 1);
        }
    }

    build_cylinder_top(height, top_radius, bottom_radius, stack_count, slice_count, mesh_data);
    build_cylinder_bottom(height, top_radius, bottom_radius, stack_count, slice_count, mesh_data);
}
