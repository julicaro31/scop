#include "Texture.hpp"

#include <fstream>
#include <stdexcept>
#include <cstdint>

// ----------------------------------------------------------------------------
// Little-endian readers.
// BMP stores all multi-byte numbers least-significant-byte first, regardless of
// the CPU. Reading byte-by-byte makes the loader correct on any machine and
// avoids struct-packing surprises.
// ----------------------------------------------------------------------------
static uint32_t le32(const unsigned char *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static int32_t le32s(const unsigned char *p)
{
    return (int32_t)le32(p);
}

static uint16_t le16(const unsigned char *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

std::vector<unsigned char>
Texture::loadBMP(const std::string &path, int &outWidth, int &outHeight)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Could not open texture: " + path);

    std::vector<unsigned char> raw(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    // 14-byte file header + 40-byte info header = 54 bytes minimum.
    if (raw.size() < 54)
        throw std::runtime_error("File too small to be a BMP: " + path);
    if (raw[0] != 'B' || raw[1] != 'M')
        throw std::runtime_error("Not a BMP (bad magic): " + path);

    uint32_t dataOffset = le32(&raw[10]); // where pixel bytes start
    int32_t width = le32s(&raw[18]);
    int32_t height = le32s(&raw[22]);      // negative => rows are top-down
    uint16_t bpp = le16(&raw[28]);         // bits per pixel
    uint32_t compression = le32(&raw[30]); // 0 == BI_RGB (uncompressed)

    if (bpp != 24)
        throw std::runtime_error("Only 24-bit BMPs are supported: " + path);
    if (compression != 0)
        throw std::runtime_error("Only uncompressed BMPs are supported: " + path);
    if (width <= 0 || height == 0)
        throw std::runtime_error("Bad BMP dimensions: " + path);

    bool topDown = height < 0;
    int h = topDown ? -height : height;
    int w = width;

    // Each scanline is padded up to a multiple of 4 bytes.
    size_t rowStride = (((size_t)w * 3 + 3) / 4) * 4;
    size_t needed = (size_t)dataOffset + rowStride * (size_t)h;
    if (raw.size() < needed)
        throw std::runtime_error("BMP pixel data is truncated: " + path);

    // Output: tightly packed RGB, written bottom-to-top so it matches OpenGL.
    std::vector<unsigned char> rgb((size_t)w * (size_t)h * 3);
    for (int y = 0; y < h; ++y)
    {
        int srcRow = topDown ? (h - 1 - y) : y;
        const unsigned char *src = &raw[dataOffset + (size_t)srcRow * rowStride];
        unsigned char *dst = &rgb[(size_t)y * (size_t)w * 3];
        for (int x = 0; x < w; ++x)
        {
            dst[x * 3 + 0] = src[x * 3 + 2]; // R  (BMP stores B,G,R)
            dst[x * 3 + 1] = src[x * 3 + 1]; // G
            dst[x * 3 + 2] = src[x * 3 + 0]; // B
        }
    }

    outWidth = w;
    outHeight = h;
    return rgb;
}

Texture::Texture(const std::string &path) : _id(0)
{
    int width = 0;
    int height = 0;
    std::vector<unsigned char> pixels = loadBMP(path, width, height);

    glGenTextures(1, &_id);
    glBindTexture(GL_TEXTURE_2D, _id);

    // Our RGB rows are tightly packed (width*3 bytes, not 4-byte aligned), so
    // tell OpenGL not to assume 4-byte row alignment, or it would read skewed.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Wrapping: what to sample when a UV falls outside [0, 1]. REPEAT tiles it.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Filtering: how to blend texels when the texture is scaled on screen.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 width, height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Mipmaps: pre-shrunk copies used when the model is far away, which removes
    // the sparkly aliasing you get sampling a big texture into few pixels.
    glGenerateMipmap(GL_TEXTURE_2D);
}

Texture::~Texture()
{
    if (_id)
        glDeleteTextures(1, &_id);
}

void Texture::bind(GLuint unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, _id);
}
