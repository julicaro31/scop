#version 330 core

in vec3 vColor;
in vec2 vUV;

uniform sampler2D tex;     // the texture, read from texture unit 0
uniform float useTexture;  // 0.0 = per-face grey, 1.0 = texture

out vec4 FragColor;

void main()
{
    vec3 texColor = texture(tex, vUV).rgb;
    vec3 rgb = mix(vColor, texColor, useTexture);
    FragColor = vec4(rgb, 1.0);
}
