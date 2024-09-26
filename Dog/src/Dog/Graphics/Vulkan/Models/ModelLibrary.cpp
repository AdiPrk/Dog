#include <PCH/pch.h>
#include "ModelLibrary.h"
#include "Model.h"

const uint32_t ModelLibrary::INVALID_MODEL_INDEX = 9999;

ModelLibrary::ModelLibrary()
{
}

ModelLibrary::~ModelLibrary()
{
}

uint32_t ModelLibrary::AddModel(const std::string& modelPath)
{
	/*if (models.size() >= MAX_MODEL_COUNT) {
		throw std::runtime_error("Model count exceeded maximum");
		return INVALID_MODEL_INDEX;
	}

	if (modelMap.find(modelPath) == modelMap.end()) {
		modelMap[modelPath] = static_cast<uint32_t>(models.size());
		models.push_back(std::make_unique<Model>(modelPath));
		return static_cast<uint32_t>(models.size() - 1);
	}
	else {
		return INVALID_MODEL_INDEX;
	}*/
	return 0;
}

uint32_t ModelLibrary::GetModel(const std::string& modelPath)
{
	if (modelMap.find(modelPath) != modelMap.end()) {
		return modelMap[modelPath];
	}
	else {
		return INVALID_MODEL_INDEX;
	}
}
