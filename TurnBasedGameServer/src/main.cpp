#include <iostream>
#include <array>
#include <spdlog/spdlog.h>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/SocketSelector.hpp>
#include <SFML/Network/Packet.hpp>

#include "Consts.hpp"
#include "Packet.hpp"
#include "ServerState.hpp"

void ReceiveSockets(const sf::SocketSelector& selector, const std::array<sf::TcpSocket*, 2>& players)
{
	for (sf::TcpSocket* player : players)
	{
		if (!selector.isReady(*player)) continue;

		// The client has some data
		// ReSharper disable once CppTooWideScopeInitStatement
		sf::Packet packet;
		if (player->receive(packet) == sf::Socket::Done)
		{
			spdlog::info("Listening received data...");

			// Read packet
			stw::MovePacket movePacket;
			packet >> movePacket;
			spdlog::debug("Packet : x:{0}, y:{1}", movePacket.moveVector.x, movePacket.moveVector.y);

			// Reset packet
			//packet.clear();
			//packet << movePacket;

			//switch (movePacket.playerNumber) {
			//case stw::PlayerNumber::None:
			//	break;
			//case stw::PlayerNumber::P1:
			//	if (players[1]->send(packet) == sf::Socket::Done)
			//	{
			//		spdlog::info("Transfered P1 move to P2");
			//	}
			//	break;
			//case stw::PlayerNumber::P2:
			//	if (players[0]->send(packet) == sf::Socket::Done)
			//	{
			//		spdlog::info("Transfered P2 move to P1");
			//	}
			//	break;
			//}
		}
	}
}

void ConnectSockets(sf::TcpListener& listener, sf::SocketSelector& selector,
	sf::TcpSocket& playerOne, sf::TcpSocket& playerTwo, ServerState& state)
{
	spdlog::info("Listening for a connection...");
	// The listener is ready, there is a connection
	// ReSharper disable once CppTooWideScopeInitStatement

	if (state == ServerState::WaitingForP1Connexion)
	{
		if (listener.accept(playerOne) == sf::Socket::Done)
		{
			spdlog::info("Connection success with P1 !");
			selector.add(playerOne);

			sf::Packet packet;
			const stw::InitGamePacket initGamePacket(stw::PlayerNumber::P1);
			packet << initGamePacket;

			if (playerOne.send(packet) != sf::Socket::Done)
			{
				spdlog::error("Problem with sending player number to P1");
			}

			state = ServerState::WaitingForP2Connexion;
		}
	}
	else if (state == ServerState::WaitingForP2Connexion)
	{
		if (listener.accept(playerTwo) == sf::Socket::Done)
		{
			spdlog::info("Connection success with P2 !");
			selector.add(playerTwo);

			// Init start game packet
			sf::Packet packet;
			const stw::Packet startGamePacket(stw::PacketType::StartGame);
			packet << startGamePacket;

			// Send start game packet
			if (playerOne.send(packet) != sf::Socket::Done)
			{
				spdlog::error("Problem with sending start game info to P1");
			}

			state = ServerState::WaitingForP1Move;
		}
	}
}

int main()
{
	sf::TcpListener listener;
	if (listener.listen(stw::SERVER_PORT) != sf::Socket::Done)
	{
		spdlog::error("Can't listen.");
		return EXIT_FAILURE;
	}

	sf::SocketSelector selector;
	selector.add(listener);

	sf::TcpSocket playerOne;
	sf::TcpSocket playerTwo;

	const std::array players{ &playerOne, &playerTwo };

	auto state = ServerState::WaitingForP1Connexion;

	spdlog::info("Starting server...");
	while (true)
	{
		// Make the selector wait for data on any socket
		if (selector.wait(sf::seconds(10.0f)))
		{
			if (selector.isReady(listener))
			{
				ConnectSockets(listener, selector, playerOne, playerTwo, state);
			}
			else
			{
				if (state == ServerState::WaitingForP1Move || state == ServerState::WaitingForP2Move)
				{
					// The listener socket is not ready, test all other sockets (the clients)
					ReceiveSockets(selector, players);
				}
			}
		}
	}

	return EXIT_SUCCESS;
}
