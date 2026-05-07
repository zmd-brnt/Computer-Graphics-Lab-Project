#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include "Animation.h"
#include "Bone.h"

class Animator
{
public:
	// Constructor: recibe la animación inicial y prepara el vector de matrices
	Animator(Animation* currentAnimation)
	{
		m_CurrentTime = 0.0f;
		m_CurrentAnimation = currentAnimation;

		// Reservamos espacio para 100 huesos (el máximo que pusimos en el Shader)
		m_FinalBoneMatrices.reserve(100);
		for (int i = 0; i < 100; i++)
			m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
	}

	// Se llama en CADA FRAME dentro de tu ciclo while(!glfwWindowShouldClose)
	void UpdateAnimation(float dt)
	{
		m_DeltaTime = dt;
		if (m_CurrentAnimation)
		{
			// 1. Avanzamos el tiempo de la animación basándonos en la velocidad (TicksPerSecond)
			m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;

			// 2. Usamos fmod para que cuando la animación termine, vuelva a empezar (Loop)
			m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());

			// 3. Empezamos a calcular las posiciones desde el nodo "Raíz" (Root) del esqueleto
			CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
		}
	}

	// Útil si después quieres cambiar entre caminar, correr, saltar, etc.
	void PlayAnimation(Animation* pAnimation)
	{
		m_CurrentAnimation = pAnimation;
		m_CurrentTime = 0.0f;
	}

	// Esta es la función clave: Calcula la jerarquía de los huesos (Padre afecta a Hijo) recursivamente
	void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
	{
		std::string nodeName = node->name;
		glm::mat4 nodeTransform = node->transformation;

		// Buscamos si este nodo es un hueso animado
		Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

		if (Bone)
		{
			// Le decimos al hueso: "Actualízate al segundo actual de la animación"
			Bone->Update(m_CurrentTime);
			nodeTransform = Bone->GetLocalTransform();
		}

		// Multiplicamos el movimiento local por el movimiento del hueso "Padre"
		glm::mat4 globalTransformation = parentTransform * nodeTransform;

		// Si el nodo actual es un hueso que afecta la malla, guardamos su matriz final
		auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
		if (boneInfoMap.find(nodeName) != boneInfoMap.end())
		{
			int index = boneInfoMap[nodeName].id;
			glm::mat4 offset = boneInfoMap[nodeName].offset;

			// Matriz Global * Matriz Offset (Mueve el vértice del espacio del modelo al espacio del hueso)
			m_FinalBoneMatrices[index] = globalTransformation * offset;
		}

		// Repetimos el proceso para todos los "hijos" de este hueso (ej. del Hombro pasamos al Codo)
		for (int i = 0; i < node->childrenCount; i++)
			CalculateBoneTransform(&node->children[i], globalTransformation);
	}

	std::vector<glm::mat4> GetFinalBoneMatrices()
	{
		return m_FinalBoneMatrices;
	}

private:
	std::vector<glm::mat4> m_FinalBoneMatrices;
	Animation* m_CurrentAnimation;
	float m_CurrentTime;
	float m_DeltaTime;
};