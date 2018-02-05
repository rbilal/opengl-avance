#version 330
in vec3 vViewSpacePosition; // Position du sommet dans l'espace view
in vec3 vViewSpaceNormal; // Normale du sommet transformé dans l'espace view
in vec2 vTexCoords; // Coordonnées de texture du sommet

uniform vec3 uDirectionalLightDir;
uniform vec3 uDirectionalLightIntensity;

uniform vec3 uPointLightPosition;
uniform vec3 uPointLightIntensity;

uniform vec3 uKd;
uniform vec3 uKa;
uniform vec3 uKs;

uniform sampler2D uKaSampler;
uniform sampler2D uKsSampler;
uniform sampler2D uKdSampler;

out vec3 fFragColor;

void main() {
	
	vec3 coeff = texture(uKaSampler, vTexCoords).xyz;

	float distToPointLight = length(uPointLightPosition - vViewSpacePosition);
	vec3 dirToPointLight = (uPointLightPosition - vViewSpacePosition) / distToPointLight;
	fFragColor = uKd * coeff * (uDirectionalLightIntensity * max(0.0, dot(vViewSpaceNormal, uDirectionalLightDir)) + uPointLightIntensity * max(0.0, dot(vViewSpaceNormal, dirToPointLight)) / distToPointLight * distToPointLight);
	//fFragColor = normalize(vViewSpaceNormal);
	fFragColor = coeff;
};
