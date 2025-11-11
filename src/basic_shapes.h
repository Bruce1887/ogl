#pragma once

constexpr float box_vertices[] = {
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

constexpr unsigned int box_indices[] = {
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

// Plane lying on the XZ plane, centered at origin, size 1x1
constexpr float plane_vertices[] = {
    // positions      // texture coords
    0.5f, 0.0f, 0.5f, 1.0f, 0.0f, // Front right
    -0.5f, 0.0f, 0.5f, 0.0f, 0.0f, // Front left
    -0.5f, 0.0f, -0.5f, 0.0f, 1.0f, // Back left
    0.5f, 0.0f, -0.5f, 1.0f, 1.0f}; // Back right

constexpr unsigned int plane_indices[] = {
    0, 1, 2,
    2, 3, 0};