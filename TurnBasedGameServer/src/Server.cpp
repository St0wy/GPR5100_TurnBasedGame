#include "Server.hpp"

#include <spdlog/spdlog.h>
#include <SFML/Network/TcpSocket.hpp>

#include "Consts.hpp"
#include "Packet.hpp"

stw::Server::Server()
{
	if (_listener.listen(SERVER_PORT) != sf::Socket::Done)
	{
		spdlog::error("Can't listen.");
	}

	_selector.add(_listener);
}

void stw::Server::Run()
{
	spdlog::info("Starting server...");
	while (true)
	{
		// Make the selector wait for data on any socket
		if (_selector.wait(sf::seconds(10.0f)))
		{
			if (_state == ServerState::WaitingForP1Connexion || _state == ServerState::WaitingForP2Connexion)
			{
				ConnectSockets();
			}
			else if (_state == ServerState::WaitingForP1Move || _state == ServerState::WaitingForP2Move)
			{
				ReceiveSockets();
			}
		}
	}
}

void stw::Server::ConnectSockets()
{
	if (!_selector.isReady(_listener)) return;

	spdlog::info("Listening for a connection...");
	// The listener is ready, there is a connection
	// ReSharper disable once CppTooWideScopeInitStatement

	if (_state == ServerState::WaitingForP1Connexion)
	{
		if (_listener.accept(_players[0]) == sf::Socket::Done)
		{
			spdlog::info("Connection success with P1 !");
			_selector.add(_players[0]);

			sf::Packet packet;
			const InitGamePacket initGamePacket(PlayerNumber::P1);
			packet << initGamePacket;

			if (_players[0].send(packet) != sf::Socket::Done)
			{
				spdlog::error("Problem with sending player number to P1");
			}

			_state = ServerState::WaitingForP2Connexion;
		}
	}
	else if (_state == ServerState::WaitingForP2Connexion)
	{
		if (_listener.accept(_players[1]) == sf::Socket::Done)
		{
			spdlog::info("Connection success with P2 !");
			_selector.add(_players[1]);

			sf::Packet packetP2;
			const InitGamePacket initGamePacket(PlayerNumber::P2);
			packetP2 << initGamePacket;

			if (_players[1].send(packetP2) != sf::Socket::Done)
			{
				spdlog::error("Problem with sending player number to P2");
			}

			// Init start game packet
			sf::Packet packetP1;
			const Packet startGamePacket(PacketType::StartGame);
			packetP1 << startGamePacket;

			// Send start game packet
			if (_players[0].send(packetP1) != sf::Socket::Done)
			{
				spdlog::error("Problem with sending start game info to P1");
			}

			_state = ServerState::WaitingForP1Move;
		}
	}
}

void stw::Server::ReceiveSockets()
{
	for (sf::TcpSocket& player : _players)
	{
		if (!_selector.isReady(player)) continue;

		// The client has some data
		// ReSharper disable once CppTooWideScopeInitStatement
		sf::Packet packet;
		if (player.receive(packet) == sf::Socket::Done)
		{
			spdlog::info("Listening received data...");

			// Read packet
			MovePacket movePacket;
			packet >> movePacket;
			spdlog::debug("Packet recieved : x:{0}, y:{1}", movePacket.moveVector.x, movePacket.moveVector.y);

			if (movePacket.type != PacketType::Move)
			{
				spdlog::error("Wrong packet type recieved");
			}

			// Reset packet
			packet.clear();
			packet << movePacket;

			switch (movePacket.playerNumber) {
			case PlayerNumber::None:
				spdlog::error("No player.");
				break;
			case PlayerNumber::P1:
				spdlog::debug("Trying to send move to P2...");
				if (_players[1].send(packet) == sf::Socket::Done)
				{
					spdlog::info("Transfered P1 move to P2");
				}
				break;
			case PlayerNumber::P2:
				spdlog::debug("Trying to send move to P1...");
				if (_players[0].send(packet) == sf::Socket::Done)
				{
					spdlog::info("Transfered P2 move to P1");
				}
				break;
			}
		}
	}
}
