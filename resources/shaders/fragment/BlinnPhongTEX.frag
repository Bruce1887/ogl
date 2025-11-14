//  
#version 400 core
out vec4 FragColor;

in vec3 normal;  
in vec3 fragPos;  
in vec2 texCoord;

uniform sampler2D u_texture;
uniform vec3 u_camPos; // we get this from in variable

// light parameters
uniform vec3 u_light_ambient;
uniform vec3 u_light_position;
uniform vec3 u_light_diffuse;
uniform vec3 u_light_specular;

void main()
{
    // ambient
    vec3 ambient = u_light_ambient * vec3(texture(u_texture, texCoord));
  	
    // diffuse
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(u_light_position - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = u_light_diffuse * diff * vec3(texture(u_texture, texCoord));  
    
    // specular
    vec3 viewDir = normalize(u_camPos - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);  // Blinn-Phong approach
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32);
    
    // Don't multiply specular by texture - use light color only
    vec3 specular = u_light_specular * spec;  

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);    
}