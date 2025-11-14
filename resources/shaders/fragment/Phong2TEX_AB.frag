//  
#version 400 core
out vec4 FragColor;

in vec3 normal;  
in vec3 fragPos;  
in vec2 texCoord;

uniform sampler2D u_texture1;
uniform sampler2D u_texture2;
uniform vec3 u_camPos;

// light parameters
uniform vec3 u_light_ambient;
uniform vec3 u_light_position;
uniform vec3 u_light_diffuse;
uniform vec3 u_light_specular;

void main()
{
    vec4 tex1 = texture(u_texture1, texCoord);
	vec4 tex2 = texture(u_texture2, texCoord);    	
	// Standard alpha blending
	vec4 combinedTex = tex1 * (1.0 - tex2.a) + tex2 * tex2.a;
    
    // ambient
    vec3 ambient = u_light_ambient * vec3(combinedTex);
  	
    // diffuse
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(u_light_position - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = u_light_diffuse * diff * vec3(combinedTex);  
    
    // specular
    vec3 viewDir = normalize(u_camPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = u_light_specular * spec * vec3(combinedTex);  

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);    
}