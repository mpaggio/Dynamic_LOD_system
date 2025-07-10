#version 460 core

layout(vertices = 4) out; //Specifica che la patch output avrà 4 vertici, quindi la tessellazione sarà fatta su patch quadrate

uniform vec3 cameraPosition;
uniform sampler2D u_fbmTexture;

const int MIN_TES = 2;
const int MAX_TES = 32;
const float MIN_DIST = 0.0;
const float MAX_DIST = 5.0;

void main() {
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position; //Copia la posizione di ciascun vertice di input come output, senza modifiche (gl_InvocationID è l'indice dell'istanza corrente del vertex)

    //Solo per il vertice con indice 0 si impostano i livelli di tessellazione
    if (gl_InvocationID == 0) { 
        //Calcolo del centro dei vari lati del quadrilatero
        vec3 center0 = gl_in[0].gl_Position.xyz + (gl_in[3].gl_Position.xyz - gl_in[0].gl_Position.xyz) / 2.0; //Left
        vec3 center1 = gl_in[1].gl_Position.xyz + (gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz) / 2.0; //Bottom
        vec3 center2 = gl_in[2].gl_Position.xyz + (gl_in[1].gl_Position.xyz - gl_in[2].gl_Position.xyz) / 2.0; //Right
        vec3 center3 = gl_in[3].gl_Position.xyz + (gl_in[2].gl_Position.xyz - gl_in[3].gl_Position.xyz) / 2.0; //Top

        //Calcolo delle distanze fra i centri e la camer
        float dist0 = length(cameraPosition - center0); 
        float dist1 = length(cameraPosition - center1);
        float dist2 = length(cameraPosition - center2);
        float dist3 = length(cameraPosition - center3);

        //Calcolo dei livelli di tassellizzazione
        int tes0 = int(mix(MAX_TES, MIN_TES, clamp(dist0 / MAX_DIST, 0.0, 1.0)));
        int tes1 = int(mix(MAX_TES, MIN_TES, clamp(dist1 / MAX_DIST, 0.0, 1.0)));
        int tes2 = int(mix(MAX_TES, MIN_TES, clamp(dist2 / MAX_DIST, 0.0, 1.0)));
        int tes3 = int(mix(MAX_TES, MIN_TES, clamp(dist3 / MAX_DIST, 0.0, 1.0)));

        //Indica la suddivisione interna del patch (concretamente indica il numero di linee parallele a quelle dei lati della figura originale)
        gl_TessLevelInner[0] = max(tes1, tes3); //Lato interno in alto
        gl_TessLevelInner[1] = max(tes0, tes2); //Lato interno in basso

        //Indica la suddivisione sui bordi esterni del patch
        gl_TessLevelOuter[0] = tes0; //Lato di sinistra
        gl_TessLevelOuter[1] = tes1; //Lato in basso
        gl_TessLevelOuter[2] = tes2; //Lato a destra
        gl_TessLevelOuter[3] = tes3; //Lato in alto
    }
}