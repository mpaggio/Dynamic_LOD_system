#include "modelLoader.h"
#include "textureHandler.h"

vector<vec3> positions;
vector<vec3> normals;
vector<vec3> texCoords;
vector<unsigned int> indices;

vector<BoneInfo> bone_info; //contiene offset matrix e la trasformazione animata finale

vector<VertexBoneData> vertices_to_bones; //mapping dai vertici alle informazioni delle ossa che li influenzano (mapping inverso)
vector<int> mesh_vertices; //contiene l'indice iniziale di ogni mesh all'interno dell'array globale dei vertici
map<string, unsigned int> bone_name_to_index; //mapping dai nomi delle ossa agli indici relativi


mat4 aiMatrix4x4_to_mat4(const aiMatrix4x4& from) {
    mat4 to;
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

int getBoneID(const aiBone* bone) {
    int boneID = 0;
    string boneName = bone->mName.C_Str(); //recupero il nome dell'osso

    if (bone_name_to_index.find(boneName) == bone_name_to_index.end()) { //se il nome non è stato trovato
        boneID = bone_name_to_index.size(); //indice della nuova posizione (in fondo)
        bone_name_to_index[boneName] = boneID; //inserisco nella posizione data l'indice del nuovo osso

        // Assicurati che bone_info abbia spazio
        if (bone_info.size() <= boneID)
            bone_info.resize(boneID + 1);

        // Salva la offset matrix dell'osso
        bone_info[boneID].offsetMatrix = aiMatrix4x4_to_mat4(bone->mOffsetMatrix);
        bone_info[boneID].finalTransform = mat4(1.0f);
    }
    else { //se il nome è stato trovato
        boneID = bone_name_to_index[boneName]; //prendo il suo indice
    }

    return boneID;
}

void loadMeshBones(const int meshIndex, const aiMesh* mesh) {
    for (int i = 0; i < mesh->mNumBones; i++) {
        const aiBone* bone = mesh->mBones[i];
        
        int boneID = getBoneID(bone); //recupero l'ID dell'osso

        //printf("Bone %d \'%s\': num vertices affected (%d)\n", i, bone->mName.C_Str(), bone->mNumWeights);

        for (int j = 0; j < bone->mNumWeights; j++) {
            const aiVertexWeight& vertexWeight = bone->mWeights[j];

            unsigned int globalVertexID = mesh_vertices[meshIndex] + vertexWeight.mVertexId; //calcolo l'indice del vertice nell'array globale

            vertices_to_bones[globalVertexID].addBone(boneID, vertexWeight.mWeight); //aggiungo le informazioni dell'osso ai dati del vertice

            //printf("Bone weigth %d: vertex id (%d), value (%f)\n", j, vertexWeight.mVertexId, vertexWeight.mWeight);
        }
    }
}

void printSceneData(const aiScene* scene) {
    printf("Parsing %d meshes\n\n", scene->mNumMeshes);

    int total_vertices = 0;
    int total_indices = 0;
    int total_bones = 0;

    mesh_vertices.resize(scene->mNumMeshes); //inizializzo con il numero effettivo di mesh

    for (int i = 0; i < scene->mNumMeshes; i++) {
        const aiMesh* mesh = scene->mMeshes[i];
        total_vertices += mesh->mNumVertices;
        total_indices += mesh->mNumFaces * 3;
        total_bones += mesh->mNumBones;
    }

    positions.reserve(total_vertices);
    normals.reserve(total_vertices);
    texCoords.reserve(total_vertices);
    indices.reserve(total_indices);

    vertices_to_bones.resize(total_vertices); //inizializzo con il numero effettivo di vertici

    int vertex_offset = 0;
    for (int i = 0; i < scene->mNumMeshes; i++) {
        const aiMesh* mesh = scene->mMeshes[i];
        int num_vertices = mesh->mNumVertices;
        int num_indices = mesh->mNumFaces * 3;
        int num_bones = mesh->mNumBones;

        mesh_vertices[i] = vertex_offset; //assegno a ciascuna mesh l'indice di partenza corrispondente nell'array globale dei vertici
        
        printf("Mesh %d \'%s\': vertices (%d), indices (%d), bones (%d)\n", i, mesh->mName.C_Str(), num_vertices, num_indices, num_bones);

        // Vertici
        for (int v = 0; v < num_vertices; v++) {
            unsigned int globalVertexID = mesh_vertices[i] + v;
            VertexBoneData& vertex = vertices_to_bones[globalVertexID];

            // Position
            aiVector3D pos = mesh->mVertices[v];
            positions.push_back(vec3(pos.x, pos.y, pos.z));

            // Normal
            if (mesh->HasNormals()) {
                aiVector3D normal = mesh->mNormals[v];
                normals.push_back(vec3(normal.x, normal.y, normal.z));
            }
            else {
                aiVector3D backupNormal(0.0f, 1.0f, 0.0f);
                normals.push_back(vec3(backupNormal.x, backupNormal.y, backupNormal.z));
            }

            // Texture coords
            if (mesh->HasTextureCoords(0)) {
                aiVector3D uv = mesh->mTextureCoords[0][v];
                texCoords.push_back(vec3(uv.x, uv.y, uv.z));
            }
            else {
                aiVector3D backupUV(0.0f, 0.0f, 0.0f);
                texCoords.push_back(vec3(backupUV.x, backupUV.y, backupUV.z));
            }
        }

        // Indici
        for (int f = 0; f < mesh->mNumFaces; f++) {
            const aiFace& face = mesh->mFaces[f];
            indices.push_back(mesh_vertices[i] + face.mIndices[0]);
            indices.push_back(mesh_vertices[i] + face.mIndices[1]);
            indices.push_back(mesh_vertices[i] + face.mIndices[2]);
        }
    
        // Dati ossa
        if (mesh->HasBones()) {
            loadMeshBones(i, mesh);
        }

        vertex_offset += num_vertices;
    }

    for (auto& v : vertices_to_bones) {
        v.normalize();
    }

    printf("\n=== DEBUG VERTEX-BONE DATA ===\n");
    for (int i = 0; i < std::min((int)vertices_to_bones.size(), 10); i++) {
        VertexBoneData& v = vertices_to_bones[i];
        printf("Vertex %d - BIDs: [%d, %d, %d, %d] | Ws: [%.2f, %.2f, %.2f, %.2f] | sum = %.2f\n",
            i,
            v.boneIDs[0], v.boneIDs[1], v.boneIDs[2], v.boneIDs[3],
            v.weights[0], v.weights[1], v.weights[2], v.weights[3],
            v.weights[0] + v.weights[1] + v.weights[2] + v.weights[3]
        );
    }
}

void printModelData(string modelPath) {
    Importer printerImporter;
    const aiScene* scene = printerImporter.ReadFile(
        modelPath,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices
    );

    if (!scene || !scene->HasMeshes()) {
        printf("Failed to load model or no meshes found\n");
        return;
    }

    printSceneData(scene);
}