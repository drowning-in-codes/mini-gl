#version 400 core
in vec3  TexCoord; 
out vec4 FragColor;

uniform samplerCube cubeTexture;
void main(){
    gl_FragColor = texture(cubeTexture, TexCoord);
}