#include <engine/application.h>

#include "PlatformerGame.h"


int main()
{
	Engine::AppConfig config;
	config.window.title = "Shadow Runner";
	config.window.vsync = true;

	//config.fixed_step = 1.0f / 20.0f;

	PlatformerGame game;

	return Engine::Run(game, config);
}