#pragma once
#include "Vec3.hpp"
#include <vector>
#include <limits>

/// @brief Holds the GPU-ready data extracted from an .obj file.
///        vertices is a flat array of floats (x, y, z per vertex).
///        indices  maps triangles into that vertex array (3 per triangle).
struct Mesh
{
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    /// @brief Returns the number of unique vertex positions.
    size_t vertexCount() const { return vertices.size() / 3; }

    /// @brief Returns the number of triangles.
    size_t triangleCount() const { return indices.size() / 3; }

    /// @brief Computes the axis-aligned bounding box center.
    Vec3 center() const
    {
        if (vertices.empty())
        {
            return Vec3(0, 0, 0);
        }

        float minX = std::numeric_limits<float>::max();
        float minY = std::numeric_limits<float>::max();
        float minZ = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float maxY = std::numeric_limits<float>::lowest();
        float maxZ = std::numeric_limits<float>::lowest();

        for (size_t i = 0; i < vertices.size(); i += 3)
        {
            if (vertices[i] < minX)
                minX = vertices[i];
            if (vertices[i] > maxX)
                maxX = vertices[i];
            if (vertices[i + 1] < minY)
                minY = vertices[i + 1];
            if (vertices[i + 1] > maxY)
                maxY = vertices[i + 1];
            if (vertices[i + 2] < minZ)
                minZ = vertices[i + 2];
            if (vertices[i + 2] > maxZ)
                maxZ = vertices[i + 2];
        }

        return Vec3((minX + maxX) * 0.5f,
                    (minY + maxY) * 0.5f,
                    (minZ + maxZ) * 0.5f);
    }

    /// @brief Computes the maximum extent (half-diagonal of the AABB).
    ///        Useful for normalizing the model to fit in a unit cube.
    float extent() const
    {
        if (vertices.empty())
        {
            return 1.0f;
        }

        float minX = std::numeric_limits<float>::max();
        float minY = std::numeric_limits<float>::max();
        float minZ = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float maxY = std::numeric_limits<float>::lowest();
        float maxZ = std::numeric_limits<float>::lowest();

        for (size_t i = 0; i < vertices.size(); i += 3)
        {
            if (vertices[i] < minX)
                minX = vertices[i];
            if (vertices[i] > maxX)
                maxX = vertices[i];
            if (vertices[i + 1] < minY)
                minY = vertices[i + 1];
            if (vertices[i + 1] > maxY)
                maxY = vertices[i + 1];
            if (vertices[i + 2] < minZ)
                minZ = vertices[i + 2];
            if (vertices[i + 2] > maxZ)
                maxZ = vertices[i + 2];
        }

        float dx = maxX - minX;
        float dy = maxY - minY;
        float dz = maxZ - minZ;

        float maxDim = dx;
        if (dy > maxDim)
            maxDim = dy;
        if (dz > maxDim)
            maxDim = dz;

        return maxDim * 0.5f;
    }

    /// @brief Produces a de-indexed, per-triangle-colored vertex array.
    ///        Layout per vertex: x, y, z, r, g, b (6 floats). No index array needed.
    std::vector<float> expandedColoredVertices() const
    {
        std::vector<float> out;
        out.reserve(indices.size() * 6);

        for (size_t i = 0; i + 2 < indices.size(); i += 3)
        {
            size_t triIndex = i / 3;

            float t = static_cast<float>(triIndex) * 0.61803398875f;
            t = t - std::floor(t);          // fractional part, [0, 1)
            float grey = 0.25f + 0.60f * t; // [0.25, 0.85]

            for (int j = 0; j < 3; ++j)
            {
                unsigned int v = indices[i + j];
                out.push_back(vertices[v * 3 + 0]);
                out.push_back(vertices[v * 3 + 1]);
                out.push_back(vertices[v * 3 + 2]);
                out.push_back(grey);
                out.push_back(grey);
                out.push_back(grey);
            }
        }

        return out;
    }
};