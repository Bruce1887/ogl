#pragma once

class IndexBuffer
{
private:
    unsigned int m_RendererID; // Unique ID for the buffer
    unsigned int m_Count; // Number of indices
public:
    IndexBuffer(const unsigned int *data, unsigned int count);
    ~IndexBuffer();

    void Bind() const;
    void Unbind() const;
    inline unsigned int GetCount() const { return m_Count; }
};