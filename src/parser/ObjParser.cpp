#include "ObjParser.hpp"

int ObjParser::parseFaceVertex(const std::string &token)
{
    std::string indexStr;
    for (size_t i = 0; i < token.size(); ++i)
    {
        if (token[i] == '/')
        {
            break;
        }
        indexStr += token[i];
    }

    return std::stoi(indexStr);
}

Mesh ObjParser::parse(const std::string &filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        throw std::runtime_error("OBJ ERROR: Could not open file: " + filepath);
    }

    Mesh mesh;

    std::vector<Vec3> positions;

    std::string line;
    int lineNum = 0;

    while (std::getline(file, line))
    {
        lineNum++;

        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v")
        {
            float x, y, z;
            if (!(iss >> x >> y >> z))
            {
                std::cerr << "OBJ WARNING: Malformed vertex at line " << lineNum << "\n";
                continue;
            }
            positions.push_back(Vec3(x, y, z));
        }
        else if (prefix == "f")
        {
            std::vector<int> faceIndices;
            std::string token;

            while (iss >> token)
            {
                int idx = parseFaceVertex(token);

                idx = idx >= 0 ? idx - 1 : idx + static_cast<int>(positions.size());

                if (idx < 0 || idx >= static_cast<int>(positions.size()))
                {
                    std::cerr << "OBJ WARNING: Vertex index out of range at line " << lineNum << "\n";
                    faceIndices.clear();
                    break;
                }

                faceIndices.push_back(idx);
            }

            // Fan triangulation
            for (size_t i = 1; i + 1 < faceIndices.size(); ++i)
            {
                mesh.indices.push_back(static_cast<unsigned int>(faceIndices[0]));
                mesh.indices.push_back(static_cast<unsigned int>(faceIndices[i]));
                mesh.indices.push_back(static_cast<unsigned int>(faceIndices[i + 1]));
            }
        }
    }

    file.close();

    if (positions.empty())
    {
        throw std::runtime_error("OBJ ERROR: No vertices found in: " + filepath);
    }

    if (mesh.indices.empty())
    {
        throw std::runtime_error("OBJ ERROR: No faces found in: " + filepath);
    }

    // Flatten positions into the mesh's float array
    mesh.vertices.reserve(positions.size() * 3);
    for (const Vec3 &p : positions)
    {
        mesh.vertices.push_back(p.x);
        mesh.vertices.push_back(p.y);
        mesh.vertices.push_back(p.z);
    }

    std::cout << "OBJ: Loaded " << filepath << " — "
              << mesh.vertexCount() << " vertices, "
              << mesh.triangleCount() << " triangles\n";

    return mesh;
}
