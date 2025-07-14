#include "lib.h"
#include "strutture.h"
#include "utilities.h"
#include "guiHandler.h"
#include "modelLoader.h"
#include "noiseHandler.h"
#include "shaderHandler.h"
#include "cameraHandler.h"
#include "bufferHandler.h"
#include "textureHandler.h"
#include "geometryHandler.h"
#include "interactionHandler.h"

int height = 600; //Altezza della finestra
int width = 600; //Larghezza della finestra

float Theta = -90.0f; //Angolo per la rotazione orizzontale
float Phi = 0.0f; //Angolo per la rotazione verticale
long long startTimeMillis = 0;

bool mouseLocked = true;
bool lineMode = true;

ViewSetup SetupTelecamera;
PerspectiveSetup SetupProspettiva;

extern vector<unsigned int> indices;
extern vector<BoneInfo> bone_info;
extern const aiScene* scene;

int main() {
    int division = 12;
    int numSpheres = 100;
    float offset = 5.0f;
    float terrainSize = 10.0f;
    float r_min = 0.01f, r_max = 0.05f;

    mat4 model = mat4(1.0f); //(Nessuna trasformazione) --> Qui potrei scalare, ruotare o traslare 
    mat4 view = lookAt(vec3(0.0f, 0.0f, 2.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)); //(Davanti all'origine)
    mat4 proj = perspective(radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f); //(FOV: 45, ASPECT: 4.3, ZNEAR: 0.1, ZFAR: 100)
    
    //GLFW
    glfwInit(); //Inizializzazione di GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); //Specifica a GLFW che verrà utilizzato OpenGL versione 4.x (specifica la versione maggiore)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6); //Specifica a GLFW che verrà utilizzato OpenGL versione 4.6 (specifica la versione minore)
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //Richiede un core profile di OpenGL (che esclude le funzionalità deprecate)

    //GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor(); // Prendi il monitor principale
    //const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor); // Prendi le modalità video del monitor (risoluzione, refresh rate, ecc)
    //height = mode->height;
    //width = mode->width;
    //GLFWwindow* window = glfwCreateWindow(width, height, "Tessellation Shader", primaryMonitor, nullptr); // Crea la finestra fullscreen con le dimensioni del monitor
    GLFWwindow* window = glfwCreateWindow(width, height, "Tessellation Shader", nullptr, nullptr);

    if (!window) { //Gestione dell'errore
        std::cerr << "Errore nella creazione della finestra GLFW\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window); //Attiva il contesto OpenGL associato alla finestra creata, rendendo il contesto corrente per il thread in cui viene chiamata


    //CALLBACKS
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    

    //Inizializzazione di GLAD (carica i puntatori alle funzioni OpenGL)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Errore nell'inizializzazione di GLAD\n";
        return -1;
    }


    //MAPPA
    vector<float> planeVertices = simplePlane(division, terrainSize);
    vector<float> planePatches = generatePatches(planeVertices, division);
    BufferPair planePair = INIT_PLANE_BUFFERS(planePatches);


    //TEXTURE
    GLuint fbmTexture = generateFBMTexture(512, 512, 8);


    //TRIANGOLI x SFERE
    vector<vec3> allSphereVertices;
    vector<vec3> allCenters;
    for (int i = 0; i < numSpheres; i++) {
        vec3 center = randomPosition(terrainSize);
        float radius = randomFloat(r_min, r_max);
        vector<vec3> sphereTriangleVerts = generateSphericalBase(center, radius);
        for (int j = 0; j < sphereTriangleVerts.size(); j++) {
            allCenters.push_back(center);
        }
        allSphereVertices.insert(allSphereVertices.end(), sphereTriangleVerts.begin(), sphereTriangleVerts.end());
    }
    BufferPair spherePair = INIT_SPHERE_BUFFERS(allSphereVertices, allCenters);


    //MODEL
    string path = "Model/Knight/source/Walking.fbx";
    loadModel(path);
    ModelBufferPair modelPair = INIT_MODEL_BUFFERS();


    //SHADER PROGRAMS
    unsigned int terrainProgram = createShaderProgram(
        "vertex.glsl",
        "tess_control.glsl",
        "tess_eval.glsl",
        "geometry.glsl",
        "fragment.glsl"
    ); 
    unsigned int sphereProgram = createShaderProgram(
        "vertex_sphere.glsl", 
        "tess_control_sphere.glsl", 
        "tess_eval_sphere.glsl", 
        "geometry_sphere.glsl", 
        "fragment_sphere.glsl"
    );
    unsigned int modelProgram = createSimpleShaderProgram(
        "vertex_model.glsl",
        "fragment_model.glsl"
    );


    //TEXTURES
    vector<GLuint> allTextures = loadAllTextures();


    //UNIFORMS
    //Terrain program
    int uTimeLocation = glGetUniformLocation(terrainProgram, "u_time");
    int modelLocation = glGetUniformLocation(terrainProgram, "model");
    int viewLocation = glGetUniformLocation(terrainProgram, "view");
    int projLocation = glGetUniformLocation(terrainProgram, "proj");
    int cameraPositionLocation = glGetUniformLocation(terrainProgram, "cameraPosition");
    //Sphere program
    int uTimeLocation_sphere = glGetUniformLocation(sphereProgram, "u_time");
    int modelLocation_sphere = glGetUniformLocation(sphereProgram, "model");
    int viewLocation_sphere = glGetUniformLocation(sphereProgram, "view");
    int projLocation_sphere = glGetUniformLocation(sphereProgram, "proj");
    int sphereOffset = glGetUniformLocation(sphereProgram, "sphereYOffset");
    int cameraPositionLocation_sphere = glGetUniformLocation(sphereProgram, "cameraPosition");
    //Model program
    int modelLoc = glGetUniformLocation(modelProgram, "model");
    int viewLoc = glGetUniformLocation(modelProgram, "view");
    int projLoc = glGetUniformLocation(modelProgram, "proj");
    GLuint bonesLoc = glGetUniformLocation(modelProgram, "bones");


    //SETTINGS
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Imposta la modalità Wireframe per vedere le suddivisioni fatte dallo shader
    glDisable(GL_CULL_FACE); //Disabilita il culling
    glEnable(GL_DEPTH_TEST); //Abilita il depth test


    //TELECAMERA
    INIT_CAMERA_PROJECTION();


    //GUI
    initializeGui(window); //Inizializza la finestra di interazione


    //TIME
    startTimeMillis = static_cast<long long>(glfwGetTime() * 1000.0);
    

    //MAIN LOOP
    while (!glfwWindowShouldClose(window)) {

        long long currentTimeMillis = static_cast<long long>(glfwGetTime() * 1000.0);
        float animationTimeSec = ((float)(currentTimeMillis - startTimeMillis)) / 1000.0f;

        if (lineMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Imposta la modalità Wireframe per vedere le suddivisioni fatte dallo shader
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //Imposta la modalità Fill per vedere le suddivisioni riempite
        }

        float timeValue = glfwGetTime(); //Restituisce il tempo in secondi dall'avvio

        glPatchParameteri(GL_PATCH_VERTICES, 4); //Dice a OpenGL che ogni patch ha 4 vertici
        

        //TERRAIN PROGRAM
        glUseProgram(terrainProgram);

        for (int i = 0; i < allTextures.size(); i++) { //Attiva le texture
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, allTextures[i]);
        }
        //Lega le texture alle relative variabili uniform
        for (int i = 0; i < allTextures.size(); i++) {
            std::string uniformName = "texture" + std::to_string(i);
            GLint location = glGetUniformLocation(terrainProgram, uniformName.c_str());
            glUniform1i(location, i);
        }

        glActiveTexture(GL_TEXTURE0 + 8);
        glBindTexture(GL_TEXTURE_2D, fbmTexture);
        glUniform1i(glGetUniformLocation(terrainProgram, "u_fbmTexture"), 8);

        glUniform1f(uTimeLocation, timeValue); //Aggiornamento della variabile uniforme
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, value_ptr(model));
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projLocation, 1, GL_FALSE, value_ptr(proj));
        glUniform3fv(cameraPositionLocation, 1, value_ptr(SetupTelecamera.position));

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); //Definisce il colore dello sfondo come grigio scuro
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Pulisci il color e il depth buffer
        
        glBindVertexArray(planePair.vao); //Attiva il VAO che contiene la configurazione dei vertici
        glDrawArrays(GL_PATCHES, 0, division * division * 4); //Disegna i vertici definiti nel VAO

        glPatchParameteri(GL_PATCH_VERTICES, 3); //Dice a OpenGL che ogni patch ha 4 vertici


        //SPHERE PROGRAM
        glUseProgram(sphereProgram);

        glUniform1f(sphereOffset, offset);
        glUniform1f(uTimeLocation_sphere, timeValue); //Aggiornamento della variabile uniforme
        glUniformMatrix4fv(modelLocation_sphere, 1, GL_FALSE, value_ptr(model));
        glUniformMatrix4fv(viewLocation_sphere, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projLocation_sphere, 1, GL_FALSE, value_ptr(proj));
        glUniform3fv(cameraPositionLocation_sphere, 1, value_ptr(SetupTelecamera.position));

        glBindVertexArray(spherePair.vao); //Attiva il VAO che contiene la configurazione dei vertici
        glDrawArrays(GL_PATCHES, 0, allSphereVertices.size()); //Disegna i vertici definiti nel VAO


        //MODEL PROGRAM
        glUseProgram(modelProgram);

        //aggiornamento dell'animazione del personaggio (se presente)
        if (scene && scene->mNumAnimations > 0 && scene->mAnimations[0]) {
            float ticksPerSecond = scene->mAnimations[0]->mTicksPerSecond != 0 ? scene->mAnimations[0]->mTicksPerSecond : 25.0f; //quanti tick al secondo
            float timeInTicks = animationTimeSec * ticksPerSecond; //quanti tick sono passati
            float animationTimeTicks = fmod(timeInTicks, scene->mAnimations[0]->mDuration); //prendo la parte decimale dell'operazione modulo (animazione continua)
            updateBoneTransforms(animationTimeTicks);
        }

        mat4 objectModel = mat4(1.0f);
        objectModel = translate(objectModel, vec3(3.0f));
        objectModel = scale(objectModel, vec3(0.005f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(objectModel));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(proj));

        mat4 boneTransforms[128];
        for (int i = 0; i < bone_info.size(); i++)
            boneTransforms[i] = bone_info[i].finalTransform;

        glUniformMatrix4fv(bonesLoc, bone_info.size(), GL_FALSE, value_ptr(boneTransforms[0]));

        glBindVertexArray(modelPair.vao);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);


        renderGui();
        glfwSwapBuffers(window); //Scambia il buffer frontale con quello posteriore
        glfwPollEvents(); //Controlla e gestisce gli eventi della finestra (input tastiera, mouse, ...)


        //MATRICI DI TRASFORMAZIONE
        view = lookAt(SetupTelecamera.position, SetupTelecamera.target, SetupTelecamera.upVector);
        proj = perspective(radians(SetupProspettiva.fovY), SetupProspettiva.aspect, SetupProspettiva.near_plane, SetupProspettiva.far_plane);
    

        process_input(window); //Gestione degli input da tastiera
    }


    //TERMINAZIONE
    glDeleteVertexArrays(1, &planePair.vao); //Elimina il VAO
    glDeleteBuffers(1, &planePair.vbo); //Elimina il VBO
    glDeleteVertexArrays(1, &spherePair.vao); //Elimina il VAO
    glDeleteBuffers(1, &spherePair.vbo); //Elimina il VBO
    glDeleteBuffers(1, &spherePair.centerVBO); //Elimino il VBO per i centri
    glDeleteProgram(terrainProgram); //Elimina lo shader program
    glDeleteProgram(sphereProgram); //Elimina lo shader program
    destroyGui();
    glfwDestroyWindow(window); //Elimina la finestra GLFW
    glfwTerminate(); //Termina la libreria GLFW, liberando tutte le risorse rimaste
    return 0;
}