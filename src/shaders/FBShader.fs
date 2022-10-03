#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform float init;
uniform float shift;
uniform sampler2D screenTexture;
uniform vec3 kernelTop;
uniform vec3 kernelMid;
uniform vec3 kernelBottom;
uniform float offset;

void main()
{
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), 
        vec2( 0.0f,    offset),
        vec2( offset,  offset), 
        vec2(-offset,  0.0f),  
        vec2( 0.0f,    0.0f), 
        vec2( offset,  0.0f),
        vec2(-offset, -offset),
        vec2( 0.0f,   -offset),
        vec2( offset, -offset) 
    );

    float kernel[9] = float[](kernelTop.x, kernelTop.y, kernelTop.z,
                              kernelMid.x,kernelMid.y, kernelMid.z,
                              kernelBottom.x, kernelBottom.y, kernelBottom.z);

    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += vec3(texture(screenTexture, TexCoords.st + offsets[i])) * kernel[i];
        
    FragColor = vec4(col, 1.0);
}