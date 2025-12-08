#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D blockTexture;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform bool useTexture;  // 텍스처 사용 여부
uniform vec3 solidColor;   // 단색

void main()
{
    vec3 baseColor;
    
    if (useTexture) {
        // 텍스처 사용
        baseColor = texture(blockTexture, TexCoord).rgb;
    } else {
        // 단색 사용
        baseColor = solidColor;
    }
    
    // Ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Result
    vec3 result = (ambient + diffuse) * baseColor;
    FragColor = vec4(result, 1.0);
}