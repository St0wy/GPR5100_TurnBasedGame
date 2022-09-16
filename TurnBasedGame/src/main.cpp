#include <spdlog/spdlog.h>

#include "Client.hpp"

int main()
{
	spdlog::set_level(spdlog::level::debug);

	Client client{};
	client.Run();
}
