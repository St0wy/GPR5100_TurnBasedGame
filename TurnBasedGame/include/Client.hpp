#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include "GameState.hpp"
#include "MorpionGrid.hpp"


class Client
{
public:
	Client();

	void Run();
	bool RenderLoginWindow(sf::IpAddress& ip, unsigned short& port);
	void HandleServerConnection();
	void FindPlayerNumber();
	void CheckWin();
	void HandlePlayer();
private:
	sf::RenderWindow _window;
	sf::TcpSocket _socket;
	stw::MorpionGrid _grid{};

	bool _mousePressed = false;

	char _portInputBuffer[stw::PORT_BUFFER_SIZE] = "8008";
	char _ipInputBuffer[stw::IP_BUFFER_SIZE] = "localhost";

	sf::Packet _receivePacket;
	sf::Packet _playerNumberPacket;
	stw::GameState _state = stw::GameState::ConnectingToServer;
	stw::PlayerNumber _myNumber = stw::PlayerNumber::None;
	sf::Font _lModern;
	sf::Text _text;

	sf::Clock _deltaClock;
};
