#pragma once

constexpr unsigned int BOX_VERTICES_TEX_SIZE = 24 * 5 * sizeof(float); // 24 vertices, each with 5 floats (3 pos, 2 texcoord)
constexpr float BOX_VERTICES_TEX[] = {
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

constexpr unsigned int BOX_INDICES_TEX_COUNT = 36;
constexpr unsigned int BOX_INDICES_TEX[] = {
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

constexpr unsigned int BOX_VERTICES_SIZE = 8 * 3 * sizeof(float); // 8 vertices, each with 3 floats (3 pos)
constexpr float BOX_VERTICES[] = {
    // Front vertices (z = 0.5)
    -0.5f, -0.5f, 0.5f, // [0]
    0.5f, -0.5f, 0.5f, // [1]
    0.5f, 0.5f, 0.5f, // [2]
    -0.5f, 0.5f, 0.5f, // [3]
    // Back vertices (z = -0.5)
    -0.5f, -0.5f, -0.5f, // [4]
    0.5f, -0.5f, -0.5f, // [5]
    0.5f, 0.5f, -0.5f, // [6]
    -0.5f, 0.5f, -0.5f, // [7]
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
    0, 3, 7,
    7, 4, 0,
    // Right face
    1, 5, 6,
    6, 2, 1,
    // Top face
    3, 2, 6,
    6, 7, 3,
    // Bottom face
    0, 4, 5,
    5, 1, 0};

constexpr unsigned int BOX_VERTICES_NORM_TEX_SIZE = 6 * 6 * 8 * sizeof(float); // 6 vertices per side, 6 sides , each with 8 floats (3 pos, 3 norm, 2 texcoord)
constexpr float BOX_VERTICES_NORM_TEX[] = {
    //  positions         // normals         // texture coords
    //  (x, y, z)         (nx, ny, nz)      (u, v)

    // Front face (Z = -0.5f)
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
    0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,  // Bottom-right
    0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // Top-right
    0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // Top-right (duplicate for second triangle)
    -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,  // Top-left
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left (duplicate for second triangle)

    // Back face (Z = 0.5f)
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // Bottom-left
    0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // Bottom-right
    0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // Top-right
    0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // Top-right (duplicate)
    -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // Top-left
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // Bottom-left (duplicate)

    // Left face (X = -0.5f)
    -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // Top-front (mapped to 1,0)
    -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // Top-back (mapped to 0,0)
    -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // Bottom-back (mapped to 0,1)
    -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // Bottom-back (duplicate)
    -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // Bottom-front (mapped to 1,1)
    -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // Top-front (duplicate)

    // Right face (X = 0.5f)
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,   // Top-front (mapped to 0,0)
    0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // Top-back (mapped to 1,0)
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // Bottom-back (mapped to 1,1)
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // Bottom-back (duplicate)
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // Bottom-front (mapped to 0,1)
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,   // Top-front (duplicate)

    // Bottom face (Y = -0.5f)
    -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // Back-left (mapped to 0,1)
    0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,  // Back-right (mapped to 1,1)
    0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // Front-right (mapped to 1,0)
    0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // Front-right (duplicate)
    -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // Front-left (mapped to 0,0)
    -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // Back-left (duplicate)

    // Top face (Y = 0.5f)
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // Back-left (mapped to 0,1)
    0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // Back-right (mapped to 1,1)
    0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // Front-right (mapped to 1,0)
    0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // Front-right (duplicate)
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,  // Front-left (mapped to 0,0)
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f  // Back-left (duplicate)
};

constexpr unsigned int PLANE_VERTICES_TEX_SIZE = 4 * 5 * sizeof(float); // 4 vertices, each with 5 floats (3 pos, 2 texcoord)
constexpr float PLANE_VERTICES_TEX[] = {
    // positions      // texture coords
    0.5f, 0.0f, 0.5f, 1.0f, 0.0f,   // Front right
    -0.5f, 0.0f, 0.5f, 0.0f, 0.0f,  // Front left
    -0.5f, 0.0f, -0.5f, 0.0f, 1.0f, // Back left
    0.5f, 0.0f, -0.5f, 1.0f, 1.0f}; // Back right

constexpr unsigned int PLANE_INDICES_COUNT = 6;
constexpr unsigned int PLANE_INDICES[] = {
    0, 1, 2,
    2, 3, 0};