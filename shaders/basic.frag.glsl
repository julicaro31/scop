#version 330 core

// The fragment shader's job: decide the COLOR of each pixel.
// "out vec4 FragColor" is our output — a 4-component color (R, G, B, Alpha).
out vec4 FragColor;

void main()
{
    // Set every pixel to an orange color.
    // Values are 0.0 to 1.0 (not 0-255 like in CSS).
    // (1.0, 0.5, 0.2) = full red, half green, a bit of blue = orange
    // Alpha 1.0 = fully opaque
    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
