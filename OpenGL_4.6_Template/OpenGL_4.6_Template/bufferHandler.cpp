#include "bufferHandler.h"

BufferPair INIT_PLANE_BUFFERS(vector<float> planeVertices) {
	BufferPair pair;

	glGenVertexArrays(1, &pair.vao); //Genera un VAO 
	glGenBuffers(1, &pair.vbo); //Genera un VBO

	glBindVertexArray(pair.vao); //Attiva il VAO appena generato
	glBindBuffer(GL_ARRAY_BUFFER, pair.vbo); //Collega il VBO al target GL_ARRAY_BUFFER

	glBufferData(GL_ARRAY_BUFFER, planeVertices.size() * sizeof(float), planeVertices.data(), GL_STATIC_DRAW); //Copia i dati del piano nella memoria della GPU
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); //Dice a OpenGL come leggere i dati nel VBO per usarli negli shader
	glEnableVertexAttribArray(0); //Attiva l'attributo numero 0 (OpenGL userà i dati collegati a quell'attributo durante il rendering)

	return pair;
}

BufferPair INIT_QUAD_BUFFERS(float* vertices, size_t count) {
	BufferPair pair;
	
	glGenVertexArrays(1, &pair.vao); //Genera un VAO 
	glGenBuffers(1, &pair.vbo); //Genera un VBO

	glBindVertexArray(pair.vao); //Attiva il VAO appena generato
	glBindBuffer(GL_ARRAY_BUFFER, pair.vbo); //Collega il VBO al target GL_ARRAY_BUFFER

	glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), vertices, GL_STATIC_DRAW); //Copia i dati del quadrilatero nella memoria della GPU
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); //Dice a OpenGL come leggere i dati nel VBO per usarli negli shader
	glEnableVertexAttribArray(0); //Attiva l'attributo numero 0 (OpenGL userà i dati collegati a quell'attributo durante il rendering)

	return pair;
}

BufferPair INIT_SPHERE_BUFFERS(vector<vec3> instancePositions, vector<vec3> allCenters) {
	BufferPair pair;

	glGenVertexArrays(1, &pair.vao);
    glBindVertexArray(pair.vao);

    // --- VBO per le posizioni ---
    glGenBuffers(1, &pair.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, pair.vbo);
    glBufferData(GL_ARRAY_BUFFER, instancePositions.size() * sizeof(vec3), instancePositions.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // Posizione: location 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);

    // --- VBO per i centri ---
    glGenBuffers(1, &pair.centerVBO);
    glBindBuffer(GL_ARRAY_BUFFER, pair.centerVBO);
    glBufferData(GL_ARRAY_BUFFER, allCenters.size() * sizeof(vec3), allCenters.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(1); // Centro: location 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);

    glBindVertexArray(0); // Unbind VAO

    return pair;
}

BufferPair INIT_DEER_BUFFERS(vector<Vertex> vertices) {
    BufferPair pair;

    // VAO/VBO per la mesh
    glGenVertexArrays(1, &pair.vao);
    glGenBuffers(1, &pair.vbo);

    glBindVertexArray(pair.vao);
    glBindBuffer(GL_ARRAY_BUFFER, pair.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // layout (posizione = 0, normal = 1, texcoords = 2)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    glEnableVertexAttribArray(2);

    glVertexAttribIPointer(3, 1, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, textureIndex));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);

    return pair;
}

BufferPair INIT_SIMPLE_MODEL_BUFFERS(vector<SimpleVertex> vertices) {
    BufferPair pair;

    // VAO/VBO per la mesh
    glGenVertexArrays(1, &pair.vao);
    glGenBuffers(1, &pair.vbo);

    glBindVertexArray(pair.vao);
    glBindBuffer(GL_ARRAY_BUFFER, pair.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(SimpleVertex), vertices.data(), GL_STATIC_DRAW);

    // layout (posizione = 0, normal = 1, color = 2, boneIDs = 3, weights = 4)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)offsetof(SimpleVertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)offsetof(SimpleVertex, color));
    glEnableVertexAttribArray(2);

    glVertexAttribIPointer(3, 4, GL_INT, sizeof(SimpleVertex), (void*)offsetof(SimpleVertex, boneIDs));
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)offsetof(SimpleVertex, weights));
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);

    return pair;
}