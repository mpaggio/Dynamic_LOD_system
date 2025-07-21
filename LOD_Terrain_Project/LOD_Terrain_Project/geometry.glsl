#version 460 core

//UNA ESECUZIONE PER OGNI PRIMITIVA GENERATA DAL TES


// --- LAYOUT --- //
layout(triangles) in;
layout(triangle_strip, max_vertices = 100) out; // aumentato per supportare più foglie


// --- INPUT & OUTPUT --- //
in vec4 worldPos[]; // Input dal Tessellation Evaluation Shader
in vec3 tes_normal[];

out vec4 gs_worldPos; // Output per il Fragment Shader
out vec3 gs_normal;
out int gs_isGrass; //se si tratta di erba
out int gs_isKelp; //se si tratta di alghe


// --- COSTANTI --- //
const float MAX_DISTANCE = 4.0f;

const float GRASS_BLADE_HEIGHT = 0.015; // altezza della foglia
const float GRASS_BLADE_WIDTH = 0.005; // larghezza della foglia
const float GRASS_OSC_AMP = 0.008; // ampiezza oscillazione (movimento laterale con il tempo)
const float GRASS_OSC_FREQ = 1.5; // frequenza oscillazione (più è alta, più è rapida)
const float GRASS_MAX_ANGLE = radians(20.0); // orientamento casuale massimo ±20°

const int KELP_SEGMENTS = 4; //numero di segmenti rettangolari
const float KELP_WIDTH = 0.005; //larghezza di ogni rettangolo
const float KELP_SEGMENT_LENGTH = 0.02; //altezza di ogni rettangolo
const float KELP_OSC_STRENGTH = 0.2; //forza dell'oscillazione

const float GRASS_MIN_TERRAIN_HEIGHT = 0.1;
const float GRASS_MAX_TERRAIN_HEIGHT = 0.6;
const float KELP_MAX_TERRAIN_HEIGHT = -0.6;


// --- UNIFORMS --- //
uniform bool useCharacterToTess; //variabile di controllo sull'uso del punto di riferimento
uniform float u_time; //tempo
uniform vec3 cameraPosition; //posizione della telecamera
uniform vec3 characterPosition; //posizione del character
uniform mat4 view; // Simula la camera
uniform mat4 proj; // Matrice di proiezione


float random(vec2 co) {
    // calcola il prodotto scalare tra il seme e un vettore costante, applica il seno per non linearizzare, 
    // moltiplica per un numero grande per spargere, prendi le cifre decimali i numeri sono scelti empiricamente 
    // per essere molto diversi fra loro e non legati
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453); //genera un numero in [0,1]
}

void generateGrass() {
    // Calcolo la normale
    vec3 n = normalize(cross(
        worldPos[1].xyz - worldPos[0].xyz,
        worldPos[2].xyz - worldPos[0].xyz
    ));
    if (n.y < 0.0) n = -n;

    // Calcolo tangente (per la larghezza della foglia)
    vec3 tangent = normalize(cross(n, vec3(0.0, 0.0, 1.0)));
    if (length(tangent) < 0.001)
        tangent = vec3(1.0, 0.0, 0.0);

    // Posizioni delle foglie (i 3 vertici, il centro di ogni lato, il centro del triangolo)
    vec4 positions[8];

    positions[0] = worldPos[0];
    positions[1] = worldPos[1];
    positions[2] = worldPos[2];
    positions[3] = (worldPos[0] + worldPos[1]) * 0.5;
    positions[4] = (worldPos[1] + worldPos[2]) * 0.5;
    positions[5] = (worldPos[2] + worldPos[0]) * 0.5;
    vec4 center = (worldPos[0] + worldPos[1] + worldPos[2]) / 3.0;
    positions[6] = center;
    positions[7] = center; //Raddoppiato perchè nel centro desidero avere due fasci di foglie

    // Per ogni posizione, disegna una foglia con orientamento random
    for (int i = 0; i < 8; ++i) {
        vec4 pos = positions[i];

        // Angolo casuale tra -maxAngle e +maxAngle
        float seed = pos.x * 12.57 + pos.z * 41.31;
        float angle = (random(vec2(seed, float(i))) - 0.5) * 2.0 * GRASS_MAX_ANGLE;

        // La direzione della tangente è ruotata rispetto alla normale di un certo angolo
        vec3 dir = normalize(tangent * cos(angle) + cross(n, tangent) * sin(angle));

        // Le due basi sono simmetriche rispetto a pos
        vec4 baseLeft = pos + vec4(-dir * GRASS_BLADE_WIDTH * 0.5, 0.0);
        vec4 baseRight = pos + vec4(dir * GRASS_BLADE_WIDTH * 0.5, 0.0);

        float phase = random(vec2(floor(pos.xz * 10.0) / 10.0)) * 6.28; //ogni foglia ha una fase random
        vec3 tipPos = pos.xyz + n * GRASS_BLADE_HEIGHT + dir * sin(u_time * GRASS_OSC_FREQ + phase) * GRASS_OSC_AMP; //La direzione viene aggiornata con la forma generale della sinusoide "A * sin(2 * pi * f * t + fase)"
        vec4 tip = vec4(tipPos, 1.0);

        gl_Position = proj * view * baseLeft;
        gs_worldPos = baseLeft;
        gs_normal = normalize(n);
        gs_isGrass = 1;
        gs_isKelp = 0;
        EmitVertex();

        gl_Position = proj * view * baseRight;
        gs_worldPos = baseRight;
        gs_normal = normalize(n);
        gs_isGrass = 1;
        gs_isKelp = 0;
        EmitVertex();

        gl_Position = proj * view * tip;
        gs_worldPos = tip;
        gs_normal = normalize(n);
        gs_isGrass = 1;
        gs_isKelp = 0;
        EmitVertex();

        EndPrimitive();
    }
}


void generateKelps() {
    vec4 baseCenter = (worldPos[0] + worldPos[1] + worldPos[2]) / 3.0; //coordinate del centro

    vec3 up = vec3(0.0, 1.0, 0.0); //vettore verso l'alto
    vec3 tangent = normalize(cross(up, vec3(0.0, 0.0, 1.0))); //versore ortogonale all'asse z e ad up
    vec3 bitangent = normalize(cross(up, tangent)); //versore ortogonale ad up e all'altro versore

    vec2 stableCoord = floor(baseCenter.xz * 10.0) / 10.0; // arrotonda a 0.1
    float seed = stableCoord.x * 3.17 + stableCoord.y * 7.31; //seme pseudo-casuale basato sulla posizione
    float baseAngle = random(vec2(seed, 0.0)) * radians(60.0) - radians(30.0); //angolo casuale fra -30° e 30° (prima mappa i risultati in [0°, 60°] e poi in [-30°, 30°])

    vec3 p0 = baseCenter.xyz; //coordinate xyz del centro
    vec3 dir = normalize(tangent * cos(baseAngle) * 0.2 + bitangent * sin(baseAngle) * 0.2 + up * 1.5); //prima direzione
    vec3 side = normalize(cross(dir, up)) * KELP_WIDTH * 0.5; //calcola la larghezza del primo segmento

    vec4 v0 = vec4(p0 - side, 1.0); //base 1
    vec4 v1 = vec4(p0 + side, 1.0); //base 2

    for (int i = 0; i < KELP_SEGMENTS; ++i) {
        float delta = (random(vec2(seed, float(i))) - 0.5) * radians(30.0); //variazione d'angolo fra -15° e 15° (prima mappa in [-0.5, 0.5] e poi in [-15, 15])
        float nextAngle = baseAngle + delta; //angolo con variazione
        
        //OSCILLAZIONE ORIZZONTALE
        vec3 baseDir = normalize(tangent * cos(nextAngle) * 0.2 + bitangent * sin(nextAngle) * 0.2 + up * 1.0); // Direzione iniziale verso l’alto con deviazione

        float phase = float(i) * 1.3; //fase diversa per ogni segmento
        float oscAngle = sin(u_time * 1.5 + phase) * radians(25.0); // +/-15° oscillazione

        vec3 oscillatedDir = normalize(baseDir + (tangent * cos(oscAngle) + bitangent * sin(oscAngle)) * KELP_OSC_STRENGTH);

        vec3 nextDir = oscillatedDir;

        vec3 p1 = p0 + nextDir * KELP_SEGMENT_LENGTH; //calcola la nuova posizione finale del segmento rettangolare
        
        vec3 nextSide = normalize(cross(nextDir, up)) * KELP_WIDTH * 0.5; //calcola l'asse perpendicolare alla direzione del segmento (a metà perchè è la distanza dal centro, ovvero la metà della width)

        vec4 v2 = vec4(p1 - nextSide, 1.0); //top 1
        vec4 v3 = vec4(p1 + nextSide, 1.0); //top 2

        // Primo triangolo
        vec3 normal = normalize(cross(v1.xyz - v0.xyz, v2.xyz - v0.xyz));
        gl_Position = proj * view * v0;
        gs_worldPos = v0;
        gs_normal = normalize(normal);
        gs_isGrass = 0;
        gs_isKelp = 1;
        EmitVertex();

        gl_Position = proj * view * v1;
        gs_worldPos = v1;
        gs_normal = normalize(normal);
        gs_isGrass = 0;
        gs_isKelp = 1;
        EmitVertex();

        gl_Position = proj * view * v2;
        gs_worldPos = v2;
        gs_normal = normalize(normal);
        gs_isGrass = 0;
        gs_isKelp = 1;
        EmitVertex();

        EndPrimitive();

        // Secondo triangolo
        vec3 normal2 = normalize(cross(v3.xyz - v1.xyz, v2.xyz - v1.xyz));
        gl_Position = proj * view * v2;
        gs_worldPos = v2;
        gs_normal = normalize(normal2);
        gs_isGrass = 0;
        gs_isKelp = 1;
        EmitVertex();

        gl_Position = proj * view * v1;
        gs_worldPos = v1;
        gs_normal = normalize(normal2);
        gs_isGrass = 0;
        gs_isKelp = 1;
        EmitVertex();

        gl_Position = proj * view * v3;
        gs_worldPos = v3;
        gs_normal = normalize(normal2);
        gs_isGrass = 0;
        gs_isKelp = 1;
        EmitVertex();

        EndPrimitive();

        p0 = p1; // nuova base
        v0 = v2; // nuovo lato sinistro
        v1 = v3; // nuovo lato destro
        baseAngle = nextAngle; //imposta il nuovo angolo di partenza
    }
}

void main() {
    // Disegna il triangolo del terreno
    for (int i = 0; i < 3; ++i) {
        gl_Position = proj * view * worldPos[i];
        gs_worldPos = worldPos[i];
        gs_normal = normalize(tes_normal[i]);
        gs_isGrass = 0;
        gs_isKelp = 0;
        EmitVertex();
    }
    EndPrimitive();


    // Scelta del punto di riferimento (character / camera)
    vec3 basePosition;
    if (useCharacterToTess) {
        basePosition = characterPosition;
    }
    else {
        basePosition = cameraPosition;
    }


    // LOD in base alla distanza
    vec3 center = (worldPos[0].xyz + worldPos[1].xyz + worldPos[2].xyz) / 3.0; //centro del triangolo
    bool isClose = (length(basePosition - center) < MAX_DISTANCE); //verifica se il punto di riferimento è vicino

    if (isClose) {
        float centerHeight = center.y;

        if (centerHeight > GRASS_MIN_TERRAIN_HEIGHT && centerHeight < GRASS_MAX_TERRAIN_HEIGHT) {
            generateGrass();
        }
        else if (centerHeight < KELP_MAX_TERRAIN_HEIGHT) {
            generateKelps();
        }
    }
}
