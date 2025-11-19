// Phong lighting with vertex color (no texture)
#version 400 core
out vec4 FragColor;

in vec3 normal;  
in vec3 fragPos;  
in vec3 vertColor;
in vec2 texCoord;

uniform vec3 u_camPos;

// light parameters
uniform vec3 u_light_ambient;
uniform vec3 u_light_position;
uniform vec3 u_light_diffuse;
uniform vec3 u_light_specular;

void main()
{
    // ambient
    vec3 ambient = u_light_ambient * vertColor;
  	
    // diffuse
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(u_light_position - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = u_light_diffuse * diff * vertColor;  
    
    // specular
    vec3 viewDir = normalize(u_camPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = u_light_specular * spec * vec3(0.3); // reduced specular intensity

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);    
}
