// Terrain Fragment Shader with multi-texture blending based on height
#version 400 core
out vec4 FragColor;

in vec3 normal;  
in vec3 fragPos;  
in vec2 texCoord;
in float height;

uniform sampler2D u_texture0; // Low terrain (ground)
uniform sampler2D u_texture1; // Mid terrain (grass)
uniform sampler2D u_texture2; // High terrain (mountain/rock)

uniform vec3 u_camPos;

// light parameters
uniform vec3 u_light_ambient;
uniform vec3 u_light_position;
uniform vec3 u_light_diffuse;
uniform vec3 u_light_specular;

void main()
{
    // Height-based texture blending
    vec4 groundColor = texture(u_texture0, texCoord);
    vec4 grassColor = texture(u_texture1, texCoord);
    vec4 mountainColor = texture(u_texture2, texCoord);
    
    // Blend textures - mountains should be mostly ground/stone
    vec4 finalColor;
    
    if (height < 0.15) {
        // Very low terrain: ground
        finalColor = groundColor;
    } else if (height < 0.20) {
        // Blend between ground and grass
        float blend = (height - 0.15) / 0.05;
        finalColor = mix(groundColor, grassColor, blend);
    } else if (height < 0.40) {
        // Low-mid terrain: grass
        finalColor = grassColor;
    } else if (height < 0.48) {
        // Transition back to ground as we go higher
        float blend = (height - 0.40) / 0.08;
        finalColor = mix(grassColor, groundColor, blend);
    } else if (height < 0.70) {
        // Mountain slopes: ground/dirt texture
        finalColor = groundColor;
    } else if (height < 0.75) {
        // Blend to stone at peaks
        float blend = (height - 0.70) / 0.05;
        finalColor = mix(groundColor, mountainColor, blend);
    } else {
        // High peaks: stone texture
        finalColor = mountainColor;
    }
    
    // Phong lighting
    // ambient
    vec3 ambient = u_light_ambient * vec3(finalColor);
  	
    // diffuse
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(u_light_position - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = u_light_diffuse * diff * vec3(finalColor);  
    
    // specular
    vec3 viewDir = normalize(u_camPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
    vec3 specular = u_light_specular * spec * 0.3; // Reduced specular for terrain

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);    
}
