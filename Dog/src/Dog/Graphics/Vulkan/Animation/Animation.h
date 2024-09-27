#pragma once

#include "../Models/Model.h"
#include "Bone.h"

namespace Dog {

	struct AssimpNodeData
	{
		glm::mat4 transformation;
		std::string name;
		int childrenCount;
		std::vector<AssimpNodeData> children;
	};

	class Animation
	{
	public:
		Animation() = default;

		Animation(const std::string& animationPath, Model* model)
		{
			static Assimp::Importer importer;

			// cut filepath until the last slash and remove extension
			std::string filename = animationPath.substr(animationPath.find_last_of("/\\") + 1);
			filename = filename.substr(0, filename.find_last_of("."));
			std::string assbinFilename = "assets/models/cached/" + filename + ".assbin";

			// check if the assbin file exists
			bool doesAssbinExist = true;
			{
				std::ifstream assbinFileCheck(assbinFilename);
				if (!assbinFileCheck.good()) doesAssbinExist = false;
			}
			if (!doesAssbinExist) {
				throw std::runtime_error("Assbin file does not exist, load model before animation!!!!!!");
			}

			const aiScene* scene = importer.ReadFile(animationPath, 0);

			assert(scene && scene->mRootNode);
			auto animation = scene->mAnimations[0];
			m_Duration = float(animation->mDuration);
			m_TicksPerSecond = int(animation->mTicksPerSecond); // CAREFUL, MIGHT NOT BE SET (0)
			aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
			globalTransformation = globalTransformation.Inverse();
			ReadHierarchyData(m_RootNode, scene->mRootNode);
			ReadMissingBones(animation, *model);
		}

		~Animation()
		{
		}

		Bone* FindBone(const std::string& name)
		{
			auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
				[&](const Bone& Bone)
				{
					return Bone.GetBoneName() == name;
				}
			);
			if (iter == m_Bones.end()) return nullptr;
			else return &(*iter);
		}


		inline float GetTicksPerSecond() { return float(m_TicksPerSecond); }
		inline float GetDuration() { return m_Duration; }
		inline const AssimpNodeData& GetRootNode() { return m_RootNode; }
		inline const std::map<std::string, BoneInfo>& GetBoneIDMap()
		{
			return m_BoneInfoMap;
		}

	private:
		void ReadMissingBones(const aiAnimation* animation, Model& model)
		{
			int size = animation->mNumChannels;

			auto& boneInfoMap = model.GetBoneInfoMap();//getting m_BoneInfoMap from Model class
			int& boneCount = model.GetBoneCount(); //getting the m_BoneCounter from Model class

			//reading channels(bones engaged in an animation and their keyframes)
			for (int i = 0; i < size; i++)
			{
				auto channel = animation->mChannels[i];
				std::string boneName = channel->mNodeName.data;

				if (boneInfoMap.find(boneName) == boneInfoMap.end())
				{
					boneInfoMap[boneName].id = boneCount;
					boneCount++;
				}
				m_Bones.push_back(Bone(channel->mNodeName.data,
					boneInfoMap[channel->mNodeName.data].id, channel));
			}

			m_BoneInfoMap = boneInfoMap;
		}

		void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
		{
			assert(src);

			dest.name = src->mName.data;
			dest.transformation = aiMatToGlm(src->mTransformation);
			dest.childrenCount = src->mNumChildren;

			// log dest.name
			printf("%s\n", dest.name.c_str());

			for (unsigned i = 0; i < src->mNumChildren; i++)
			{
				AssimpNodeData newData;
				ReadHierarchyData(newData, src->mChildren[i]);
				dest.children.push_back(newData);
			}
		}
		float m_Duration;
		int m_TicksPerSecond;
		std::vector<Bone> m_Bones;
		AssimpNodeData m_RootNode;
		std::map<std::string, BoneInfo> m_BoneInfoMap;
	};

} // namespace Dog