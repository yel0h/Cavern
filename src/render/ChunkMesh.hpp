#ifndef CAVERN_CHUNKMESH_HPP
#define CAVERN_CHUNKMESH_HPP
#include <glad/glad.h>
#include <vector>

class Chunk;
class World;
class TextureAtlas;

struct Vertex
{
    float x;
    float y;
    float z;
    float u;
    float v;
    float light;
    float flags;
};

class ChunkMesh
{
private:
    unsigned int vao = 0;
    unsigned int vbo = 0;
    unsigned int ibo = 0;
    int indexCount = 0;
    unsigned int transVao = 0;
    unsigned int transVbo = 0;
    unsigned int transIbo = 0;
    int transIndexCount = 0;
    std::vector<Vertex> verts;
    std::vector<unsigned int> indices;
    std::vector<Vertex> transVerts;
    std::vector<unsigned int> transIndices;

    void addFace(float x, float y, float z, int face, float u0, float v0, float u1, float v1, float light, float flags, bool transparent);

public:
    ~ChunkMesh() { free(); }

    void build(const Chunk &chunk, const World &world);

    void upload();

    void drawOpaque() const;

    void drawTransparent() const;

    void free();
};
#endif//CAVERN_CHUNKMESH_HPP