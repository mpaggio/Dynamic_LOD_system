#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "textureHandler.h"

extern Importer importer;

vector<GLuint> loadAllTextures() {
    const char* paths[] = {
        "./Texture/snowColor.png",
        "./Texture/snowNormal.png",
        "./Texture/rockColor.png",
        "./Texture/rockNormal.png",
        "./Texture/groundColor.png",
        "./Texture/groundNormal.png",
        "./Texture/sandColor.png",
        "./Texture/sandNormal.png"
    };
    vector<GLuint> texturesID;
    int size = sizeof(paths) / sizeof(paths[0]);

    for (int i = 0; i < size; i++) {
        GLuint textureID;
        glGenTextures(1, &textureID);

        int width, height, nrChannels;
        // Carica immagine (RGBA o RGB)
        unsigned char* data = stbi_load(paths[i], &width, &height, &nrChannels, 0);
        if (!data) {
            cerr << "Errore nel caricamento della texture: " << paths[i] << endl;
            continue;
        }

        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Impostazioni di wrapping e filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        texturesID.push_back(textureID);
    }

    return texturesID;
}

string extractFilename(const string& fullPath) {
    size_t pos = fullPath.find_last_of("\\/");
    if (pos == std::string::npos) {
        return fullPath;
    }
    return fullPath.substr(pos + 1);
}

GLuint loadTextureFromMaterial(aiMaterial* material, aiTextureType type, const string& directory) {
    aiString texturePath;
    bool hasTexture = (material->GetTexture(type, 0, &texturePath) == AI_SUCCESS);

    string filenameOnly;
   
    if (hasTexture) {
        filenameOnly = extractFilename(texturePath.C_Str());
    }
    else {
        filenameOnly = "Elk.png";
    }

    // Se la texture non è esattamente "Antler.png", usiamo "Elk.png"
    if (filenameOnly != "Antler.png") {
        filenameOnly = "Elk.png";
    }

    string filename = directory + "/" + filenameOnly;
    
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);

    if (!data) {
        cerr << "ATTENZIONE: Texture mancante o non caricabile: " << filename << endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLenum format;
    if (channels == 1)
        format = GL_RED;
    else if (channels == 3)
        format = GL_RGB;
    else if (channels == 4)
        format = GL_RGBA;
    else {
        cerr << "Formato texture non supportato: " << channels << " canali\n";
        stbi_image_free(data);
        return 0;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    return textureID;
}
