#version 400 core
out vec4 FragColor;
// in vec2 TexCoord;
// uniform sampler2D texture1;
// uniform sampler2D texture2;
in vec3 Normal;
in vec3 FragPos;
// uniform vec3 lightColor;
// uniform vec3 objectColor;
uniform vec3 viewPos;
struct Material {
//     vec3 ambient;
//     vec3 diffuse;
    sampler2D specular;
    sampler2D diffuse;
    float shininess;
//     sampler2D emissive;
};
struct Light {
     vec3 position;
     // vec3 LightDir; // 定向光/平行光
     vec3 ambient;
     vec3 diffuse;
     vec3 specular;

    float K_c;
     float K_l;
     float K_q;
};
// struct SpotLight {
//      vec3 position;
//      vec3 LightDir;
//      float cutOff;

// };
in vec2 TextCoord;
in vec2 emissive_coord;
uniform Light light;
uniform Material material;
void main() {
     // FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
     // FragColor = vec4(lightColor * objectColor,1.0);
     // 强度
     float d = length(FragPos- light.position);

     float F_att = 1.0/(light.K_c +light.K_l  * d + light.K_q * d * d);
     // 环境光照
     // vec3 ambient = light.ambient * material.ambient;
     vec3 ambient = light.ambient * texture(material.diffuse, TextCoord).rgb;
     // 漫反射光照
     // vec3 norm = normalize(Normal);
     // vec3 lightDir = normalize(light.position - FragPos);
     // float diff = max(dot(norm, lightDir), 0.0);
     // vec3 diffuse = diff* light.diffuse * material.diffuse;
     // 漫反射贴图
     vec3 norm = normalize(Normal);
    
     vec3 lightDir = normalize(light.position - FragPos);
     // vec3 lightDir = normalize(-light.LightDir);
     float diff = max(dot(norm, lightDir), 0.0);
     vec3 diffuse = diff * light.diffuse * vec3(texture(material.diffuse, TextCoord));

     // 镜面反射
     vec3 reflectDir = reflect(-lightDir,Normal);
     vec3 viewDir = normalize(viewPos - FragPos);
     float spec = pow(max(dot(reflectDir, viewDir),0.0), material.shininess);
    
     // vec3 specular = light.specular * spec * material.specular;
     vec3 specular = light.specular * spec * vec3(texture(material.specular, TextCoord));
     vec3 result =  ambient + F_att* diffuse+F_att*specular ;
     FragColor = vec4(result, 1.0);
}
