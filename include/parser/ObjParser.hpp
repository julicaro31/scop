#ifndef OBJ_PARSER_HPP
#define OBJ_PARSER_HPP

#include "Mesh.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

/// @brief Parses Wavefront .obj files into a Mesh.
///
/// Supported line types:
///   v  x y z          — vertex position
///   f  v1 v2 v3 ...   — face (triangulated via fan if > 3 vertices)
///   f  v1/vt1 ...     — face with texture coords (texture coords ignored)
///   f  v1/vt1/vn1 ... — face with tex + normals  (both ignored)
///   f  v1//vn1 ...    — face with normals only    (normals ignored)
///   #  ...            — comment (skipped)
///
/// Everything else (normals, materials, groups, etc.) is silently ignored.
class ObjParser
{
public:
    /// @brief Parses the .obj file at the given path and returns a Mesh.
    /// @param filepath Path to the .obj file.
    /// @return A Mesh containing the parsed vertex and index data.
    /// @throws std::runtime_error if the file cannot be opened or contains no geometry.
    static Mesh parse(const std::string &filepath);

private:
    /// @brief Extracts the vertex index from a face token like "3", "3/1", "3/1/2", or "3//2".
    ///        OBJ indices are 1-based; this returns a 0-based index.
    static int parseFaceVertex(const std::string &token);
};

#endif
