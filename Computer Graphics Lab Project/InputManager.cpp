#include "Header/InputManager.h" // Asegúrate de que la ruta coincida con tu estructura

InputManager::InputManager()
{
	// Inicializamos todas las teclas en false (no presionadas)
	for (int i = 0; i < 1024; ++i)
	{
		keys[i] = false;
	}
}

InputManager::~InputManager()
{
}

bool InputManager::isKeyDown(int key) const
{
	// Verificamos que el código de la tecla esté dentro de los límites seguros
	if (key >= 0 && key < 1024)
	{
		return keys[key];
	}
	return false;
}

void InputManager::setKey(int key, bool isPressed)
{
	// Actualizamos el estado si está dentro de los límites
	if (key >= 0 && key < 1024)
	{
		keys[key] = isPressed;
	}
}