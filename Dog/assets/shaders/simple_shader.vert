#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 texCoord;
layout(location = 4) in ivec4 boneIds;
layout(location = 5) in vec4 weights;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;
layout(location = 3) out vec2 fragTexCoord;

struct PointLight {
  vec4 position; // ignore w
  vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  mat4 invView;
  vec4 ambientLightColor; // w is intensity
  PointLight pointLights[10];
  int numLights;
} ubo;

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

layout(set = 0, binding = 2) uniform BonesUbo {
    mat4 finalBonesMatrices[MAX_BONES];
} bones;

void main() {
    vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(boneIds[i] == -1) 
            continue;
        if(boneIds[i] >=MAX_BONES) 
        {
            totalPosition = vec4(position,1.0f);
            break;
        }
        vec4 localPosition = bones.finalBonesMatrices[boneIds[i]] * vec4(position,1.0f);
        totalPosition += localPosition * weights[i];
        vec3 localNormal = mat3(bones.finalBonesMatrices[boneIds[i]]) * normal;
   }

    if (totalPosition == vec4(0.0)) {
		totalPosition = vec4(position, 1.0);
	}

    // Transform the vertex by the model matrix and the projection/view matrices
    vec4 worldPosition = push.modelMatrix * totalPosition;
    gl_Position = ubo.projection * ubo.view * worldPosition;

    // Pass through the other varying data (colors, normals, texture coordinates)
    fragColor = color;
    fragPosWorld = worldPosition.xyz;
    fragNormalWorld = normalize((push.normalMatrix * vec4(normal, 0.0)).xyz);
    fragTexCoord = texCoord;
}
