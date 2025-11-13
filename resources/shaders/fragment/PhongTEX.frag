//  
#version 400 core
out vec4 FragColor;

in vec3 normal;  
in vec3 fragPos;  
in vec2 texCoord;

// material parameters
uniform sampler2D u_texture;
uniform vec3 u_camPos;

// light parameters
uniform vec3 light_ambient;
uniform vec3 light_position;
uniform vec3 light_diffuse;
uniform vec3 light_specular;

void main()
{
    // ambient
    vec3 ambient = light_ambient * vec3(texture(u_texture, texCoord));
  	
    // diffuse
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(light_position - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light_diffuse * diff * vec3(texture(u_texture, texCoord));  
    
    // specular
    vec3 viewDir = normalize(u_camPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = light_specular * spec * vec3(texture(u_texture, texCoord));  
        
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);    
}