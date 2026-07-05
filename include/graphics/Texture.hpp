#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <glad/glad.h>
#include <string>
#include <vector>

class Texture
{
public:
    Texture(const std::string &path);
    ~Texture();

    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;

    void bind(GLuint unit = 0) const;

private:
    GLuint _id;

    static std::vector<unsigned char> loadBMP(const std::string &path, int &outWidth, int &outHeight);
};

#endif
