// Terrain Fragment Shader with multi-texture blending based on height
#version 400 core
out vec4 FragColor;

in vec3 normal;  
in vec3 fragPos;  
in vec2 texCoord;
in float height;
in float waterMask;
in float fogDistance;

uniform sampler2D u_texture0; // Low terrain (ground)
uniform sampler2D u_texture1; // Mid terrain (grass)
uniform sampler2D u_texture2; // High terrain (mountain/rock)
uniform sampler2D u_texture3; // Water (blue water)
uniform sampler2D u_texture4; // Water detail (white water)

uniform vec3 u_camPos;

// light parameters
uniform vec3 u_light_ambient;
uniform vec3 u_light_position;
uniform vec3 u_light_diffuse;
uniform vec3 u_light_specular;

// fog parameters
uniform float u_fogStart;      // Distance where fog starts to become noticeable
uniform float u_fogEnd;        // Distance where fog is fully opaque
uniform vec3 u_fogColor;       // Color of the fog (typically sky color)

void main()
{
    // Height-based texture blending
    vec4 groundColor = texture(u_texture0, texCoord);
    vec4 grassColor = texture(u_texture1, texCoord);
    vec4 mountainColor = texture(u_texture2, texCoord);
    
    // Blend textures - mountains should be mostly ground/stone
    vec4 terrainColor;
    
    if (height < 0.15) {
        // Very low terrain: ground
        terrainColor = groundColor;
    } else if (height < 0.20) {
        // Blend between ground and grass
        float blend = (height - 0.15) / 0.05;
        terrainColor = mix(groundColor, grassColor, blend);
    } else if (height < 0.40) {
        // Low-mid terrain: grass
        terrainColor = grassColor;
    } else if (height < 0.48) {
        // Transition back to ground as we go higher
        float blend = (height - 0.40) / 0.08;
        terrainColor = mix(grassColor, groundColor, blend);
    } else if (height < 0.70) {
        // Mountain slopes: ground/dirt texture
        terrainColor = groundColor;
    } else if (height < 0.75) {
        // Blend to stone at peaks
        float blend = (height - 0.70) / 0.05;
        terrainColor = mix(groundColor, mountainColor, blend);
    } else {
        // High peaks: stone texture
        terrainColor = mountainColor;
    }
    
    // Phong lighting for terrain
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(u_light_position - fragPos);
    vec3 viewDir = normalize(u_camPos - fragPos);
    
    // ambient
    vec3 ambient = u_light_ambient * vec3(terrainColor);
  	
    // diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = u_light_diffuse * diff * vec3(terrainColor);  
    
    // specular
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
    vec3 specular = u_light_specular * spec * 0.3; // Reduced specular for terrain

    vec3 terrainResult = ambient + diffuse + specular;
    
    // Water overlay with transparency
    if (waterMask > 0.5) {
        // Sample both water textures
        vec4 blueWater = texture(u_texture3, texCoord * 1.5);
        vec4 whiteWater = texture(u_texture4, texCoord * 2.5);
        
        // Mix blue and white water (mostly blue, some white for foam/detail)
        vec4 waterColor = mix(blueWater, whiteWater, 0.10);
        
        // Water has high specular (reflective surface)
        float waterSpec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
        vec3 waterSpecular = u_light_specular * waterSpec * 1.5;
        
        // Water lighting (slightly enhanced ambient for underwater glow)
        vec3 waterAmbient = u_light_ambient * vec3(waterColor) * 1.2;
        vec3 waterDiffuse = u_light_diffuse * diff * vec3(waterColor) * 0.7;
        vec3 waterResult = waterAmbient + waterDiffuse + waterSpecular;
        
        // Water transparency: 0.85 alpha (semi-transparent, more opaque)
        FragColor = vec4(waterResult, 0.85);
    } else {
        FragColor = vec4(terrainResult, 1.0);
    }
    
    // Apply cylindrical fog that extends from ground to sky
    // Calculate fog factor using smoothstep for smooth transition
    // The fog is based purely on horizontal distance, creating a cylinder effect
    float fogFactor = smoothstep(u_fogStart, u_fogEnd, fogDistance);
    
    // Add height-based fog density modifier to create visible fog wall
    // The higher you look, the more fog you see, creating a wall effect
    float heightAboveCamera = fragPos.y - u_camPos.y;
    float heightFactor = smoothstep(-50.0, 150.0, heightAboveCamera);
    fogFactor = max(fogFactor, heightFactor * fogFactor * 0.8);
    
    // Mix the final color with fog color based on distance
    vec3 finalColor = mix(FragColor.rgb, u_fogColor, fogFactor);
    FragColor = vec4(finalColor, FragColor.a);
}
