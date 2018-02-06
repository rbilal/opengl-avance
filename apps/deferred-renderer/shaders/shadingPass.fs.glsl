#version 330

in vec3 vViewSpacePosition;
in vec3 vViewSpaceNormal;
in vec2 vTexCoords;

out vec3 fColor;

uniform vec3 uDirectionalLightDir;
uniform vec3 uDirectionalLightIntensity;

uniform vec3 uPointLightPosition;
uniform vec3 uPointLightIntensity;

uniform vec3 uKa;
uniform vec3 uKd;
uniform vec3 uKs;
uniform float uShininess;

uniform sampler2D uKaSampler;
uniform sampler2D uKdSampler;
uniform sampler2D uKsSampler;
uniform sampler2D uShininessSampler;

uniform sampler2D uGPosition;
uniform sampler2D uGNormal;
uniform sampler2D uGAmbient;
uniform sampler2D uGDiffuse;
uniform sampler2D uGlossyShininess;

void main()
{
    vec3 ka = vec3(texelFetch(uGAmbient, ivec2(gl_FragCoord.xy), 0));


    vec3 kd = vec3(texelFetch(uGDiffuse, ivec2(gl_FragCoord.xy), 0));

    vec3 ks = vec3(texelFetch(uGlossyShininess, ivec2(gl_FragCoord.xy), 0));;

    float shininess = uShininess * vec3(texture(uShininessSampler, vTexCoords)).x;

    vec3 texnormal = vec3(texelFetch(uGNormal, ivec2(gl_FragCoord.xy), 0));

    vec3 normal = normalize(texnormal);

    vec3 position = vec3(texelFetch(uGPosition, ivec2(gl_FragCoord.xy), 0)); // Correspond a vViewSpacePosition dans le forward renderer

    vec3 eyeDir = normalize(-position);

    float distToPointLight = length(uPointLightPosition - vViewSpacePosition);
    vec3 dirToPointLight = (uPointLightPosition - vViewSpacePosition) / distToPointLight;
    vec3 pointLightIncidentLight = uPointLightIntensity / (distToPointLight * distToPointLight);

    // half vectors, for blinn-phong shading
    vec3 hPointLight = normalize(eyeDir + dirToPointLight);
    vec3 hDirLight = normalize(eyeDir + uDirectionalLightDir);

    float dothPointLight = shininess == 0 ? 1.f : max(0.f, dot(normal, hPointLight));
    float dothDirLight = shininess == 0 ? 1.f :max(0.f, dot(normal, hDirLight));

    if (shininess != 1.f && shininess != 0.f)
    {
        dothPointLight = pow(dothPointLight, shininess);
        dothDirLight = pow(dothDirLight, shininess);
    }

    fColor = ka;
    fColor += kd * (uDirectionalLightIntensity * max(0.f, dot(normal, uDirectionalLightDir)) + pointLightIncidentLight * max(0., dot(normal, dirToPointLight)));
    fColor += ks * (uDirectionalLightIntensity * dothDirLight + pointLightIncidentLight * dothPointLight);
}