#version 400 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D texture1;
void main(){
    // 反相
    // vec3 col = 1 - texture(texture1, TexCoord).rgb;
    // 灰度
    FragColor = texture(texture1, TexCoord);
//     float average = (.2126*FragColor.r + .7152* FragColor.g + .0722*FragColor.b) / 3.0;
//     FragColor = vec4(average,average,average, 1.0); 


// const float offset = 1.0 / 300.0;
//     vec2[9] offsets = vec2[9](
//         vec2(-offset,offset),
//         vec2(.0f,offset),
//         vec2(offset,offset),
//         vec2(-offset,.0f),
//         vec2(.0f,.0f),
//         vec2(offset,.0f),
//         vec2(-offset,-offset),
//         vec2(.0f,-offset),
//         vec2(offset,-offset)
//     );
//     vec3[9] sampleTex;
//     for(int i=0;i<9;i++){
//         sampleTex[i] = vec3(texture(texture1, TexCoord.st + offsets[i]));
//     }
//     // float[9] kernel = float[9](
//     //     -1,-1,-1,
//     //     -1,9,-1,
//     //     -1,-1,-1
//     // );
//     vec3 col = vec3(0.0);
//     // for(int i =0;i<9;i++){
//     //     col += sampleTex[i] * kernel[i];
//     // }
//     //核效果
    
//     // 模糊
//     // float[9] blur_kernel = float[9](
//     //     1.0/16,2.0/16,1.0/16,
//     //     2.0/16,4.0/16,2.0/16,
//     //     1.0/16,2.0/16,1.0/16
//     // );
//     // for(int i = 0;i<9;i++){
//     //     col += sampleTex[i] * blur_kernel[i];}

//     //边缘检测
//     float[9] detection_kernel = float[9](
//         -1,-1,-1,
//         -1,8,-1,
//         -1,-1,-1
//     );
//     for(int i=0;i<9;i++){
//         col += sampleTex[i] * detection_kernel[i];
//     }
//     FragColor = vec4(col, 1.0);

}