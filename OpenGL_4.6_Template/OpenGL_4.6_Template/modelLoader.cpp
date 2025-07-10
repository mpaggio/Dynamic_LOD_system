#include "modelLoader.h"
#include "textureHandler.h"

Importer importer;
map<string, int> boneMapping;          // Nome -> ID
vector<mat4> boneOffsetMatrices;       // Offset per ciascun bone
unsigned int boneCount = 0;

void extractBoneWeightsForMesh(aiMesh* mesh, vector<SimpleVertex>& vertices, size_t vertexStartIndex) {
    for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
        string boneName(mesh->mBones[boneIndex]->mName.C_Str());

        int boneID;
        if (boneMapping.find(boneName) == boneMapping.end()) {
            boneID = boneCount++;
            boneMapping[boneName] = boneID;

            mat4 offset;
            aiMatrix4x4 m = mesh->mBones[boneIndex]->mOffsetMatrix;
            offset = glm::transpose(glm::make_mat4(&m.a1)); // Assimp usa column-major
            boneOffsetMatrices.push_back(offset);
        }
        else {
            boneID = boneMapping[boneName];
        }

        // Associa il bone ai vertici influenzati
        aiBone* bone = mesh->mBones[boneIndex];
        for (unsigned int weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
            unsigned int vertexID = bone->mWeights[weightIndex].mVertexId;
            float weight = bone->mWeights[weightIndex].mWeight;

            if (vertexID >= mesh->mNumVertices) {
                cerr << "VertexID " << vertexID << " out of bounds in mesh with "
                    << mesh->mNumVertices << " vertices.\n";
                continue;
            }

            for (int i = 0; i < 4; ++i) {
                if (vertices[vertexStartIndex + vertexID].weights[i] == 0.0f) {
                    vertices[vertexStartIndex + vertexID].boneIDs[i] = boneID;
                    vertices[vertexStartIndex + vertexID].weights[i] = weight;
                    break;
                }
            }
        }
    }
}

mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& mat) {
    return glm::make_mat4(&mat.a1);
}

void processNode(aiNode* node, const aiScene* scene, const glm::mat4& parentTransform,
    vector<SimpleVertex>& vertices, const vector<vec3>& materialColors) {
    glm::mat4 nodeTransform = parentTransform * aiMatrix4x4ToGlm(node->mTransformation);

    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        int matIndex = mesh->mMaterialIndex;
        vec3 diffuseColor = materialColors[matIndex];

        // Pre-alloca spazio per i vertici
        size_t startIndex = vertices.size();
        vertices.resize(startIndex + mesh->mNumVertices);

        for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
            SimpleVertex& vertex = vertices[startIndex + j];
            vec4 pos = vec4(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z, 1.0f);
            vec3 norm = vec3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z);

            vertex.position = vec3(nodeTransform * pos);
            vertex.normal = normalize(mat3(transpose(inverse(nodeTransform))) * norm);
            vertex.color = diffuseColor;
            
            for (int k = 0; k < 4; ++k) {
                vertex.boneIDs[k] = 0;
                vertex.weights[k] = 0.0f;
            }
        }

        // Estrai pesi ossei
        extractBoneWeightsForMesh(mesh, vertices, startIndex);
    }

    // Ricorsione
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene, nodeTransform, vertices, materialColors);
    }
}

tuple<vector<Vertex>, vector<GLuint>> loadFBX() {
    vector<Vertex> vertices;
    vector<GLuint> textures;
    string modelPath = "Model/Deer/source/Deer Pose.fbx";

    const aiScene* scene = importer.ReadFile(
        modelPath,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices
    );

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        cerr << "Assimp error: " << importer.GetErrorString() << endl;
        return { vertices, textures }; // vuoti
    }

    // Carica le texture per tutti i materiali
    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* material = scene->mMaterials[i];
        GLuint texID = loadTextureFromMaterial(material, aiTextureType_DIFFUSE, "Model/Deer/textures");
        textures.push_back(texID);
    }

    // Per ogni mesh, estrai i vertici e assegna l'indice del materiale
    for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) {
        aiMesh* mesh = scene->mMeshes[meshIndex];
        int matIndex = mesh->mMaterialIndex;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            vertex.position = glm::vec3(
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z
            );
            vertex.normal = glm::vec3(
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            );
            if (mesh->mTextureCoords[0]) {
                vertex.texCoords = glm::vec2(
                    mesh->mTextureCoords[0][i].x,
                    mesh->mTextureCoords[0][i].y
                );
            }
            else {
                vertex.texCoords = glm::vec2(0.0f, 0.0f);
            }

            vertex.textureIndex = matIndex;
            vertices.push_back(vertex);
        }
    }

    return { vertices, textures };
}

vector<SimpleVertex> loadSimpleFBX(string modelPath) {
    vector<SimpleVertex> vertices;

    const aiScene* scene = importer.ReadFile(
        modelPath,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices
    );

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        cerr << "Assimp error: " << importer.GetErrorString() << endl;
        return vertices;
    }

    // Colori per ogni materiale
    vector<vec3> materialColors(scene->mNumMaterials);

    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* material = scene->mMaterials[i];
        aiColor3D color(1.0f, 1.0f, 1.0f); // Default bianco
        material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        materialColors[i] = vec3(color.r, color.g, color.b);
    }

    processNode(scene->mRootNode, scene, mat4(1.0f), vertices, materialColors);

    return vertices;
}

void printSceneData(const aiScene* scene) {
    printf("Parsing %d meshes\n\n", scene->mNumMeshes);
}

void printModelData(string modelPath) {
    Importer printerImporter;
    const aiScene* scene = printerImporter.ReadFile(
        modelPath,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices
    );

    printSceneData(scene);
}