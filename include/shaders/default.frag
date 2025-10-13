#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 ourNormal; 
in vec2 TexCoord;
in float vFade;
//in float HeightLevel;
//in float SinkLevel;

uniform sampler2D ourTexture;
uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;


void main()
{

    //if (lodScale > 1) {
    //    if ((SinkLevel + 25) < HeightLevel) {
            //discard;
    //    }
   // }

   //Ambient
	float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;

	// Diffuse
    vec3 norm = normalize(ourNormal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    //specular
    float specularStrength = 0.9;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  

    // Combine lighting
    vec3 resultLight = (ambient + diffuse + specular);

	// Apply lighting to texture
    vec4 texColor = texture(ourTexture, TexCoord);
    FragColor = vec4(resultLight, 1.0) * texColor;
}