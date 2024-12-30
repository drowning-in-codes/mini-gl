#version 400 core
in vec2  TextCoord;
out vec4 FragColor;
uniform sampler2D texture1;
void main(){
    FragColor = texture(texture1,TextCoord);
}