#include <PCH/pch.h>
#include "Model.h"
#include "../Texture/Texture.h"

#include "assimp/Exporter.hpp"

namespace Dog {

    Model::Model(Device& device, const std::string& filePath, TextureLibrary& textureLibrary)
        : device{ device }
    {
        loadMeshes(filePath, textureLibrary);

        for (Mesh& mesh : meshes) {
            mesh.createVertexBuffers(device);
            mesh.createIndexBuffers(device);
        }
    }

    Model::~Model() {}

    std::string aiTexturePathToNLEPath(const aiString& texturePath, const std::string& directory) {
        std::string textureFilepath = texturePath.C_Str();

        textureFilepath.erase(0, textureFilepath.find_first_not_of("./\\"));

        std::string textureFullpath;

        // if texture filepath doesn't start with textures/ or directoroy doesn't end with textures
        if (textureFilepath.find("textures") == std::string::npos && directory.find("textures") == std::string::npos) {
            textureFullpath = directory + "/textures/" + textureFilepath;
        }
        else {
            textureFullpath = directory + "/" + textureFilepath;
        }

        return textureFullpath;
    }

    const char* getTextureTypeString(aiTextureType type) {
        switch (type) {
        case aiTextureType_NONE: return "None";
        case aiTextureType_DIFFUSE: return "Diffuse";
        case aiTextureType_SPECULAR: return "Specular";
        case aiTextureType_AMBIENT: return "Ambient";
        case aiTextureType_EMISSIVE: return "Emissive";
        case aiTextureType_HEIGHT: return "Height";
        case aiTextureType_NORMALS: return "Normals";
        case aiTextureType_SHININESS: return "Shininess";
        case aiTextureType_OPACITY: return "Opacity";
        case aiTextureType_DISPLACEMENT: return "Displacement";
        case aiTextureType_LIGHTMAP: return "Lightmap";
        case aiTextureType_REFLECTION: return "Reflection";
        case aiTextureType_BASE_COLOR: return "Base Color";
        case aiTextureType_NORMAL_CAMERA: return "Normal Camera";
        case aiTextureType_EMISSION_COLOR: return "Emission Color";
        case aiTextureType_METALNESS: return "Metalness";
        case aiTextureType_DIFFUSE_ROUGHNESS: return "Diffuse Roughness";
        case aiTextureType_AMBIENT_OCCLUSION: return "Ambient Occlusion";
        case aiTextureType_SHEEN: return "Sheen";
        case aiTextureType_CLEARCOAT: return "Clearcoat";
        case aiTextureType_TRANSMISSION: return "Transmission";
        case aiTextureType_UNKNOWN: return "Unknown";
        default: return "Other";
        }
    }

    void LogTextures(aiMaterial* material) {
        std::vector<aiTextureType> textureTypes = {
            aiTextureType_DIFFUSE,
            aiTextureType_SPECULAR,
            aiTextureType_AMBIENT,
            aiTextureType_EMISSIVE,
            aiTextureType_HEIGHT,
            aiTextureType_NORMALS,
            aiTextureType_SHININESS,
            aiTextureType_OPACITY,
            aiTextureType_DISPLACEMENT,
            aiTextureType_LIGHTMAP,
            aiTextureType_REFLECTION,
            aiTextureType_BASE_COLOR,
            aiTextureType_NORMAL_CAMERA,
            aiTextureType_EMISSION_COLOR,
            aiTextureType_METALNESS,
            aiTextureType_DIFFUSE_ROUGHNESS,
            aiTextureType_AMBIENT_OCCLUSION,
            aiTextureType_SHEEN,
            aiTextureType_CLEARCOAT,
            aiTextureType_TRANSMISSION,
            aiTextureType_UNKNOWN
        };

        // Iterate over each texture type and log the textures present
        for (aiTextureType textureType : textureTypes) {
            unsigned int textureCount = material->GetTextureCount(textureType);
            if (textureCount > 0) {
                std::cout << "-  Material has " << textureCount << " texture(s) of type " << getTextureTypeString(textureType) << std::endl;
                for (unsigned int i = 0; i < textureCount; ++i) {
                    aiString texturePath;
                    if (material->GetTexture(textureType, i, &texturePath) == AI_SUCCESS) {
                        std::string textureFilepath = texturePath.C_Str();
                        std::cout << "     Texture " << i << ": " << textureFilepath << std::endl;
                    }
                }
            }
        }
    }

    // aiProcess_Triangulate              // Ensure all faces are triangles
    // | aiProcess_JoinIdenticalVertices  // Combine identical vertices
    // | aiProcess_GenNormals             // Generate normals if they don't exist
    // | aiProcess_LimitBoneWeights       // Limit bone influences (relevant for animated FBX)
    // | aiProcess_OptimizeGraph		   // Optimize the scene graph (Bad if we want a cool editor doing stuff, good for fast rendering)
    // | aiProcess_OptimizeMeshes         // Optimize the meshes for better performance
    // | aiProcess_SplitLargeMeshes       // Split large meshes into smaller submeshes
    // | aiProcess_RemoveRedundantMaterials // Remove redundant materials (be careful)
    // | aiProcess_ImproveCacheLocality   // Improve GPU cache performance

    void Model::loadMeshes(const std::string& filepath, TextureLibrary& textureLibrary) {
        // Making an Importer is supposedly expensive, so I made it static
        static Assimp::Importer importer;

        // Log the file being loaded
        std::cout << "Loading model: " << filepath << std::endl;

        try {
            // Not so sure about these flags
            // Maybe we can pre-load models and store them somewhere
            // so that in the real game it doesn't have to read the model slowly?
            unsigned int processFlags = aiProcessPreset_TargetRealtime_MaxQuality; // slowest to load, but highest quality

            // set global scale
            importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.f);
            processFlags |= aiProcess_GlobalScale;

            // cut filepath until the last slash and remove extension
            std::string filename = filepath.substr(filepath.find_last_of("/\\") + 1);
            filename = filename.substr(0, filename.find_last_of("."));
            std::string assbinFilename = "assets/models/cached/" + filename + ".assbin";

            // check if the assbin file exists
            bool doesAssbinExist = true;
            {
                std::ifstream assbinFileCheck(assbinFilename);
                if (!assbinFileCheck.good()) doesAssbinExist = false;
            }

            const aiScene* scene;
            if (doesAssbinExist) {
                scene = importer.ReadFile(assbinFilename, 0);
            }
            else {
                scene = importer.ReadFile(filepath, processFlags);
                if (scene) {
                    aiReturn result = aiExportScene(scene, "assbin", assbinFilename.c_str(), 0);
                }
            }

            // Check if the scene was loaded successfully
            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                std::cerr << "Error loading model: " << importer.GetErrorString() << std::endl;
                throw std::runtime_error("Failed to load model: " + std::string(importer.GetErrorString()));
            }

            meshes.clear();

            // Start recursive loading all the meshes
            //glm::mat4 globalTransform = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f));  // Start with identity matrix
            //processNode(scene->mRootNode, scene, textureLibrary, filepath, globalTransform);
            processNode(scene->mRootNode, scene, textureLibrary, filepath);
        }
        catch (const std::exception& e) {
            std::cerr << "Exception occurred while loading model: " << e.what() << std::endl;
            throw;  // Rethrow the exception after logging
        }
    }

    // Recursive function to process a node and its children
    void Model::processNode(aiNode* node, const aiScene* scene, TextureLibrary& textureLibrary, const std::string& filepath, const glm::mat4& parentTransform) {
        // Convert the node's transformation matrix
        glm::mat4 nodeTransform = aiMatToGlm(node->mTransformation);

        // Combine this node's transformation with the parent transformation
        glm::mat4 globalTransform = parentTransform * nodeTransform;

        // Process each mesh in the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            processMesh(mesh, scene, textureLibrary, filepath, globalTransform);
        }

        // Recursively process each child node
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene, textureLibrary, filepath, globalTransform);
        }
    }

    // Process a single mesh and extract vertices, indices, and materials
    void Model::processMesh(aiMesh* mesh, const aiScene* scene, TextureLibrary& textureLibrary, const std::string& filepath, const glm::mat4& transform) {
        Mesh& newMesh = meshes.emplace_back();
        newMesh.vertices.clear();
        newMesh.indices.clear();

        // printf("-  New Mesh\n");

        // Extract vertex data
        for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
            Vertex vertex{};
            SetVertexBoneDataToDefault(vertex);

            glm::vec4 position = {
                mesh->mVertices[j].x,
                mesh->mVertices[j].y,
                mesh->mVertices[j].z,
                1.0f
            };
            //position = transform * position;

            vertex.position = glm::vec3(position.x, position.y, position.z);

            // Normals
            if (mesh->HasNormals()) {
                glm::vec4 normal(
                    mesh->mNormals[j].x,
                    mesh->mNormals[j].y,
                    mesh->mNormals[j].z,
                    0.0f  // Normals don't use the translation part of the matrix, so w = 0
                );
                //normal = transform * normal;
                vertex.normal = glm::normalize(glm::vec3(normal));
            }
            else {
                vertex.normal = glm::vec3(0.0f);
            }

            // UV Coordinates
            if (mesh->HasTextureCoords(0)) {
                vertex.uv = {
                    mesh->mTextureCoords[0][j].x,
                    mesh->mTextureCoords[0][j].y
                };
            }
            else {
                vertex.uv = glm::vec2(0.0f);
            }

            // Colors
            if (mesh->HasVertexColors(0)) {
                vertex.color = {
                    mesh->mColors[0][j].r,
                    mesh->mColors[0][j].g,
                    mesh->mColors[0][j].b
                };
            }
            else {
                vertex.color = glm::vec3(1.0f); // Default white color
            }

            newMesh.vertices.push_back(vertex);
        }

        // Process materials and textures
        processMaterials(mesh, scene, newMesh, textureLibrary, filepath);

        ExtractBoneWeightForVertices(newMesh.vertices, mesh, scene);

        // Extract indices from faces
        for (unsigned int k = 0; k < mesh->mNumFaces; k++) {
            const aiFace& face = mesh->mFaces[k];
            for (unsigned int l = 0; l < face.mNumIndices; l++) {
                newMesh.indices.push_back(face.mIndices[l]);
            }
        }
    }

    // process materials
    void Model::processMaterials(aiMesh* mesh, const aiScene* scene, Mesh& newMesh, TextureLibrary& textureLibrary, const std::string& filepath) {
        // Loop through materials (textures)
        if (scene->HasMaterials()) {
            // auto& newMaterial = newMesh.material;
            
            std::string directory = filepath.substr(0, filepath.find_last_of('/'));

            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            // log texture type of all materials
            // LogTextures(material);

            aiString texturePath;

            // Needs to be modified with
            // scene->GetEmbeddedTexture
            // probably to be more consistent with different file formats
            // the current one works with glb though (only one that's checked for embedded textures)

            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {

                if (texturePath.data[0] == '*') {
                    int textureIndex = atoi(texturePath.C_Str() + 1);

                    if (textureIndex >= 0 && textureIndex < int(scene->mNumTextures)) {
                        aiTexture* embeddedTexture = scene->mTextures[textureIndex];

                        if (embeddedTexture->mHeight == 0) {
                            unsigned char* textureData = reinterpret_cast<unsigned char*>(embeddedTexture->pcData);
                            int textureSize = embeddedTexture->mWidth;

                            newMesh.textureIndex = textureLibrary.AddTextureFromMemory(textureData, textureSize);
                        }
                    }

                    // std::cout << ">  Loaded DIFFUSE texture from memory successfully!" << std::endl;
                }
                else {
                    std::string textureFullpath = aiTexturePathToNLEPath(texturePath, directory);

                    textureLibrary.AddTexture(textureFullpath);
                    newMesh.textureIndex = textureLibrary.GetTexture(textureFullpath);

                    // log loaded texture from which model
                    // std::cout << ">  Loaded DIFFUSE " << textureFullpath << " successfully!" << std::endl;
                }
            }
        }
    }

    void Model::SetVertexBoneDataToDefault(Vertex& vertex)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            vertex.mBoneIDs[i] = -1;
            vertex.mWeights[i] = 0.0f;
        }
    }

    void Model::SetVertexBoneData(Vertex& vertex, int boneID, float weight)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
        {
            if (vertex.mBoneIDs[i] < 0)
            {
                vertex.mWeights[i] = weight;
                vertex.mBoneIDs[i] = boneID;
                break;
            }
        }
    }

    void Model::ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
    {
        auto& boneInfoMap = mBoneInfoMap;
        int& boneCount = mBoneCounter;

        for (unsigned boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            int boneID = -1;
            std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
            if (boneInfoMap.find(boneName) == boneInfoMap.end())
            {
                BoneInfo newBoneInfo;
                newBoneInfo.id = boneCount;
                newBoneInfo.offset = aiMatToGlm(mesh->mBones[boneIndex]->mOffsetMatrix);
                boneInfoMap[boneName] = newBoneInfo;
                boneID = boneCount;
                boneCount++;
            }
            else
            {
                boneID = boneInfoMap[boneName].id;
            }
            assert(boneID != -1);
            auto weights = mesh->mBones[boneIndex]->mWeights;
            int numWeights = mesh->mBones[boneIndex]->mNumWeights;

            for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
            {
                int vertexId = weights[weightIndex].mVertexId;
                float weight = weights[weightIndex].mWeight;
                assert(vertexId <= vertices.size());
                SetVertexBoneData(vertices[vertexId], boneID, weight);
            }
        }
    }

} // namespace Dog
