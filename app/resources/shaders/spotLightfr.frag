#version 400 core
out vec4 FragColor;
in vec3 Normal;
in vec3 FragPos;
uniform vec3 viewPos;
struct Material {
    sampler2D specular;
    sampler2D diffuse;
    float shininess;
};
struct SpotLight {
     vec3 position;
     vec3 spotDir;

     vec3 ambient;
     vec3 diffuse;
     vec3 specular;

     float cutOff;
     float outerCutOff;
};
in vec2 TextCoord;
uniform SpotLight light;
uniform Material material;
void main() {
     vec3 lightDir = normalize(light.position - FragPos);
     float theta = dot(lightDir, normalize(-light.spotDir));

    if(theta > light.cutOff){
        float epsilon = light.cutOff - light.outerCutOff;
        float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
     vec3 ambient = light.ambient * texture(material.diffuse, TextCoord).rgb;
     vec3 norm = normalize(Normal);
     float diff = max(dot(lightDir,norm),0.0);
     float spec = pow(max(dot(reflect(-lightDir, Normal), normalize(viewPos - FragPos)), 0.0),material.shininess);
        vec3 diffuse = light.diffuse *diff* vec3(texture(material.diffuse, TextCoord))*intensity;
        vec3 specular = light.specular* spec * vec3(texture(material.specular, TextCoord))*intensity;
       vec3 result = ambient+ diffuse + specular;
        FragColor = vec4(result, 1.0);
     }else{
        FragColor = vec4(light.ambient*texture(material.diffuse, TextCoord).rgb, 1.0);
     }

}
