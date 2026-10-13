#include <engine/core/logger/log.h>
#include <engine/platform/platform_window.h>

int main()
{
	Engine::Log::Init();
	LOG_INFO("Game Started");

	auto wnd = Engine::Window::Create();
	
	while (!wnd->ShouldClose())
	{
		wnd->PollEvents();
		wnd->SwapBuffers();
	}

	Engine::Log::Shutdown();

	return 0;
}