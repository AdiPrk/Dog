#pragma once

#include "../Buffers/Buffer.h"
#include "../Core/Device.h"

namespace Dog {

    struct MaterialComponent;

    struct Vertex {
        glm::vec3 position{};
        glm::vec3 color{};
        glm::vec3 normal{};
        glm::vec2 uv{};

        int mBoneIDs[MAX_BONE_INFLUENCE];
        float mWeights[MAX_BONE_INFLUENCE];

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
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

        uint32_t textureIndex = INVALID_TEXTURE_INDEX;
        
        // MaterialComponent materialComponent{};
    };

} // namespace Dog