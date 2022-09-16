#include <spdlog/spdlog.h>

#include "Server.hpp"

int main()
{
	spdlog::set_level(spdlog::level::debug);

	stw::Server server;
	server.Run();
}
