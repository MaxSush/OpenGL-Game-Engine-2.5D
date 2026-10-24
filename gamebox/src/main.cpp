#include <PlatformerGame.h>


int main()
{
	Engine::AppConfig config;
	config.window.title = "Shadow Runner";
	config.window.vsync = false;

	PlatformerGame game;

	return Engine::Run(game, config);
}