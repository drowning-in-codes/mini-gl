#version 400 core
in vec3 Normal;
in vec3 FragPos;
in vec2 TextCoord;
out vec4 FragColor;
struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
struct PointLight{
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
struct SpotLight{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
struct Material{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};
vec3 CalcDirLight(DirLight light,vec3 normal,vec3 viewDir);
vec3 CalcPointLight(PointLight light,vec3 normal,vec3 fragPos,vec3 viewDir);
vec3 CalcSpotLight(SpotLight light,vec3 normal,vec3 fragPos,vec3 viewDir);

uniform vec3 viewPos;

#define NR_POINT_LIGHTS 4
// 定向光
uniform DirLight dirLight;
// 点光源
uniform PointLight pointLights[NR_POINT_LIGHTS];
// 聚光
uniform SpotLight spotLight;
uniform Material material;

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = CalcDirLight(dirLight,norm,viewDir);
    for(int i =0; i<NR_POINT_LIGHTS; i++){
        result += CalcPointLight(pointLights[i],norm,FragPos,viewDir);
    }
    result += CalcSpotLight(spotLight,norm,FragPos,viewDir);
    FragColor = vec4(result,1.0);
    // FragColor = vec4(vec3(gl_FragCoord.z),1.0);
}

vec3 CalcDirLight(DirLight light,vec3 normal,vec3 viewDir){
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(-light.direction);
    // 环境光
    vec3 ambient = texture(material.diffuse, TextCoord).rgb * light.ambient;
    // 漫反射
    float diff = max(dot(norm,lightDir),0.0);
    vec3 diffuse = texture(material.diffuse, TextCoord).rgb * light.diffuse * diff;
    // 镜面反射
    vec3 reflectDir = reflect(-lightDir,norm);
    float spec = pow(max(dot(reflectDir,viewDir),0.0),material.shininess);
    vec3 specular = texture(material.specular, TextCoord).rgb * light.specular * spec;
    //叠加
    vec3 result = ambient + diffuse + specular;
    return result;
}


vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TextCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TextCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TextCoord));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light,vec3 normal,vec3 fragPos,vec3 viewDir){
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(light.position - fragPos);
    float theta = dot(-lightDir,normalize(light.direction));
    //环境光
    vec3 ambient = texture(material.diffuse, TextCoord).rgb * light.ambient;
    // 漫反射光
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon,.0,1.0);
    float diff = max(dot(norm,lightDir),0.0);
    vec3 diffuse = diff*texture(material.diffuse, TextCoord).rgb * light.diffuse*intensity;
    //镜面反射
    vec3 reflectDir = reflect(-lightDir,norm);
    float spec = pow(max(dot(reflectDir,viewDir),.0),material.shininess);
    vec3 specular = spec*texture(material.specular, TextCoord).rgb * light.specular*intensity;
    return ambient + diffuse + specular;
}