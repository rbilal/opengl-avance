#version 330
out vec3 fColor;

uniform vec3 uDirectionalLightDir;
uniform vec3 uDirectionalLightIntensity;

uniform vec3 uPointLightPosition;
uniform vec3 uPointLightIntensity;

uniform sampler2D uGPosition;
uniform sampler2D uGNormal;
uniform sampler2D uGAmbient;
uniform sampler2D uGDiffuse;
uniform sampler2D uGlossyShininess;

uniform mat4 uDirLightViewProjMatrix;
uniform sampler2D uDirLightShadowMap;
uniform float uDirLightShadowMapBias;


void main()
{
    vec3 ka = vec3(texelFetch(uGAmbient, ivec2(gl_FragCoord.xy), 0));

    vec3 kd = vec3(texelFetch(uGDiffuse, ivec2(gl_FragCoord.xy), 0));

    vec4 ksShininess = vec4(texelFetch(uGlossyShininess, ivec2(gl_FragCoord.xy), 0));

    vec3 ks = ksShininess.rgb;

    float shininess = ksShininess.a;

    vec3 normal = vec3(texelFetch(uGNormal, ivec2(gl_FragCoord.xy), 0));

    vec3 position = vec3(texelFetch(uGPosition, ivec2(gl_FragCoord.xy), 0)); // Correspond a vViewSpacePosition dans le forward renderer

    vec3 eyeDir = normalize(-position);

    float distToPointLight = length(uPointLightPosition - position);
    vec3 dirToPointLight = (uPointLightPosition - position) / distToPointLight;
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


    vec4 positionInDirLightScreen = uDirLightViewProjMatrix * vec4(position, 1);
    vec3 positionInDirLightNDC = vec3(positionInDirLightScreen / positionInDirLightScreen.w) * 0.5 + 0.5;
    float depthBlockerInDirSpace = texture(uDirLightShadowMap, positionInDirLightNDC.xy).r;
    float dirLightVisibility = positionInDirLightNDC.z < depthBlockerInDirSpace + uDirLightShadowMapBias ? 1.0 : 0.0;

    uDirectionalLightIntensity *= dirLightVisibility;

    fColor = ka;
    fColor += kd * (uDirectionalLightIntensity * max(0.f, dot(normal, uDirectionalLightDir)) + pointLightIncidentLight * max(0., dot(normal, dirToPointLight)));
    fColor += ks * (uDirectionalLightIntensity * dothDirLight + pointLightIncidentLight * dothPointLight);


}