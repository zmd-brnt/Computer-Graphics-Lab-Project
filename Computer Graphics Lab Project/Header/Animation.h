#pragma once

#include <vector>
#include <map>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>

#include "Model.h"
#include "Bone.h"
#include "AssimpMath.h"

// Esta estructura será nuestra propia copia de la jerarquía del esqueleto de Assimp.
// La necesitamos porque no queremos mantener el objeto aiScene completo en la memoria ram.
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

	// Constructor: Carga la animación usando la ruta del archivo y un puntero a tu Modelo.
	Animation(const std::string& animationPath, Model* model)
	{
		Assimp::Importer importer;
		// Leemos el archivo. Nota: No necesitamos triangular aquí porque solo nos interesa la animación, no la malla.
		const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);

		if (!scene || !scene->mRootNode) {
			std::cout << "ERROR::ANIMATION::ASSIMP:: No se pudo cargar el archivo: " << animationPath << std::endl;
			return;
		}

		// Verificamos que el archivo realmente contenga animaciones
		if (scene->mNumAnimations == 0) {
			std::cout << "ADVERTENCIA::ANIMATION:: El archivo no contiene animaciones: " << animationPath << std::endl;
			return;
		}

		// Tomamos la primera animación del archivo (índice 0)
		auto animation = scene->mAnimations[0];

		m_Duration = animation->mDuration;
		printf("Duracion: %f\n", m_Duration);
		m_TicksPerSecond = animation->mTicksPerSecond != 0 ? animation->mTicksPerSecond : 25.0f; // 25 TPS por defecto si no lo tiene

		// Obtenemos una referencia al mapa de huesos que creamos en Model.h
		m_BoneInfoMap = model->GetBoneInfoMap();

		// Leemos la jerarquía de nodos (quién es hijo de quién)
		ReadHierarchyData(m_RootNode, scene->mRootNode);

		// Leemos los keyframes de cada hueso
		ReadMissingBones(animation, *model);
	}

	~Animation() {}

	// Busca un hueso por su nombre dentro de nuestro vector de huesos animados
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

	inline float GetTicksPerSecond() const { return m_TicksPerSecond; }
	inline float GetDuration() const { return m_Duration; }
	inline const AssimpNodeData& GetRootNode() const { return m_RootNode; }
	inline const std::map<std::string, BoneInfo>& GetBoneIDMap() const { return m_BoneInfoMap; }

private:
	void ReadMissingBones(const aiAnimation* animation, Model& model)
	{
		int size = animation->mNumChannels; // Los "channels" son los huesos animados

		// Leemos nuestro mapa de huesos (BoneInfoMap) del Modelo
		auto& boneInfoMap = model.GetBoneInfoMap();
		int& boneCount = model.GetBoneCount();

		// Iteramos sobre todos los canales de animación
		for (int i = 0; i < size; i++)
		{
			auto channel = animation->mChannels[i];
			std::string boneName = channel->mNodeName.data;

			// Si el hueso animado no existe en nuestro modelo, lo agregamos.
			if (boneInfoMap.find(boneName) == boneInfoMap.end())
			{
				boneInfoMap[boneName].id = boneCount;
				boneCount++;
			}

			// Creamos un nuevo objeto Bone (que leerá sus keyframes) y lo guardamos
			m_Bones.push_back(Bone(boneName, boneInfoMap[boneName].id, channel));
		}

		// Actualizamos nuestro mapa interno
		m_BoneInfoMap = boneInfoMap;
	}

	// Copia la jerarquía de nodos de Assimp a nuestra propia estructura recursivamente
	void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
	{
		if (!src) return;

		dest.name = src->mName.data;
		dest.transformation = AssimpMath::ConvertMatrixToGLMFormat(src->mTransformation);
		dest.childrenCount = src->mNumChildren;

		for (int i = 0; i < src->mNumChildren; i++)
		{
			AssimpNodeData newData;
			ReadHierarchyData(newData, src->mChildren[i]);
			dest.children.push_back(newData);
		}
	}

	float m_Duration;
	float m_TicksPerSecond;
	std::vector<Bone> m_Bones;
	AssimpNodeData m_RootNode;
	std::map<std::string, BoneInfo> m_BoneInfoMap;
};