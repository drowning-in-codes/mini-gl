#version 400 core
in vec2  TextCoord;
in vec3 Normal;
in vec3 FragPos;
uniform vec3 viewPos;
out vec4 FragColor;
uniform sampler2D texture1;
void main(){
    vec4 textColor = texture(texture1,TextCoord);
    if (textColor.a < 0.1) {
        discard;
    }
    float dis = distance(FragPos,viewPos);
    if(dis> 15.0){
        discard;
    }
    FragColor = textColor;
}