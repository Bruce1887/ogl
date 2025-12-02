//  
#version 400 core
out vec4 FragColor;

in vec3 normal;  
in vec3 fragPos;  

uniform vec3 u_color;
uniform vec3 u_camPos;


// light parameters
uniform vec3 u_light_ambient;
uniform vec3 u_light_position;
uniform vec3 u_light_diffuse;
uniform vec3 u_light_specular;

void main()
{
    // ambient
    vec3 ambient = u_light_ambient * u_color;
    // diffuse
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(u_light_position - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = u_light_diffuse * diff * u_color;      
    // specular
    vec3 viewDir = normalize(u_camPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = u_light_specular * spec * u_color;  
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);    
}