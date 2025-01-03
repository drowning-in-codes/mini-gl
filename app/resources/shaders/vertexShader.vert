#version 400 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
// out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec2 TextCoord;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
// uniform vec2 coord_trans;
void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection*view*vec4(FragPos, 1.0f);
    // TexCoord = aTexCoord;
    Normal = mat3(transpose(inverse(model)))*aNormal;
    TextCoord = aTexCoords;
}
