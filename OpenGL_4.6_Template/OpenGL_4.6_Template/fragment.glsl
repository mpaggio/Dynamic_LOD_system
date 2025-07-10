#version 460 core

in flat int gs_isGrass;
in flat int gs_isKelp;

in vec4 gs_worldPos; //Input proveniente dal Geometry Shader
out vec4 FragColor; //Output del Fragment Shader, ovvero il colore finale del pixel (variabile mandata direttamente al frame buffer)

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



void main() {
    float h = gs_worldPos.y;
    vec2 uv = gs_worldPos.xz;

    if (gs_isGrass == 1) {
        FragColor = vec4(0.2, 0.8, 0.1, 1.0); // Verde foglia
    }
    else if (gs_isKelp == 1) {
        FragColor = vec4(0.2, 0.25, 0.1, 1.0); // Verde alga
    }
    else {
        vec4 color = blendTextures(h, uv);
        FragColor = color;
        //vec3 color = interpolateColor(h);
        //FragColor = vec4(color, 0.0f);
    }
}