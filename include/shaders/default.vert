#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aPitchYaw;
layout (location = 2) in vec3 aOffset;

out vec3 FragPos;
out vec3 ourNormal; 
out vec2 TexCoord; 
//out float HeightLevel;
//out float SinkLevel;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 playerPos;
uniform int lodScale;

uniform float startDistMultiplier;
uniform float endDistAdd;
uniform float baseSinkStrength; 
uniform float fadePow;


void main()
{
	float pitch = aPitchYaw.x * 2.0 * 3.14159265 - 3.14159265; 
	float yaw   = aPitchYaw.y * 2.0 * 3.14159265 - 3.14159265;  

	vec3 normal;
    normal.x = cos(pitch) * cos(yaw);
    normal.y = sin(pitch);
    normal.z = cos(pitch) * sin(yaw);
    normal   = normalize(normal);

    vec3 worldPos = aOffset + (aPos * lodScale);

    //HeightLevel = worldPos.y;

    if (lodScale > 1.0) {
        vec2 origin = playerPos.xz;
        vec2 blockPos = worldPos.xz;
        float dist = length(blockPos - origin);

        // Adjust this falloff distance to match your chunk radius
        float startDist = startDistMultiplier * lodScale; 
        float endDist   = startDist + endDistAdd * lodScale;

        // Smooth falloff factor (1 near boundary, 0 when far)
        float t = clamp((dist - startDist) / max(1.0, endDist - startDist), 0.0, 1.0);
        float fade = pow(1.0 - smoothstep(0.0, 1.0, t), fadePow);

        // sinkStrength scales more naturally (linear-ish, not exponential)
        
        float sinkStrength = baseSinkStrength * log2(lodScale + 1.0);

        worldPos.y -= fade * sinkStrength;
    }

        
	gl_Position = projection * view * model * vec4(worldPos, 1.0);
    ourNormal = mat3(transpose(inverse(model))) * normal;
    FragPos = vec3(model * vec4(aPos, 1.0)); 

	float epsilon = 0.01;

    if (abs(normal.x) > 1.0 - epsilon) {
        TexCoord = aPos.yz + 0.5;

    } else if (abs(normal.y) > 1.0 - epsilon) {
        TexCoord = aPos.xz + 0.5;

    } else {
        TexCoord = aPos.xy + 0.5;
    }

	//TexCoord = aTexCoord;
}