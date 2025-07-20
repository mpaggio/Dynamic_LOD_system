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
float moveSpeed = 0.002;
long long startTimeMillis = 0;

bool mouseLocked = true;
bool lineMode = true;
bool mainCharacter = true;

ViewSetup SetupTelecamera;
PerspectiveSetup SetupProspettiva;

pointLight light;

extern vector<unsigned int> indices;
extern vector<BoneInfo> bone_info_walking;
extern vector<BoneInfo> bone_info_standing;
extern const aiScene* scene_walking;
extern const aiScene* scene_standing;

int main() {
    int division = 14;
    int numSpheres = 100;
    float offset = 5.0f;
    float terrainSize = 20.0f;
    float r_min = 0.01f, r_max = 0.05f;

    mat4 model = mat4(1.0f); //(Nessuna trasformazione) --> Qui potrei scalare, ruotare o traslare 
    mat4 view = lookAt(vec3(0.0f, 0.0f, 2.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)); //(Davanti all'origine)
    mat4 proj = perspective(radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f); //(FOV: 45, ASPECT: 4.3, ZNEAR: 0.1, ZFAR: 100)
    
    //GLFW
    glfwInit(); //Inizializzazione di GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); //Specifica a GLFW che verrà utilizzato OpenGL versione 4.x (specifica la versione maggiore)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6); //Specifica a GLFW che verrà utilizzato OpenGL versione 4.6 (specifica la versione minore)
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //Richiede un core profile di OpenGL (che esclude le funzionalità deprecate)

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor(); // Prendi il monitor principale
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor); // Prendi le modalità video del monitor (risoluzione, refresh rate, ecc)
    height = mode->height;
    width = mode->width;
    GLFWwindow* window = glfwCreateWindow(width, height, "Tessellation Shader", primaryMonitor, nullptr); // Crea la finestra fullscreen con le dimensioni del monitor
    //GLFWwindow* window = glfwCreateWindow(width, height, "Tessellation Shader", nullptr, nullptr);

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


    //ILLUMINAZIONE
    light.position = { 10.0, 50.0, -10.0 };
    light.color = { 1.0,1.0,1.0 };
    light.power = 3.0f;


    //MAPPA
    vector<float> planeVertices = simplePlane(division, terrainSize);
    vector<float> planePatches = generatePatches(planeVertices, division);
    BufferPair planePair = INIT_PLANE_BUFFERS(planePatches);
    GLuint tfBuffer = INIT_TRANSFORM_FEEDBACK_BUFFERS();


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
    //Texture
    string path = "Model/Knight/source/castle_guard_01.fbx";
    //extractEmbeddedTextures(path, "Model/Knight/textures");
    //Walking
    path = "Model/Knight/source/Walking.fbx";
    loadModel(path, WALKING);
    ModelBufferPair walkingModelPair = INIT_MODEL_BUFFERS();
    //Standing
    path = "Model/Knight/source/Standing.fbx";
    loadModel(path, STANDING);
    ModelBufferPair standingModelPair = INIT_MODEL_BUFFERS();


    //SKYBOX
    vector<float> skyboxVertices = generateSkyboxCube();
    BufferPair skyboxPair = INIT_SKYBOX_BUFFERS(skyboxVertices);


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
    unsigned int skyboxProgram = createSimpleShaderProgram(
        "vertex_skybox.glsl",
        "fragment_skybox.glsl"
    );
    unsigned int transformFeedbackProgram = createTransformFeedbackShaderProgram(
        "vertex_fs.glsl",
        "tess_control_fs.glsl",
        "tess_eval_fs.glsl",
        "geometry_fs.glsl"
    );


    //TEXTURES
    GLuint fbmTexture = generateFBMTexture(512, 512, 8);
    vector<GLuint> allTextures = loadAllTextures();
    GLuint modelTexture = loadSingleTexture("Model/Knight/textures/texture_embedded_0.png");
    GLuint skyboxTexture = loadSkybox();


    //UNIFORMS
    //Terrain program
    int uTimeLocation = glGetUniformLocation(terrainProgram, "u_time");
    int modelLocation = glGetUniformLocation(terrainProgram, "model");
    int viewLocation = glGetUniformLocation(terrainProgram, "view");
    int projLocation = glGetUniformLocation(terrainProgram, "proj");
    int terrainSizeTCS = glGetUniformLocation(terrainProgram, "terrainSize_tcs");
    int terrainSizeTES = glGetUniformLocation(terrainProgram, "terrainSize_tes");
    int cameraPosLocTerrain = glGetUniformLocation(terrainProgram, "ViewPos");
    int lightPosLocTerrain = glGetUniformLocation(terrainProgram, "light.position");
    int lightColorLocTerrain = glGetUniformLocation(terrainProgram, "light.color");
    int lightPowerLocTerrain = glGetUniformLocation(terrainProgram, "light.power");
    int cameraPositionLocation = glGetUniformLocation(terrainProgram, "cameraPosition");
    int characterPositionLocation = glGetUniformLocation(terrainProgram, "characterPosition");
    int useCharacterToTessLocation = glGetUniformLocation(terrainProgram, "useCharacterToTess");
    //Transform feedback program
    int modelLocation_tf = glGetUniformLocation(transformFeedbackProgram, "model");
    int viewLocation_tf = glGetUniformLocation(transformFeedbackProgram, "view");
    int projLocation_tf = glGetUniformLocation(transformFeedbackProgram, "proj");
    int terrainSizeTCS_tf = glGetUniformLocation(transformFeedbackProgram, "terrainSize_tcs");
    int terrainSizeTES_tf = glGetUniformLocation(transformFeedbackProgram, "terrainSize_tes");
    int cameraPositionLocation_tf = glGetUniformLocation(transformFeedbackProgram, "cameraPosition");
    int characterPositionLocation_tf = glGetUniformLocation(transformFeedbackProgram, "characterPosition");
    int useCharacterToTessLocation_tf = glGetUniformLocation(transformFeedbackProgram, "useCharacterToTess");
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
    int cameraPosLoc = glGetUniformLocation(modelProgram, "ViewPos");
    int lightPosLoc = glGetUniformLocation(modelProgram, "light.position");
    int lightColorLoc = glGetUniformLocation(modelProgram, "light.color");
    int lightPowerLoc = glGetUniformLocation(modelProgram, "light.power");
    GLuint bonesLoc = glGetUniformLocation(modelProgram, "bones");
    //Skybox program
    int viewLocSkybox = glGetUniformLocation(skyboxProgram, "View");
    int projLocSkybox = glGetUniformLocation(skyboxProgram, "Projection");


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


    //MODEL MOVEMENT
    vec3 modelMovement = vec3(0.0f);
    vec3 previousModelMovement = vec3(0.0f);
    vec3 modelWorldPos = vec3(0.0f); //posizione assoluta del modello in world space
    mat4 tiltMatrix = mat4(1.0f);
    float rotationAngle = 0.0f;


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

        glPatchParameteri(GL_PATCH_VERTICES, 3); //Dice a OpenGL che ogni patch ha 4 vertici

        //SPHERE PROGRAM
        glUseProgram(sphereProgram);

        glUniform1f(sphereOffset, offset);
        glUniform1f(uTimeLocation_sphere, timeValue); //Aggiornamento della variabile uniforme
        glUniformMatrix4fv(modelLocation_sphere, 1, GL_FALSE, value_ptr(model));
        glUniformMatrix4fv(viewLocation_sphere, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projLocation_sphere, 1, GL_FALSE, value_ptr(proj));
        glUniform3fv(cameraPositionLocation_sphere, 1, value_ptr(SetupTelecamera.position));

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); //Definisce il colore dello sfondo come grigio scuro
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Pulisci il color e il depth buffer

        glBindVertexArray(spherePair.vao); //Attiva il VAO che contiene la configurazione dei vertici
        glDrawArrays(GL_PATCHES, 0, allSphereVertices.size()); //Disegna i vertici definiti nel VAO

        glPatchParameteri(GL_PATCH_VERTICES, 4); //Dice a OpenGL che ogni patch ha 4 vertici

        //MODEL PROGRAM
        glUseProgram(modelProgram);

        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, modelTexture);
        string uniformName = "modelTexture";
        GLint location = glGetUniformLocation(modelProgram, uniformName.c_str());
        glUniform1i(location, 0);

        ModelState state;
        bool isMoving = length(modelMovement - previousModelMovement) > 0.00001f;
        state = isMoving ? WALKING : STANDING;

        //aggiornamento dell'animazione del personaggio (se presente)
        if (state == WALKING) {
            if (scene_walking && scene_walking->mNumAnimations > 0 && scene_walking->mAnimations[0]) {
                float ticksPerSecond = scene_walking->mAnimations[0]->mTicksPerSecond != 0 ? scene_walking->mAnimations[0]->mTicksPerSecond : 25.0f; //quanti tick al secondo
                float timeInTicks = animationTimeSec * ticksPerSecond; //quanti tick sono passati
                float animationTimeTicks = fmod(timeInTicks, scene_walking->mAnimations[0]->mDuration); //prendo la parte decimale dell'operazione modulo (animazione continua)
                updateBoneTransforms(animationTimeTicks, state);
            }
        }
        else {
            if (scene_standing && scene_standing->mNumAnimations > 0 && scene_standing->mAnimations[0]) {
                float ticksPerSecond = scene_standing->mAnimations[0]->mTicksPerSecond != 0 ? scene_standing->mAnimations[0]->mTicksPerSecond : 25.0f; //quanti tick al secondo
                float timeInTicks = animationTimeSec * ticksPerSecond; //quanti tick sono passati
                float animationTimeTicks = fmod(timeInTicks, scene_standing->mAnimations[0]->mDuration); //prendo la parte decimale dell'operazione modulo (animazione continua)
                updateBoneTransforms(animationTimeTicks, state);
            }
        }

        mat4 objectModel = mat4(1.0f);
        objectModel = translate(objectModel, modelWorldPos);
        objectModel *= tiltMatrix;
        objectModel = scale(objectModel, vec3(0.0005f));
        objectModel = rotate(objectModel, radians(float(180)), vec3(0.0f, 1.0f, 0.0f));
        objectModel = rotate(objectModel, radians(rotationAngle), vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(objectModel));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(proj));
        glUniform3fv(cameraPosLoc, 1, value_ptr(SetupTelecamera.position));
        glUniform3fv(lightPosLoc, 1, value_ptr(light.position));
        glUniform3fv(lightColorLoc, 1, value_ptr(light.color));
        glUniform1f(lightPowerLoc, light.power);

        mat4 boneTransforms[128];

        if (state == WALKING) {
            for (int i = 0; i < bone_info_walking.size(); i++)
                boneTransforms[i] = bone_info_walking[i].finalTransform;

            glUniformMatrix4fv(bonesLoc, bone_info_walking.size(), GL_FALSE, value_ptr(boneTransforms[0]));
        }
        else {
            for (int i = 0; i < bone_info_standing.size(); i++)
                boneTransforms[i] = bone_info_standing[i].finalTransform;

            glUniformMatrix4fv(bonesLoc, bone_info_standing.size(), GL_FALSE, value_ptr(boneTransforms[0]));
        }


        glBindVertexArray(walkingModelPair.vao);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);


        //TRANSFORM FEEDBACK PROGRAM
        glUseProgram(transformFeedbackProgram);

        for (int i = 0; i < allTextures.size(); i++) { //Attiva le texture
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, allTextures[i]);
        }
        //Lega le texture alle relative variabili uniform
        for (int i = 0; i < allTextures.size(); i++) {
            string uniformName = "texture" + std::to_string(i);
            GLint location = glGetUniformLocation(transformFeedbackProgram, uniformName.c_str());
            glUniform1i(location, i);
        }

        glActiveTexture(GL_TEXTURE0 + 8);
        glBindTexture(GL_TEXTURE_2D, fbmTexture);
        glUniform1i(glGetUniformLocation(transformFeedbackProgram, "u_fbmTexture"), 8);

        glUniform1i(useCharacterToTessLocation_tf, int(mainCharacter));
        glUniformMatrix4fv(modelLocation_tf, 1, GL_FALSE, value_ptr(model));
        glUniformMatrix4fv(viewLocation_tf, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projLocation_tf, 1, GL_FALSE, value_ptr(proj));
        glUniform1f(terrainSizeTCS_tf, terrainSize);
        glUniform1f(terrainSizeTES_tf, terrainSize);
        glUniform3fv(cameraPositionLocation_tf, 1, value_ptr(SetupTelecamera.position));
        glUniform3fv(characterPositionLocation_tf, 1, value_ptr(modelWorldPos));

        glBindVertexArray(planePair.vao); //Attiva il VAO che contiene la configurazione dei vertici
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tfBuffer);
        glBeginTransformFeedback(GL_POINTS);
        glDrawArrays(GL_PATCHES, 0, division * division * 4); //Disegna i vertici definiti nel VAO
        glEndTransformFeedback();

        glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, tfBuffer);
        FeedbackData* feedbackData = (FeedbackData*)glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_READ_ONLY);
        vec3 groundNormal = vec3(0.0f, 1.0f, 0.0f);

        if (feedbackData) {
            modelWorldPos.y = feedbackData[0].pos.y;
            groundNormal = feedbackData[0].normal;
            glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);
        }

        if (modelWorldPos.x < 0.0f || modelWorldPos.x > terrainSize || modelWorldPos.z > 0.0f || modelWorldPos.z < -terrainSize) {
            modelWorldPos.y = 0.0f;
        }


        //TERRAIN PROGRAM
        glUseProgram(terrainProgram);

        for (int i = 0; i < allTextures.size(); i++) { //Attiva le texture
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, allTextures[i]);
        }
        //Lega le texture alle relative variabili uniform
        for (int i = 0; i < allTextures.size(); i++) {
            string uniformName = "texture" + std::to_string(i);
            GLint location = glGetUniformLocation(terrainProgram, uniformName.c_str());
            glUniform1i(location, i);
        }

        glActiveTexture(GL_TEXTURE0 + 8);
        glBindTexture(GL_TEXTURE_2D, fbmTexture);
        glUniform1i(glGetUniformLocation(terrainProgram, "u_fbmTexture"), 8);

        glUniform1i(useCharacterToTessLocation, int(mainCharacter));
        glUniform1f(uTimeLocation, timeValue); //Aggiornamento della variabile uniforme
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, value_ptr(model));
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projLocation, 1, GL_FALSE, value_ptr(proj));
        glUniform1f(terrainSizeTCS, terrainSize);
        glUniform1f(terrainSizeTES, terrainSize);
        glUniform3fv(cameraPositionLocation, 1, value_ptr(SetupTelecamera.position));
        glUniform3fv(characterPositionLocation, 1, value_ptr(modelWorldPos));
        glUniform3fv(cameraPosLocTerrain, 1, value_ptr(SetupTelecamera.position));
        glUniform3fv(lightPosLocTerrain, 1, value_ptr(light.position));
        glUniform3fv(lightColorLocTerrain, 1, value_ptr(light.color));
        glUniform1f(lightPowerLocTerrain, light.power);

        glBindVertexArray(planePair.vao); //Attiva il VAO che contiene la configurazione dei vertici
        glDrawArrays(GL_PATCHES, 0, division * division * 4); //Disegna i vertici definiti nel VAO


        //SKYBOX
        glDepthFunc(GL_LEQUAL);       // per permettere la skybox in fondo
        glDepthMask(GL_FALSE);        // disattiva scrittura nello z-buffer

        glUseProgram(skyboxProgram);
        
        glUniform1i(glGetUniformLocation(skyboxProgram, "skybox"), 0);
        glUniformMatrix4fv(viewLocSkybox, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projLocSkybox, 1, GL_FALSE, value_ptr(proj));

        glBindVertexArray(skyboxPair.vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);         // riattiva scrittura per gli oggetti normali
        glDepthFunc(GL_LESS);         // ripristina depth test standard


        renderGui();
        glfwSwapBuffers(window); //Scambia il buffer frontale con quello posteriore
        glfwPollEvents(); //Controlla e gestisce gli eventi della finestra (input tastiera, mouse, ...)


        //MATRICI DI TRASFORMAZIONE
        view = lookAt(SetupTelecamera.position, SetupTelecamera.target, SetupTelecamera.upVector);
        proj = perspective(radians(SetupProspettiva.fovY), SetupProspettiva.aspect, SetupProspettiva.near_plane, SetupProspettiva.far_plane);
    
        auto inputResult = process_input(window);
        previousModelMovement = modelMovement;
        if (length(inputResult.first) > 0.0001f) {
            modelMovement += inputResult.first;
            modelWorldPos += inputResult.first;
            rotationAngle = inputResult.second;
        }

        //TILT DELLA ROTAZIONE
        vec3 modelUp = vec3(0.0f, 1.0f, 0.0f);
        vec3 normal = normalize(groundNormal);
        quat tiltQuat = rotation(modelUp, normal);
        tiltMatrix = toMat4(tiltQuat);
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