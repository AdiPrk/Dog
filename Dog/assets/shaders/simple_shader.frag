#version 450

#extension GL_EXT_nonuniform_qualifier : require

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragTexCoord;

layout (location = 0) out vec4 outColor;

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

layout(set = 0, binding = 1) uniform sampler2D uTextures[];  // Texture sampler

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
  int textureIndex;
} push;

void main() {
   // Ambient light
  vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
  vec3 specularLight = vec3(0.0);
  vec3 surfaceNormal = normalize(fragNormalWorld);

  vec3 cameraPosWorld = ubo.invView[3].xyz;
  vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

  for (int i = 0; i < ubo.numLights; i++) {
    PointLight light = ubo.pointLights[i];
    vec3 directionToLight = light.position.xyz - fragPosWorld;
    float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared
    directionToLight = normalize(directionToLight);

    // Diffuse lighting
    float cosAngIncidence = max(dot(surfaceNormal, directionToLight), 0);
    vec3 intensity = light.color.xyz * light.color.w * attenuation;
    diffuseLight += intensity * cosAngIncidence;

    // Specular lighting
    vec3 halfAngle = normalize(directionToLight + viewDirection);
    float blinnTerm = max(dot(surfaceNormal, halfAngle), 0.0);
    blinnTerm = pow(blinnTerm, 64.0); // Adjust specular sharpness as needed
    specularLight += intensity * blinnTerm;
  }

  // Fetch texture color
  vec4 texColor = vec4(1.0);
  if (push.textureIndex == 999) {
    discard;
  }
  if (push.textureIndex != 999) {
    texColor = texture(uTextures[push.textureIndex], fragTexCoord);
  }

  // Combine texture color with diffuse lighting (multiplicative)
  vec3 lightingContribution = texColor.xyz * diffuseLight;

  // Add specular contribution separately
  vec3 finalColor = lightingContribution + specularLight;

  // Output final color
  //outColor = vec4(finalColor, 1.f);
  outColor = vec4(texColor.rgba);
}
