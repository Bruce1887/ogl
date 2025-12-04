// Text fragment shader for rendering glyphs
// Uses a single-channel texture atlas for font rendering

#version 400 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D u_textTexture;
uniform vec4 u_textColor;

void main()
{
    float alpha = texture(u_textTexture, TexCoords).r;
    FragColor = vec4(u_textColor.rgb, u_textColor.a * alpha);
}
