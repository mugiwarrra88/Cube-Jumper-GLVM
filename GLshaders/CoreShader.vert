#version 410 core
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textureCoordinates;
layout (location = 3) in vec4 jointIndices;
layout (location = 4) in vec4 weights;

out VS_OUT {
	vec3 fragmentPosition;
	vec3 normal;
	vec2 textureCoords;
	vec4 fragmentPositionDirectionalLightSpace[2];
	vec4 fragmentPositionSpotLightSpace[2];
} vs_out;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform bool reverseNormals;
uniform mat4 jointMatrices[18];

void main()
{
	mat4 skinMatrix;
	if (int(jointIndices.x) != -1) {
		skinMatrix =
			weights.x * jointMatrices[int(jointIndices.x)] +
			weights.y * jointMatrices[int(jointIndices.y)] +
			weights.z * jointMatrices[int(jointIndices.z)] +
			weights.w * jointMatrices[int(jointIndices.w)];
	} else {
		skinMatrix = mat4(
			1.0, 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0,
			0.0, 0.0, 0.0, 1.0
			);
	}

	vec4 worldPosition = modelMatrix * skinMatrix * vec4(vertexPosition, 1.0);

	vs_out.fragmentPosition = worldPosition.xyz;
	if(reverseNormals)
        vs_out.normal = transpose(inverse(mat3(modelMatrix * skinMatrix))) * (-1.0 * normal);
    else
        vs_out.normal = transpose(inverse(mat3(modelMatrix * skinMatrix))) * normal;
	vs_out.textureCoords = textureCoordinates;
	
	// Set dummy values for shadow matrices to maintain compatibility
	vs_out.fragmentPositionDirectionalLightSpace[0] = vec4(0.0);
	vs_out.fragmentPositionDirectionalLightSpace[1] = vec4(0.0);
	vs_out.fragmentPositionSpotLightSpace[0] = vec4(0.0);
	vs_out.fragmentPositionSpotLightSpace[1] = vec4(0.0);

	gl_Position = projectionMatrix * viewMatrix * worldPosition;
}
