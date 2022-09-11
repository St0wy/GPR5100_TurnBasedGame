#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include "SFML/Network/IpAddress.hpp"
#include <iostream>
#include <spdlog/spdlog.h>

#include "MorpionGrid.hpp"

#include "SFML/Network/Packet.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/TcpSocket.hpp"


bool RenderLoginWindow(char* ipInputBuffer, char* portInputBuffer, sf::IpAddress& ip, unsigned short& port
)
{
	bool show = true;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);

	ImGui::Begin("Login", &show, stw::WINDOW_FULLSCREEN_FLAGS);
	ImGui::TextWrapped("Please type your name, ip of the server and its ports.");
	// ReSharper disable once CppTooWideScope
	ImGui::InputTextWithHint("##ip", "IP", ipInputBuffer, stw::IP_BUFFER_SIZE);
	ImGui::InputTextWithHint("##port", "Port", portInputBuffer, stw::PORT_BUFFER_SIZE);
	bool isConnecting = false;
	if (ImGui::Button("Login"))
	{
		const std::string inputIp(ipInputBuffer);
		const unsigned short inputPort = static_cast<unsigned short>(std::stoi(portInputBuffer));
		if (inputPort && !inputIp.empty())
		{
			ip = inputIp;
			port = inputPort;
			isConnecting = true;
		}
	}
	ImGui::End();

	return isConnecting;
}


int main()
{
	spdlog::set_level(spdlog::level::debug);

	sf::RenderWindow window(sf::VideoMode(640 * 2, 480 * 2), "Online Morpion");
	if (!ImGui::SFML::Init(window))
	{
		std::cout << "Error loading ImGui" << std::endl;
		return EXIT_FAILURE;
	}

	// Set window scale for HDPI screens
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 2;

	// Create socket
	sf::TcpSocket socket;
	socket.setBlocking(false);

	// Session var
	stw::MorpionGrid grid{};
	grid.setPosition(30.0f, 30.0f);
	bool oldMousePressed = false;

	char portInputBuffer[stw::PORT_BUFFER_SIZE] = "8008";
	char ipInputBuffer[stw::IP_BUFFER_SIZE] = "localhost";

	sf::IpAddress ip;
	unsigned short port;
	bool isConnected = false;
	bool isConnecting = false;

	sf::Packet sendingPacket;
	sf::Packet receivePacket;

	sf::Clock deltaClock;
	while (window.isOpen())
	{
		sf::Event event{};
		while (window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(window, event);

			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
			else if (event.type == sf::Event::Resized)
			{
				sf::Vector2f size = { static_cast<float>(event.size.width), static_cast<float>(event.size.height) };
				sf::Vector2f center = size / 2.0f;
				window.setView(sf::View(center, size));
			}
		}

		ImGui::SFML::Update(window, deltaClock.restart());

		if (isConnected)
		{
			grid.UpdateSelection(sf::Mouse::getPosition(window));

			bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
			if (mousePressed && !oldMousePressed)
			{
				std::optional<sf::Vector2i> selection = grid.Selection();
				if (selection.has_value())
				{
					sf::Vector2i val = selection.value();
					spdlog::debug("x:{};y:{}", val.x, val.y);
				}

			}
			oldMousePressed = mousePressed;
		}
		else
		{
			bool shouldTryToConnect = RenderLoginWindow(ipInputBuffer, portInputBuffer, ip, port);

			if (!isConnecting && shouldTryToConnect)
			{
				isConnecting = shouldTryToConnect;
			}

			if (isConnecting)
			{
				sf::Socket::Status status = socket.connect(ip, port, sf::seconds(10.0f));

				if (status == sf::Socket::Disconnected || status == sf::Socket::Error)
				{
					std::cerr << "Error with the connection to the server.\n";
					isConnected = false;
					isConnecting = false;
				}

				if (status == sf::Socket::Done || status == sf::Socket::NotReady)
				{
					isConnected = true;
					isConnecting = false;
					std::cout << "Connection successful !\n";
				}
			}
		}

		window.clear();

		ImGui::SFML::Render(window);

		if (isConnected)
		{
			window.draw(grid);
		}

		window.display();
	}

	ImGui::SFML::Shutdown();
	return EXIT_SUCCESS;
}