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

void ReceiveSockets(const sf::SocketSelector& selector, std::array<sf::TcpSocket, 2>& players)
{
	for (sf::TcpSocket& player : players)
	{
		if (!selector.isReady(player)) continue;

		// The client has some data
		// ReSharper disable once CppTooWideScopeInitStatement
		sf::Packet packet;
		if (player.receive(packet) == sf::Socket::Done)
		{
			spdlog::info("Listening received data...");

			// Read packet
			stw::MovePacket movePacket;
			packet >> movePacket;
			spdlog::debug("Packet recieved : x:{0}, y:{1}", movePacket.moveVector.x, movePacket.moveVector.y);

			if (movePacket.type != stw::PacketType::Move)
			{
				spdlog::error("Wrong packet type recieved");
			}

			// Reset packet
			packet.clear();
			packet << movePacket;

			switch (movePacket.playerNumber) {
			case stw::PlayerNumber::None:
				spdlog::error("No player.");
				break;
			case stw::PlayerNumber::P1:
				spdlog::debug("Trying to send move to P2...");
				if (players[1].send(packet) == sf::Socket::Done)
				{
					spdlog::info("Transfered P1 move to P2");
				}
				break;
			case stw::PlayerNumber::P2:
				spdlog::debug("Trying to send move to P1...");
				if (players[0].send(packet) == sf::Socket::Done)
				{
					spdlog::info("Transfered P2 move to P1");
				}
				break;
			}
		}
	}
}

void ConnectSockets(sf::TcpListener& listener, sf::SocketSelector& selector,
	std::array<sf::TcpSocket, 2>& players, ServerState& state)
{
	if (!selector.isReady(listener)) return;

	spdlog::info("Listening for a connection...");
	// The listener is ready, there is a connection
	// ReSharper disable once CppTooWideScopeInitStatement

	if (state == ServerState::WaitingForP1Connexion)
	{
		if (listener.accept(players[0]) == sf::Socket::Done)
		{
			spdlog::info("Connection success with P1 !");
			selector.add(players[0]);

			sf::Packet packet;
			const stw::InitGamePacket initGamePacket(stw::PlayerNumber::P1);
			packet << initGamePacket;

			if (players[0].send(packet) != sf::Socket::Done)
			{
				spdlog::error("Problem with sending player number to P1");
			}

			state = ServerState::WaitingForP2Connexion;
		}
	}
	else if (state == ServerState::WaitingForP2Connexion)
	{
		if (listener.accept(players[1]) == sf::Socket::Done)
		{
			spdlog::info("Connection success with P2 !");
			selector.add(players[1]);

			sf::Packet packetP2;
			const stw::InitGamePacket initGamePacket(stw::PlayerNumber::P2);
			packetP2 << initGamePacket;

			if (players[1].send(packetP2) != sf::Socket::Done)
			{
				spdlog::error("Problem with sending player number to P2");
			}

			// Init start game packet
			sf::Packet packetP1;
			const stw::Packet startGamePacket(stw::PacketType::StartGame);
			packetP1 << startGamePacket;

			// Send start game packet
			if (players[0].send(packetP1) != sf::Socket::Done)
			{
				spdlog::error("Problem with sending start game info to P1");
			}

			state = ServerState::WaitingForP1Move;
		}
	}
}

int main()
{
	spdlog::set_level(spdlog::level::debug);

	sf::TcpListener listener;
	if (listener.listen(stw::SERVER_PORT) != sf::Socket::Done)
	{
		spdlog::error("Can't listen.");
		return EXIT_FAILURE;
	}

	sf::SocketSelector selector;
	selector.add(listener);

	std::array<sf::TcpSocket, 2> players{};

	auto state = ServerState::WaitingForP1Connexion;

	spdlog::info("Starting server...");
	while (true)
	{
		// Make the selector wait for data on any socket
		if (selector.wait(sf::seconds(10.0f)))
		{
			if (state == ServerState::WaitingForP1Connexion || state == ServerState::WaitingForP2Connexion)
			{
				ConnectSockets(listener, selector, players, state);
			}
			else if (state == ServerState::WaitingForP1Move || state == ServerState::WaitingForP2Move)
			{
				ReceiveSockets(selector, players);
			}
		}
	}

	return EXIT_SUCCESS;
}
