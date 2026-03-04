#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aPitchYaw;
layout (location = 2) in vec3 aLocalPos;
//layout (location = 2) in vec3 aOffset;

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
uniform vec3 chunkWorldPos;

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

    vec3 position = vec3(0.0);

    position.x = floor(gl_VertexID / 12.0);
    position.z = mod(gl_VertexID, 12.0);
    position.y = aPos.y;

    vec3 aPosTemp = aPos;

    //if (gl_VertexID == 2) {
    //    aPosTemp.y += 10;
    //}

    vec3 chunkOffset = chunkWorldPos * 16.0;

    vec3 worldPos = chunkOffset + aLocalPos + (aPosTemp * lodScale);

    //vec3 tempCombinedPos = aPos + aLocalPos + chunkOffset;

	gl_Position = projection * view * model * vec4(worldPos, 1.0);
    ourNormal = mat3(transpose(inverse(model))) * normal;
    FragPos = vec3(model * vec4(worldPos, 1.0));

	float epsilon = 0.01;

    if (abs(normal.x) > 1.0 - epsilon) {
        TexCoord = worldPos.yz + 0.5;

    } else if (abs(normal.y) > 1.0 - epsilon) {
        TexCoord = worldPos.xz + 0.5;

    } else {
        TexCoord = worldPos.xy + 0.5;
    }

	//TexCoord = aTexCoord;
}