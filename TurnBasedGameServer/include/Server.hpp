#pragma once

#include <array>

#include <SFML/Network/SocketSelector.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include "ServerState.hpp"

namespace stw
{
	class Server
	{
	public:
		Server();

		void Run();
	private:
		sf::TcpListener _listener;
		sf::SocketSelector _selector;
		std::array<sf::TcpSocket, 2> _players{};
		ServerState _state = ServerState::WaitingForP1Connexion;

		void ConnectSockets();
		void ReceiveSockets();
	};
}
