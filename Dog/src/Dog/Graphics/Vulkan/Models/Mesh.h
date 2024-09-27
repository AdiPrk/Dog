#pragma once

#include "../Buffers/Buffer.h"
#include "../Core/Device.h"

namespace Dog {

    struct Material {
        uint32_t diffuseTextures;
        uint32_t specularTextures;
        uint32_t normalMaps;
        uint32_t heightMaps;
        // Add more texture types here later

        float shininess = 32.0f; // Default value
    };

    struct Vertex {
        glm::vec3 position{};
        glm::vec3 color{};
        glm::vec3 normal{};
        glm::vec2 uv{};

        int mBoneIDs[MAX_BONE_INFLUENCE];
        float mWeights[MAX_BONE_INFLUENCE];

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

        bool operator==(const Vertex& other) const {
            return position == other.position && color == other.color && normal == other.normal &&
                uv == other.uv;
        }
    };

    class Mesh {
    public:
        Mesh() = default;

        void createVertexBuffers(Device& device);
        void createIndexBuffers(Device& device);
        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

        std::unique_ptr<Buffer> vertexBuffer;
        uint32_t vertexCount = 0;

        bool hasIndexBuffer = false;
        std::unique_ptr<Buffer> indexBuffer;
        uint32_t indexCount = 0;

        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};

        uint32_t textureIndex = 999;
        Material material{};
    };

} // namespace Dog