#pragma once

class Model;

class ModelLibrary {
public:
	ModelLibrary();
	~ModelLibrary();

	const static uint32_t INVALID_MODEL_INDEX;

	/*********************************************************************
	 * param:  modelPath: path to the model file
	 * return: index of the model in the library
	 * 
	 * brief:  Adds a model to the library if it doesn't already exist.
	 *********************************************************************/
	uint32_t AddModel(const std::string& modelPath);

	/*********************************************************************
	 * param:  modelPath: path to the model file
	 * return: index of the model in the library
	 * 
	 * brief: Gets the index of the model in the library.
	 *********************************************************************/
	uint32_t GetModel(const std::string& modelPath);

private:
	std::vector<std::unique_ptr<Model>> models;
	std::unordered_map<std::string, uint32_t> modelMap;
};
