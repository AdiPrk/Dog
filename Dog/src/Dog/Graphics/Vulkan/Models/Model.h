#pragma once

#include "Mesh.h"
#include "../Texture/TextureLibrary.h"
#include "../Animation/BoneInfo.h"

namespace Dog {

    class Texture;

    class Model {
    public:
        Model(Device& device, const std::string& filePath, TextureLibrary& textureLibrary);
        ~Model();

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        auto& GetBoneInfoMap() { return mBoneInfoMap; }
        int& GetBoneCount() { return mBoneCounter; }

        std::vector<Mesh> meshes;

    private:
        void loadMeshes(const std::string& filepath, TextureLibrary& textureLibrary);
        void processNode(aiNode* node, const aiScene* scene, TextureLibrary& textureLibrary, const std::string& filepath, const glm::mat4& parentTransform = glm::mat4(1.f));
        void processMesh(aiMesh* mesh, const aiScene* scene, TextureLibrary& textureLibrary, const std::string& filepath, const glm::mat4& transform);
        void processMaterials(aiMesh* mesh, const aiScene* scene, Mesh& newMesh, TextureLibrary& textureLibrary, const std::string& filepath);

        void SetVertexBoneDataToDefault(Vertex& vertex);
        void SetVertexBoneData(Vertex& vertex, int boneID, float weight);

        void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);

        Device& device;
        std::map<std::string, BoneInfo> mBoneInfoMap;
        int mBoneCounter = 0;
    };

} // namespace Dog
