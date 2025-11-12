#pragma once

constexpr unsigned int BOX_VERTICES_SIZE = 24 * 5 * sizeof(float); // 24 vertices, each with 5 floats (3 pos, 2 texcoord)
constexpr float BOX_VERTICES[] = {
    // Front face (z = 0.5)
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // [0]
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,  // [1]
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // [2]
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,  // [3]

    // Back face (z = -0.5)
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // [4]
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f,  // [5]
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // [6]
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,  // [7]

    // Left face (x = -0.5)
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // [8]
    -0.5f, -0.5f, 0.5f, 1.0f, 0.0f,  // [9]
    -0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // [10]
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,  // [11]

    // Right face (x = 0.5)
    0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // [12]
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,  // [13]
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // [14]
    0.5f, 0.5f, -0.5f, 0.0f, 1.0f,  // [15]

    // Top face (y = 0.5)
    -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, // [16]
    -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,  // [17]
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // [18]
    0.5f, 0.5f, -0.5f, 0.0f, 1.0f,  // [19]

    // Bottom face (y = -0.5)
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // [20]
    -0.5f, -0.5f, 0.5f, 1.0f, 0.0f,  // [21]
    0.5f, -0.5f, 0.5f, 1.0f, 1.0f,   // [22]
    0.5f, -0.5f, -0.5f, 0.0f, 1.0f   // [23]
};

constexpr unsigned int BOX_INDICES_COUNT = 36;
constexpr unsigned int BOX_INDICES[] = {
    // Front face
    0, 1, 2,
    2, 3, 0,
    // Back face
    4, 5, 6,
    6, 7, 4,
    // Left face
    8, 9, 10,
    10, 11, 8,
    // Right face
    12, 13, 14,
    14, 15, 12,
    // Top face
    16, 17, 18,
    18, 19, 16,
    // Bottom face
    20, 21, 22,
    22, 23, 20};

constexpr unsigned int PLANE_VERTICES_SIZE = 4 * 5 * sizeof(float); // 4 vertices, each with 5 floats (3 pos, 2 texcoord)
constexpr float PLANE_VERTICES[] = {
    // positions      // texture coords
    0.5f, 0.0f, 0.5f, 1.0f, 0.0f, // Front right
    -0.5f, 0.0f, 0.5f, 0.0f, 0.0f, // Front left
    -0.5f, 0.0f, -0.5f, 0.0f, 1.0f, // Back left
    0.5f, 0.0f, -0.5f, 1.0f, 1.0f}; // Back right

constexpr unsigned int PLANE_INDICES_COUNT = 6;
constexpr unsigned int PLANE_INDICES[] = {
    0, 1, 2,
    2, 3, 0};