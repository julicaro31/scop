#include <glad/glad.h>

class ShaderObject
{
public:
	ShaderObject(GLenum type) : id(glCreateShader(type)) {}
	~ShaderObject()
	{
		if (id)
		{
			glDeleteShader(id);
		}
	}

	ShaderObject(const ShaderObject &) = delete;
	ShaderObject &operator=(const ShaderObject &) = delete;

	GLuint get() const { return id; }

private:
	GLuint id;
};