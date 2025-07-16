#version 460 core

//UNA ESECUZIONE PER OGNI PATCH

layout(vertices = 4) out; //Specifica che la patch output avrà 4 vertici, quindi la tessellazione sarà fatta su patch quadrate

uniform float terrainSize_tcs;
uniform vec3 cameraPosition;
uniform sampler2D u_fbmTexture;

const int MIN_TES = 6;
const int MAX_TES = 30;
const float MIN_DIST = 1.5;
const float MAX_DIST = 2.5;
const float MAX_HEIGHT_DIFF = 0.04;
const float HEIGHT_SCALE = 1.5;
const float UV_SCALE = 1.0f / terrainSize_tcs;

void main() {
    //Copia la posizione di ciascun vertice di input come output, senza modifiche (gl_InvocationID è l'indice dell'istanza corrente del vertex)
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position; 

    //Solo per il vertice con indice 0 si impostano i livelli di tessellazione
    if (gl_InvocationID == 0) {

        //Prendo posizione e calcolo altezza da texture FBM per ogni vertice originale
        vec3 p[4];
        float h[4];
        for (int i = 0; i < 4; ++i) {
            p[i] = gl_in[i].gl_Position.xyz;
            vec2 uv = p[i].xz * UV_SCALE;
            h[i] = texture(u_fbmTexture, uv).r * HEIGHT_SCALE;
        }

        //Dislivello massimo nella patch
        float minH = min(min(h[0], h[1]), min(h[2], h[3]));
        float maxH = max(max(h[0], h[1]), max(h[2], h[3]));
        float deltaH = maxH - minH; //dislivello massimo

        // Calcola i centri dei lati originali
        vec3 center[4];
        center[0] = (p[0] + p[3]) * 0.5; // sinistra
        center[1] = (p[0] + p[1]) * 0.5; // basso
        center[2] = (p[1] + p[2]) * 0.5; // destra
        center[3] = (p[2] + p[3]) * 0.5; // alto

        // Calcola LOD (livello tessellazione) basato su distanza e dislivello
        for (int i = 0; i < 4; ++i) {
            float dist = length(cameraPosition - center[i]); //distanza fra centro del lato e telecamera
            float tess;
            if (dist > MAX_DIST) { //se la distanza supera la distanza massima allora non faccio tessellazione
                tess = MIN_TES;
            }
            else {
                float distFactor = dist / MAX_DIST; //rapporto fra distanza effettiva e massima distanza
                float heightFactor = clamp(deltaH / MAX_HEIGHT_DIFF, 0.0, 1.0); //il rapporto fra dislivello effettivo e massimo viene limitato a [0,1]
                float lodFactor = 0.6 * distFactor + 0.4 * (1.0 - heightFactor); //più è distante più il lodfactor aumenta, più è piatta la patch meno è dettagliata

                //interpolazione lineare tra la tessellazione massima e minima
                tess = mix(MAX_TES, MIN_TES, lodFactor); //più il lodfactor è alto più il dettaglio sarà scarso
            }
            gl_TessLevelOuter[i] = tess;
        }

        // Tessellazione interna
        gl_TessLevelInner[0] = max(gl_TessLevelOuter[1], gl_TessLevelOuter[3]);
        gl_TessLevelInner[1] = max(gl_TessLevelOuter[0], gl_TessLevelOuter[2]);
    }
}