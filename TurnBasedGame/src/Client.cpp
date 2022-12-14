#include "Client.hpp"

#include <imgui-SFML.h>
#include <imgui.h>

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>

#include <spdlog/spdlog.h>

stw::Client::Client()
	: _window(sf::VideoMode(640 * 2, 480 * 2), "Online Morpion")
{
	if (!ImGui::SFML::Init(_window))
	{
		spdlog::error("Error loading ImGui");
	}

	// Set window scale for HDPI screens
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 2;

	_socket.setBlocking(false);

	_grid.setPosition(30.0f, 30.0f);

	if (!_lModern.loadFromFile("data/lmodern.otf"))
	{
		spdlog::error("Could not load lmodern font");
	}

	_text.setFont(_lModern);
	_text.setCharacterSize(64);
	_text.setFillColor(sf::Color::White);
}

void stw::Client::Run()
{
	while (_window.isOpen())
	{
		sf::Event event{};
		while (_window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(_window, event);

			if (event.type == sf::Event::Closed)
			{
				_window.close();
			}
			else if (event.type == sf::Event::Resized)
			{
				sf::Vector2f size = { static_cast<float>(event.size.width), static_cast<float>(event.size.height) };
				sf::Vector2f center = size / 2.0f;
				_window.setView(sf::View(center, size));
			}

			_mousePressed = event.type == sf::Event::MouseButtonPressed && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
		}

		ImGui::SFML::Update(_window, _deltaClock.restart());

		switch (_state) {
		case GameState::ConnectingToServer:
			HandleServerConnection();
			break;
		case GameState::WaitingForPlayerNumber:
			FindPlayerNumber();
			break;
		case GameState::WaitingForP2Connexion:
			_text.setString("Waiting for P2...");
			_playerNumberPacket.clear();
			if (_socket.receive(_playerNumberPacket) == sf::Socket::Done)
			{
				Packet startGamePacket;
				_playerNumberPacket >> startGamePacket;
				if (startGamePacket.type == PacketType::StartGame)
				{
					_state = GameState::Playing;
				}
			}
			break;
		case GameState::Playing:
		{
			HandlePlayer();
		}
		break;
		case GameState::WaitingForMove:
			_receivePacket.clear();
			if (_socket.receive(_receivePacket) == sf::Socket::Done)
			{
				MovePacket move;
				_receivePacket >> move;
				spdlog::debug("RECIEVE MOVE : x:{};y:{}", move.moveVector.x, move.moveVector.y);
				_grid.Play(move.moveVector, move.playerNumber);
				_state = GameState::Playing;
				CheckWin();
			}
			break;
		case GameState::Win:
			_text.setString("You Win !");
			break;
		case GameState::Lose:
			_text.setString("You Lose !");
			break;
		case GameState::Draw:
			_text.setString("There is no winner !");
			break;
		}

		_window.clear();

		ImGui::SFML::Render(_window);

		if (_state == GameState::Playing || _state == GameState::WaitingForMove)
		{
			_window.draw(_grid);
		}
		else if (_state == GameState::Win || _state == GameState::Lose ||
			_state == GameState::Draw || _state == GameState::WaitingForP2Connexion)
		{
			_window.draw(_text);
		}

		_window.display();
	}

	ImGui::SFML::Shutdown();
}

bool stw::Client::RenderLoginWindow(sf::IpAddress& ip, unsigned short& port)
{
	bool show = true;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);

	ImGui::Begin("Login", &show, WINDOW_FULLSCREEN_FLAGS);
	ImGui::TextWrapped("Please type your name, ip of the server and its ports.");
	// ReSharper disable once CppTooWideScope
	ImGui::InputTextWithHint("##ip", "IP", _ipInputBuffer, IP_BUFFER_SIZE);
	ImGui::InputTextWithHint("##port", "Port", _portInputBuffer, PORT_BUFFER_SIZE);
	bool isConnecting = false;
	if (ImGui::Button("Login"))
	{
		const std::string inputIp(_ipInputBuffer);
		const unsigned short inputPort = static_cast<unsigned short>(std::stoi(_portInputBuffer));
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

void stw::Client::HandleServerConnection()
{
	static bool isConnecting = false;
	sf::IpAddress ip{};
	unsigned short port{};

	const bool shouldTryToConnect = RenderLoginWindow(ip, port);

	if (!isConnecting && shouldTryToConnect)
	{
		isConnecting = shouldTryToConnect;
	}

	if (isConnecting)
	{
		const sf::Socket::Status status = _socket.connect(ip, port, sf::seconds(10.0f));

		if (status == sf::Socket::Disconnected || status == sf::Socket::Error)
		{
			spdlog::error("Error with the connection to the server.");
			_state = GameState::ConnectingToServer;
			isConnecting = false;
		}

		if (status == sf::Socket::Done || status == sf::Socket::NotReady)
		{
			_state = GameState::WaitingForPlayerNumber;
			isConnecting = false;
			spdlog::info("Connection successful !");
		}
	}
}

void stw::Client::FindPlayerNumber()
{
	if (_socket.receive(_playerNumberPacket) == sf::Socket::Done)
	{
		InitGamePacket initGamePacket;
		_playerNumberPacket >> initGamePacket;
		_myNumber = initGamePacket.playerNumber;
		spdlog::info("My number is {}", static_cast<int>(_myNumber));
		switch (_myNumber) {
		case PlayerNumber::None:
			_state = GameState::ConnectingToServer;
			break;
		case PlayerNumber::P1:
			_state = GameState::WaitingForP2Connexion;
			break;
		case PlayerNumber::P2:
			_state = GameState::WaitingForMove;
			break;
		}
	}
}

void stw::Client::CheckWin()
{
	const std::optional<PlayerNumber> winner = _grid.GetWinner();
	if (winner.has_value())
	{
		const PlayerNumber winnerNumber = winner.value();
		if (winnerNumber == PlayerNumber::None)
		{
			_state = GameState::Draw;
		}
		else if (winnerNumber == _myNumber)
		{
			_state = GameState::Win;
		}
		else
		{
			_state = GameState::Lose;
		}
	}
}

void stw::Client::HandlePlayer()
{
	_grid.UpdateSelection(sf::Mouse::getPosition(_window));

	if (_mousePressed)
	{
		const std::optional<sf::Vector2i> selection = _grid.Selection();
		if (selection.has_value())
		{
			// Store and print move
			MovePacket movePacket{};
			const sf::Vector2i move = selection.value();
			movePacket.moveVector = move;
			movePacket.playerNumber = _myNumber;
			spdlog::debug("MOVE : x:{};y:{}", movePacket.moveVector.x, movePacket.moveVector.y);

			sf::Packet sendingPacket;
			sendingPacket << movePacket;

			const sf::Socket::Status status = _socket.send(sendingPacket);
			if (status == sf::Socket::Done)
			{
				_state = GameState::WaitingForMove;
				_grid.Play(move, _myNumber);
				_grid.ResetSelection();

				CheckWin();
			}
		}
	}
}
