#version 460 core

in flat int gs_isGrass;
in flat int gs_isKelp;

in vec4 gs_worldPos;
in vec3 gs_normal;

out vec4 FragColor; //Output del Fragment Shader, ovvero il colore finale del pixel (variabile mandata direttamente al frame buffer)

struct PointLight {
    vec3 position;
    vec3 color;
    float power;
};

uniform vec3 ViewPos;
uniform PointLight light;

uniform sampler2D texture0; // snowColor
uniform sampler2D texture1; // snowNormal
uniform sampler2D texture2; // rockColor
uniform sampler2D texture3; // rockNormal
uniform sampler2D texture4; // grassColor
uniform sampler2D texture5; // grassNormal
uniform sampler2D texture6; // sandColor
uniform sampler2D texture7; // sandNormal

vec3 interpolateColor(float h) {
    vec3 col1, col2;
    float t;

    if (h < -1.0) {
        return vec3(0.0, 0.2, 0.8); // Mare molto profondo
    }
    else if (h < -0.5) {
        col1 = vec3(0.0, 0.2, 0.8); // Mare molto profondo
        col2 = vec3(0.0, 0.4, 0.7); // Mare poco profondo
        t = (h + 1.0) / 0.5;
    }
    else if (h < 0.0) {
        col1 = vec3(0.0, 0.4, 0.7); // Mare poco profondo
        col2 = vec3(0.2, 0.6, 0.2); // Prato
        t = (h + 0.5) / 1.0;
    }
    else if (h < 0.5) {
        col1 = vec3(0.2, 0.6, 0.2); // Prato
        col2 = vec3(0.4, 0.3, 0.1); // Montagna
        t = (h - 0.5) / 0.7;
    }
    else if (h < 1.5) {
        col1 = vec3(0.4, 0.3, 0.1); // Montagna
        col2 = vec3(1.0);           // Neve
        t = (h - 1.2) / 0.8;
    }
    else {
        return vec3(1.0); // Neve pura
    }

    return mix(col1, col2, clamp(t, 0.0, 1.0));
}

// Calcolo UV: puoi usare direttamente worldPos.xz come UV
vec4 blendTextures(float h, vec2 uv) {
    vec4 col1, col2;
    float t;

    if (h < -0.6) {
        return vec4(0.0, 0.1, 0.4, 1.0); // mare profondo scuro
    }
    else if (h < -0.2) {
        return vec4(0.0, 0.2, 0.8, 1.0); // mare profondo chiaro
    }
    else if (h < 0.0) {
        return vec4(0.0, 0.4, 0.7, 1.0); // mare poco profondo
    }
    else if (h < 0.1) {
        col1 = texture(texture6, uv * 8.0); // sabbia
        col2 = texture(texture4, uv * 8.0); // erba
        t = (h + 0.2) / 0.3;
    }
    else if (h < 0.6) {
        col1 = texture(texture4, uv * 8.0); // erba
        col2 = texture(texture2, uv * 6.0); // roccia
        t = (h - 0.1) / 0.5;
    }
    else if (h < 1.3) {
        col1 = texture(texture2, uv * 6.0); // roccia
        col2 = texture(texture0, uv * 5.0); // neve
        t = (h - 0.6) / 0.7;
    }
    else {
        return texture(texture0, uv * 5.0); // neve pura
    }

    return mix(col1, col2, clamp(t, 0.0, 1.0));
}

vec3 computeLighting(vec3 normal, vec3 fragPos, vec3 viewPos, vec3 baseColor) {
    // Normale normalizzata
    vec3 norm = normalize(normal);

    // Direzione della luce
    vec3 lightDir = normalize(light.position - fragPos);

    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * light.color;

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.color;

    // Specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0); // shininess fisso
    float specularStrength = 0.5;
    vec3 specular = specularStrength * spec * light.color;

    // Calcolo illuminazione finale moltiplicata per colore base e potenza luce
    vec3 lighting = (ambient + diffuse + specular) * baseColor * light.power;

    return lighting;
}



void main() {
    float h = gs_worldPos.y;
    vec2 uv = gs_worldPos.xz;
    vec3 baseColor;

    if (gs_isGrass == 1) {
        baseColor = vec3(0.2, 0.8, 0.1);
    }
    else if (gs_isKelp == 1) {
        baseColor = vec3(0.2, 0.25, 0.1);
    }
    else {
        baseColor = blendTextures(h, uv).rgb;
    }

    vec3 lighting = computeLighting(normalize(gs_normal), gs_worldPos.xyz, ViewPos, baseColor);
    FragColor = vec4(lighting, 1.0);
}