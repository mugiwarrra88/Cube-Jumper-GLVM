#version 410 core
out vec4 FragColor;

in VS_OUT {
	vec3 FragmentPosition;
	vec3 Normal;
	vec2 TextureCoords;
} fs_in;

struct Material {
    vec3      ambient;
	sampler2D diffuse;
	sampler2D specular;
    float     shininess;
}; 

uniform Material material;

void main()
{
	// Simple texture sampling without any lighting calculations
	vec4 textureColor = texture(material.diffuse, fs_in.TextureCoords);
	FragColor = textureColor;
}


