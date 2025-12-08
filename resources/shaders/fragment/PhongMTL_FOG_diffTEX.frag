#version 400 core
out vec4 FragColor;

in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;
in float fogDistance;

uniform vec3 u_camPos;

// Material Uniforms
uniform vec3 u_material_diffuse;   // Kd (Diffuse Color)
uniform vec3 u_material_ambient;  // Ka (Ambient Color)
uniform vec3 u_material_specular;  // Ks (Specular Color)
uniform float u_material_shininess; // Ns (Specular Exponent/Shininess)

// light parameters
uniform vec3 u_light_ambient;
uniform vec3 u_light_position;
uniform vec3 u_light_diffuse;
uniform vec3 u_light_specular;

// fog parameters
uniform vec3 u_fogColor;
uniform float u_fogStart;
uniform float u_fogEnd;

uniform sampler2D u_texture_diffuse;

void main()
{
    // ambient
    vec3 ambient = u_light_ambient * u_material_ambient * vec3(texture(u_texture_diffuse, texCoord)); 
    
    // diffuse
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(u_light_position - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    // Diffuse Light * Diffuse Factor * Material Diffuse Color (Kd)
    vec3 diffuse = u_light_diffuse * diff * u_material_diffuse * vec3(texture(u_texture_diffuse, texCoord));
    
    // specular
    vec3 viewDir = normalize(u_camPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    // Use Material Shininess (Ns)
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_material_shininess);
    // Specular Light * Specular Factor * Material Specular Color (Ks)
    vec3 specular = u_light_specular * spec * u_material_specular * vec3(texture(u_texture_diffuse, texCoord));

    // The final color is the sum of the components
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
    
    // Apply fog effect
    float fogFactor = clamp((fogDistance - u_fogStart) / (u_fogEnd - u_fogStart), 0.0, 1.0);
    FragColor.rgb = mix(FragColor.rgb, u_fogColor, fogFactor);
}