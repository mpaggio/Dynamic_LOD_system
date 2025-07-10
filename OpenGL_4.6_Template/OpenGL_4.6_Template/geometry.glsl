#version 460 core

//UNA ESECUZIONE PER OGNI PRIMITIVA GENERATA DAL TES

layout(triangles) in;
layout(triangle_strip, max_vertices = 100) out; // aumentato per supportare più foglie

in vec4 worldPos[]; // Input dal Tessellation Evaluation Shader
out vec4 gs_worldPos; // Output per il Fragment Shader
out int gs_isGrass; //se si tratta di erba
out int gs_isKelp; //se si tratta di alghe

uniform float u_time;

uniform mat4 view; // Simula la camera
uniform mat4 proj; // Matrice di proiezione

float random(vec2 co) {
    //calcola il prodotto scalare tra il seme e un vettore costante, applica il seno per non linearizzare, moltiplica per un numero grande per spargere, prendi le cifre decimali
    //i numeri sono scelti empiricamente per essere molto diversi fra loro e non legati
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

    // Parametri foglia
    float bladeHeight = 0.015;
    float bladeWidth = 0.005;
    float amp = 0.008; // ampiezza oscillazione (movimento laterale con il tempo)
    float freq = 1.5; // frequenza oscillazione (più è alta, più è rapida)
    float maxAngle = radians(20.0); // orientamento casuale massimo ±20°
    int phaseIndex = 0; // per animazione desincronizzata (fase diversa per ogni foglia)

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
        float angle = (random(vec2(seed, float(i))) - 0.5) * 2.0 * maxAngle;

        // La direzione della tangente è ruotata rispetto alla normale di un certo angolo
        vec3 dir = normalize(tangent * cos(angle) + cross(n, tangent) * sin(angle));

        // Le due basi sono simmetriche rispetto a pos
        vec4 baseLeft = pos + vec4(-dir * bladeWidth * 0.5, 0.0);
        vec4 baseRight = pos + vec4(dir * bladeWidth * 0.5, 0.0);

        float phase = float(phaseIndex++) * 1.1; //ogni foglia ha una fase diversa
        vec3 tipPos = pos.xyz + n * bladeHeight + dir * sin(u_time * freq + phase) * amp; //La direzione viene aggiornata con la forma generale della sinusoide "A * sin(2 * pi * f * t + fase)"
        vec4 tip = vec4(tipPos, 1.0);

        gl_Position = proj * view * baseLeft;
        gs_worldPos = baseLeft;
        gs_isGrass = 1;
        gs_isKelp = 0;
        EmitVertex();

        gl_Position = proj * view * baseRight;
        gs_worldPos = baseRight;
        gs_isGrass = 1;
        gs_isKelp = 0;
        EmitVertex();

        gl_Position = proj * view * tip;
        gs_worldPos = tip;
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

    int segments = 4; //numero di segmenti rettangolari
    float width = 0.005; //larghezza di ogni rettangolo
    float segmentLength = 0.02; //altezza di ogni rettangolo

    float seed = baseCenter.x * 3.17 + baseCenter.z * 7.31; //seme pseudo-casuale basato sulla posizione
    float baseAngle = random(vec2(seed, 0.0)) * radians(60.0) - radians(30.0); //angolo casuale fra -30° e 30° (prima mappa i risultati in [0°, 60°] e poi in [-30°, 30°])

    vec3 p0 = baseCenter.xyz; //coordinate xyz del centro
    vec3 dir = normalize(tangent * cos(baseAngle) * 0.2 + bitangent * sin(baseAngle) * 0.2 + up * 1.5); //prima direzione
    vec3 side = normalize(cross(dir, up)) * width * 0.5; //calcola la larghezza del primo segmento

    vec4 v0 = vec4(p0 - side, 1.0); //base 1
    vec4 v1 = vec4(p0 + side, 1.0); //base 2

    for (int i = 0; i < segments; ++i) {
        float delta = (random(vec2(seed, float(i))) - 0.5) * radians(30.0); //variazione d'angolo fra -15° e 15° (prima mappa in [-0.5, 0.5] e poi in [-15, 15])
        float nextAngle = baseAngle + delta; //angolo con variazione
        
        //OSCILLAZIONE ORIZZONTALE
        vec3 baseDir = normalize(tangent * cos(nextAngle) * 0.2 + bitangent * sin(nextAngle) * 0.2 + up * 1.0); // Direzione iniziale verso l’alto con deviazione

        float phase = float(i) * 1.3; //fase diversa per ogni segmento
        float oscStrength = 0.2; //forza dell'oscillazione
        float oscAngle = sin(u_time * 1.5 + phase) * radians(25.0); // +/-15° oscillazione

        vec3 oscillatedDir = normalize(baseDir + (tangent * cos(oscAngle) + bitangent * sin(oscAngle)) * oscStrength);

        vec3 nextDir = oscillatedDir;

        vec3 p1 = p0 + nextDir * segmentLength; //calcola la nuova posizione finale del segmento rettangolare
        
        vec3 nextSide = normalize(cross(nextDir, up)) * width * 0.5; //calcola l'asse perpendicolare alla direzione del segmento (a metà perchè è la distanza dal centro, ovvero la metà della width)

        vec4 v2 = vec4(p1 - nextSide, 1.0); //top 1
        vec4 v3 = vec4(p1 + nextSide, 1.0); //top 2

        // Primo triangolo
        gl_Position = proj * view * v0;
        gs_worldPos = v0;
        gs_isGrass = 0;
        gs_isKelp = 1;
        EmitVertex();

        gl_Position = proj * view * v1;
        gs_worldPos = v1;
        gs_isGrass = 0;
        gs_isKelp = 1;
        EmitVertex();

        gl_Position = proj * view * v2;
        gs_worldPos = v2;
        gs_isGrass = 0;
        gs_isKelp = 1;
        EmitVertex();

        EndPrimitive();

        // Secondo triangolo
        gl_Position = proj * view * v2;
        gs_worldPos = v2;
        gs_isGrass = 0;
        gs_isKelp = 1;
        EmitVertex();

        gl_Position = proj * view * v1;
        gs_worldPos = v1;
        gs_isGrass = 0;
        gs_isKelp = 1;
        EmitVertex();

        gl_Position = proj * view * v3;
        gs_worldPos = v3;
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
    // Disegna il triangolo terreno
    for (int i = 0; i < 3; ++i) {
        gl_Position = proj * view * worldPos[i];
        gs_worldPos = worldPos[i];
        gs_isGrass = 0;
        EmitVertex();
    }
    EndPrimitive();

    float centerHeight = (worldPos[0].y + worldPos[1].y + worldPos[2].y) / 3.0;
    if (centerHeight > 0.1 && centerHeight < 0.6) {
        generateGrass();
    }
    else if (centerHeight < -0.6) {
        generateKelps();
    }
}
