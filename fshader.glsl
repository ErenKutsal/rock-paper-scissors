#version 150

uniform vec3 objectColor;
out vec4 fColor;

void main() 
{
    fColor = vec4(objectColor, 1.0); 
}