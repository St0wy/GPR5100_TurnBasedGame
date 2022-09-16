#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include "GameState.hpp"
#include "MorpionGrid.hpp"

namespace stw
{
	class Client
	{
	public:
		Client();

		void Run();
	private:
		sf::RenderWindow _window;
		sf::TcpSocket _socket;
		MorpionGrid _grid{};

		bool _mousePressed = false;

		char _portInputBuffer[PORT_BUFFER_SIZE] = "8008";
		char _ipInputBuffer[IP_BUFFER_SIZE] = "localhost";

		sf::Packet _receivePacket;
		sf::Packet _playerNumberPacket;
		GameState _state = GameState::ConnectingToServer;
		PlayerNumber _myNumber = PlayerNumber::None;
		sf::Font _lModern;
		sf::Text _text;

		sf::Clock _deltaClock;

		bool RenderLoginWindow(sf::IpAddress& ip, unsigned short& port);
		void HandleServerConnection();
		void FindPlayerNumber();
		void CheckWin();
		void HandlePlayer();
	};
}
