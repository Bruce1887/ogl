#pragma once

#include "Common.h"

#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h" 

class Renderer
{
public:
    static void clear();
    static void draw(const VertexArray &va, const IndexBuffer &ib, const Shader &shader);

};
