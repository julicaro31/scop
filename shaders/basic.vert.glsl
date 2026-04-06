#version 330 core

// "layout (location = 0)" tells OpenGL:
// "This input variable corresponds to vertex attribute index 0."
// This must match what you set up with glVertexAttribPointer(0, ...).
layout (location = 0) in vec3 aPos;

void main()
{
    // gl_Position is a built-in output variable.
    // It tells the GPU where this vertex should appear on screen.
    // We wrap our 3D position in a vec4 because OpenGL uses
    // 4D "homogeneous coordinates" (the 4th component, w, is usually 1.0).
    gl_Position = vec4(aPos, 1.0);
}
