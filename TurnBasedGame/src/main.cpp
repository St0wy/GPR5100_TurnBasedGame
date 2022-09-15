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
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

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
	spdlog::info("Finding number");
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
			state = stw::GameState::WaitingForP2Connexion;
			break;
		case stw::PlayerNumber::P2:
			state = stw::GameState::WaitingForMove;
			break;
		}
	}
}

void CheckWin(const stw::MorpionGrid& grid, stw::GameState& state, const stw::PlayerNumber myNumber)
{
	const std::optional<stw::PlayerNumber> winner = grid.GetWinner();
	if (winner.has_value())
	{
		const stw::PlayerNumber winnerNumber = winner.value();
		if (winnerNumber == stw::PlayerNumber::None)
		{
			state = stw::GameState::Draw;
		}
		else if (winnerNumber == myNumber)
		{
			state = stw::GameState::Win;
		}
		else
		{
			state = stw::GameState::Lose;
		}
	}
}

void HandlePlayer(const sf::RenderWindow& window, sf::TcpSocket& socket, stw::MorpionGrid& grid,
	const bool mousePressed, stw::GameState& state, const stw::PlayerNumber myNumber)
{
	grid.UpdateSelection(sf::Mouse::getPosition(window));

	if (mousePressed)
	{
		const std::optional<sf::Vector2i> selection = grid.Selection();
		if (selection.has_value())
		{
			// Store and print move
			stw::MovePacket movePacket{};
			const sf::Vector2i move = selection.value();
			movePacket.moveVector = move;
			movePacket.playerNumber = myNumber;
			spdlog::debug("MOVE : x:{};y:{}", movePacket.moveVector.x, movePacket.moveVector.y);

			sf::Packet sendingPacket;
			sendingPacket << movePacket;

			const sf::Socket::Status status = socket.send(sendingPacket);
			if (status == sf::Socket::Done)
			{
				state = stw::GameState::WaitingForMove;
				grid.Play(move, myNumber);
				grid.ResetSelection();

				CheckWin(grid, state, myNumber);
			}
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
	bool mousePressed = false;

	char portInputBuffer[stw::PORT_BUFFER_SIZE] = "8008";
	char ipInputBuffer[stw::IP_BUFFER_SIZE] = "localhost";

	sf::Packet receivePacket;
	sf::Packet playerNumberPacket;
	auto state = stw::GameState::ConnectingToServer;
	auto myNumber = stw::PlayerNumber::None;
	sf::Font lModern;
	sf::Text text;

	if (!lModern.loadFromFile("data/lmodern.otf"))
	{
		spdlog::error("Could not load lmodern font");
	}

	text.setFont(lModern);
	text.setCharacterSize(64);
	text.setFillColor(sf::Color::White);

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

			mousePressed = event.type == sf::Event::MouseButtonPressed && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
		}

		ImGui::SFML::Update(window, deltaClock.restart());

		switch (state) {
		case stw::GameState::ConnectingToServer:
			HandleServerConnection(socket, ipInputBuffer, portInputBuffer, state);
			break;
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
			HandlePlayer(window, socket, grid, mousePressed, state, myNumber);
		}
		break;
		case stw::GameState::WaitingForMove:
			receivePacket.clear();
			if (socket.receive(receivePacket) == sf::Socket::Done)
			{
				stw::MovePacket move;
				receivePacket >> move;
				spdlog::debug("RECIEVE MOVE : x:{};y:{}", move.moveVector.x, move.moveVector.y);
				grid.Play(move.moveVector, move.playerNumber);
				state = stw::GameState::Playing;
				CheckWin(grid, state, myNumber);
			}
			break;
		case stw::GameState::Win:
			text.setString("You Win !");
			break;
		case stw::GameState::Lose:
			text.setString("You Lose !");
			break;
		case stw::GameState::Draw:
			text.setString("There is no winner !");
			break;
		}

		window.clear();

		ImGui::SFML::Render(window);

		if (state == stw::GameState::Playing || state == stw::GameState::WaitingForMove)
		{
			window.draw(grid);
		}
		else if (state == stw::GameState::Win || state == stw::GameState::Lose || state == stw::GameState::Draw)
		{
			window.draw(text);
		}

		window.display();
	}

	ImGui::SFML::Shutdown();
	return EXIT_SUCCESS;
}
