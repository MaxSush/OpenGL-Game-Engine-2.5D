#include <core/logger/logger.h>
#include <platform/platform_window.h>

int main()
{
	Logger::Initialize();
	LOG_INFO("Game Started");

	auto wnd = Engine::Window::Create();
	
	while (!wnd->ShouldClose())
	{
		wnd->PollEvents();
		wnd->SwapBuffers();
	}

	return 0;
}