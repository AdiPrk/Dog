#pragma once

struct BoneInfo
{
	/* index in finalBoneMatrices */
	int id;

	/* offset matrix transforms vertex from model space to bone space */
	glm::mat4 offset;
};
