#version 400 core
layout(location = 0) in vec3 position;
out vec3 TexCoord;
uniform mat4 view;
uniform mat4 projection;

void main(){
    TexCoord = position;
    gl_Position = projection * view * vec4(position, 1.0);
}