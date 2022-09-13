#include <iostream>
#include <imgui.h>
#include <imgui-SFML.h>
#include <spdlog/spdlog.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/Network/Socket.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include "MorpionGrid.hpp"
#include "GameState.hpp"
#include "Packet.hpp"

bool RenderLoginWindow(char ipInputBuffer[stw::IP_BUFFER_SIZE], char portInputBuffer[stw::PORT_BUFFER_SIZE],
	sf::IpAddress& ip, unsigned short& port)
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


void HandleServerConnection(sf::TcpSocket& socket, char ipInputBuffer[stw::IP_BUFFER_SIZE],
	char portInputBuffer[stw::PORT_BUFFER_SIZE], stw::GameState& state)
{
	static bool isConnecting = false;
	sf::IpAddress ip{};
	unsigned short port{};

	const bool shouldTryToConnect = RenderLoginWindow(ipInputBuffer, portInputBuffer, ip, port);

	if (!isConnecting && shouldTryToConnect)
	{
		isConnecting = shouldTryToConnect;
	}

	if (isConnecting)
	{
		const sf::Socket::Status status = socket.connect(ip, port, sf::seconds(10.0f));

		if (status == sf::Socket::Disconnected || status == sf::Socket::Error)
		{
			spdlog::error("Error with the connection to the server.");
			state = stw::GameState::ConnectingToServer;
			isConnecting = false;
		}

		if (status == sf::Socket::Done || status == sf::Socket::NotReady)
		{
			state = stw::GameState::WaitingForPlayerNumber;
			isConnecting = false;
			spdlog::info("Connection successful !");
		}
	}
}

void FindPlayerNumber(sf::TcpSocket& socket, sf::Packet& playerNumberPacket, stw::GameState& state, stw::PlayerNumber& myNumber)
{
	if (socket.receive(playerNumberPacket) == sf::Socket::Done)
	{
		stw::InitGamePacket initGamePacket;
		playerNumberPacket >> initGamePacket;
		myNumber = initGamePacket.playerNumber;
		spdlog::info("My number is {}", static_cast<int>(myNumber));
		switch (myNumber) {
		case stw::PlayerNumber::None:
			state = stw::GameState::ConnectingToServer;
			break;
		case stw::PlayerNumber::P1:
			state = stw::GameState::Playing;
			break;
		case stw::PlayerNumber::P2:
			state = stw::GameState::WaitingForMove;
			break;
		}
	}
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

	sf::Packet sendingPacket;
	sf::Packet receivePacket;
	sf::Packet playerNumberPacket;
	auto state = stw::GameState::ConnectingToServer;
	auto myNumber = stw::PlayerNumber::None;

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

		switch (state) {
		case stw::GameState::ConnectingToServer:
		{
			HandleServerConnection(socket, ipInputBuffer, portInputBuffer, state);
			break;
		}
		case stw::GameState::WaitingForPlayerNumber:
			FindPlayerNumber(socket, playerNumberPacket, state, myNumber);
			break;
		case stw::GameState::WaitingForP2Connexion:
			playerNumberPacket.clear();
			if (socket.receive(playerNumberPacket) == sf::Socket::Done)
			{
				stw::Packet startGamePacket;
				playerNumberPacket >> startGamePacket;
				if (startGamePacket.type == stw::PacketType::StartGame)
				{
					state = stw::GameState::Playing;
				}
			}
			break;
		case stw::GameState::Playing:
		{
			grid.UpdateSelection(sf::Mouse::getPosition(window));

			bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
			if (mousePressed && !oldMousePressed)
			{
				std::optional<sf::Vector2i> selection = grid.Selection();
				if (selection.has_value())
				{
					// Store and print move
					stw::MovePacket movePacket;
					movePacket.moveVector = selection.value();;
					spdlog::debug("MOVE : x:{};y:{}", movePacket.moveVector.x, movePacket.moveVector.y);

					sendingPacket.clear();
					sendingPacket << movePacket;

					sf::Socket::Status status = socket.send(sendingPacket);
					if (status == sf::Socket::Done)
					{
						state = stw::GameState::WaitingForMove;
						grid.ResetSelection();
					}
				}
			}
			oldMousePressed = mousePressed;
		}
		break;
		case stw::GameState::WaitingForMove:
			break;
		case stw::GameState::Win:
			break;
		case stw::GameState::Lose:
			break;
		}

		window.clear();

		ImGui::SFML::Render(window);

		if (state == stw::GameState::Playing || state == stw::GameState::WaitingForMove)
		{
			window.draw(grid);
		}

		window.display();
	}

	ImGui::SFML::Shutdown();
	return EXIT_SUCCESS;
}
