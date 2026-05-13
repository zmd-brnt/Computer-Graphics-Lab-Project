#pragma once

#include <GLFW/glfw3.h>

class InputManager
{
public:
	InputManager();
	~InputManager();

	// Devuelve true si la tecla solicitada está siendo presionada
	bool isKeyDown(int key) const;

	// Actualiza el estado de una tecla (se llamará desde el KeyCallback de GLFW)
	void setKey(int key, bool isPressed);

private:
	// Arreglo para almacenar el estado de las teclas de GLFW
	bool keys[1024];
}; 
