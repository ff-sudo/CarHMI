#include "mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "stb_image.h"

#include <spdlog/spdlog.h>
#include <unordered_map>
#include <algorithm>

namespace CarHMI {

Mesh::~Mesh() {
    for (auto& sm : m_subMeshes) {
        if (sm.vao) glDeleteVertexArrays(1, &sm.vao);
        if (sm.vbo) glDeleteBuffers(1, &sm.vbo);
        if (sm.ebo) glDeleteBuffers(1, &sm.ebo);
    }
    for (auto& mat : m_materials) {
        if (mat.diffuseTexture) glDeleteTextures(1, &mat.diffuseTexture);
    }
}

static GLuint LoadTextureFromFile(const std::string& path) {
    int w, h, ch;
    stbi_set_flip_vertically_on_load(1);
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, 0);
    if (!data) {
        spdlog::warn("Mesh: cannot load texture {}", path);
        return 0;
    }

    GLenum format = (ch == 4) ? GL_RGBA : (ch == 3) ? GL_RGB : GL_RED;
    GLenum internalFormat = (ch == 4) ? GL_RGBA8 : (ch == 3) ? GL_RGB8 : GL_R8;

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    spdlog::info("Mesh: loaded texture {} ({}x{}, {} ch)", path, w, h, ch);
    return tex;
}

bool Mesh::LoadOBJ(const std::string& path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::string dir = path.substr(0, path.find_last_of("/\\") + 1);

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                                 path.c_str(), dir.c_str());

    if (!warn.empty()) spdlog::warn("OBJ warn: {}", warn);
    if (!err.empty()) spdlog::error("OBJ error: {}", err);
    if (!ret) return false;

    spdlog::info("OBJ loaded: {} shapes, {} materials, {} vertices",
                 shapes.size(), materials.size(), attrib.vertices.size() / 3);

    // Load materials
    for (auto& mat : materials) {
        Material3D m;
        m.name = mat.name;
        m.ambient  = {mat.ambient[0], mat.ambient[1], mat.ambient[2]};
        m.diffuse  = {mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]};
        m.specular = {mat.specular[0], mat.specular[1], mat.specular[2]};
        m.shininess = mat.shininess;

        if (!mat.diffuse_texname.empty()) {
            std::string texPath = dir + mat.diffuse_texname;
            m.diffuseTexture = LoadTextureFromFile(texPath);
            m.hasDiffuseTexture = (m.diffuseTexture != 0);
        }

        m_materials.push_back(m);
    }

    // Default material if none
    if (m_materials.empty()) {
        Material3D def;
        def.name = "default";
        m_materials.push_back(def);
    }

    // Build meshes per shape
    std::vector<Vertex3D> allVertices;

    for (auto& shape : shapes) {
        std::vector<Vertex3D> vertices;
        std::vector<uint32_t> indices;
        std::unordered_map<std::string, uint32_t> uniqueVerts;

        int matId = -1;
        if (!shape.mesh.material_ids.empty())
            matId = shape.mesh.material_ids[0];

        size_t indexOffset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            int fv = shape.mesh.num_face_vertices[f];

            for (int v = 0; v < fv; v++) {
                tinyobj::index_t idx = shape.mesh.indices[indexOffset + v];

                Vertex3D vert{};
                vert.position = {
                    attrib.vertices[3 * idx.vertex_index + 0],
                    attrib.vertices[3 * idx.vertex_index + 1],
                    attrib.vertices[3 * idx.vertex_index + 2]
                };

                if (idx.normal_index >= 0) {
                    vert.normal = {
                        attrib.normals[3 * idx.normal_index + 0],
                        attrib.normals[3 * idx.normal_index + 1],
                        attrib.normals[3 * idx.normal_index + 2]
                    };
                }

                if (idx.texcoord_index >= 0) {
                    vert.texCoord = {
                        attrib.texcoords[2 * idx.texcoord_index + 0],
                        attrib.texcoords[2 * idx.texcoord_index + 1]
                    };
                }

                // Dedup vertices
                std::string key = std::to_string(idx.vertex_index) + "/" +
                                  std::to_string(idx.normal_index) + "/" +
                                  std::to_string(idx.texcoord_index);

                auto it = uniqueVerts.find(key);
                if (it != uniqueVerts.end()) {
                    indices.push_back(it->second);
                } else {
                    uint32_t newIdx = (uint32_t)vertices.size();
                    uniqueVerts[key] = newIdx;
                    vertices.push_back(vert);
                    indices.push_back(newIdx);
                }
            }
            indexOffset += fv;
        }

        if (vertices.empty()) continue;

        // Create GPU buffers
        SubMesh sm;
        sm.indexCount = (int)indices.size();
        sm.materialIndex = matId >= 0 ? matId : 0;

        glGenVertexArrays(1, &sm.vao);
        glBindVertexArray(sm.vao);

        glGenBuffers(1, &sm.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, sm.vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex3D), vertices.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &sm.ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sm.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

        // position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, position));
        // normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, normal));
        // texcoord
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, texCoord));

        glBindVertexArray(0);

        m_subMeshes.push_back(sm);
        allVertices.insert(allVertices.end(), vertices.begin(), vertices.end());

        spdlog::info("  shape '{}': {} verts, {} indices, mat={}",
                     shape.name, vertices.size(), indices.size(), matId);
    }

    CalculateBounds(allVertices);
    return true;
}

void Mesh::Draw(GLuint shaderProgram) const {
    for (auto& sm : m_subMeshes) {
        int matIdx = sm.materialIndex;
        if (matIdx >= 0 && matIdx < (int)m_materials.size()) {
            auto& mat = m_materials[matIdx];
            glUniform3fv(glGetUniformLocation(shaderProgram, "u_Ambient"), 1, &mat.ambient[0]);
            glUniform3fv(glGetUniformLocation(shaderProgram, "u_Diffuse"), 1, &mat.diffuse[0]);
            glUniform3fv(glGetUniformLocation(shaderProgram, "u_Specular"), 1, &mat.specular[0]);
            glUniform1f(glGetUniformLocation(shaderProgram, "u_Shininess"), mat.shininess);
            glUniform1i(glGetUniformLocation(shaderProgram, "u_HasDiffuseMap"), mat.hasDiffuseTexture ? 1 : 0);

            if (mat.hasDiffuseTexture) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, mat.diffuseTexture);
                glUniform1i(glGetUniformLocation(shaderProgram, "u_DiffuseMap"), 0);
            }
        }

        glBindVertexArray(sm.vao);
        glDrawElements(GL_TRIANGLES, sm.indexCount, GL_UNSIGNED_INT, nullptr);
    }
    glBindVertexArray(0);
}

void Mesh::CalculateBounds(const std::vector<Vertex3D>& vertices) {
    if (vertices.empty()) return;

    glm::vec3 minP = vertices[0].position;
    glm::vec3 maxP = vertices[0].position;

    for (auto& v : vertices) {
        minP = glm::min(minP, v.position);
        maxP = glm::max(maxP, v.position);
    }

    m_center = (minP + maxP) * 0.5f;
    m_boundingRadius = glm::length(maxP - minP) * 0.5f;

    spdlog::info("Mesh bounds: center=({:.1f},{:.1f},{:.1f}), radius={:.1f}",
                 m_center.x, m_center.y, m_center.z, m_boundingRadius);
}

} // namespace CarHMI
