#include <iostream>
#include <array>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/SocketSelector.hpp>
#include <SFML/Network/Packet.hpp>

#include "Consts.hpp"
#include "ServerState.hpp"

void ListenSockets(const sf::SocketSelector& selector, std::array<sf::TcpSocket*, 2>& players)
{
	for (sf::TcpSocket* player : players)
	{
		if (!selector.isReady(*player)) continue;

		// The client has some data
		// ReSharper disable once CppTooWideScopeInitStatement
		sf::Packet packet;
		if (player->receive(packet) == sf::Socket::Done)
		{
			std::cout << "Listening received data.\n";
			// Read packet
			//Message message;
			//packet >> message;
			//messages.push_back(message);
			//std::cout << message.ToString() << "\n";


			// Do something
		}
	}
}

int main()
{
	sf::TcpListener listener;
	if (listener.listen(stw::SERVER_PORT) != sf::Socket::Done)
	{
		std::cerr << "Can't listen.\n";
		return EXIT_FAILURE;
	}

	sf::SocketSelector selector;
	selector.add(listener);

	sf::TcpSocket playerOne;
	sf::TcpSocket playerTwo;

	std::array players{ &playerOne, &playerTwo };

	auto state = ServerState::WaitingForP1Connexion;

	std::cout << "Starting server...\n";
	while (true)
	{
		// Make the selector wait for data on any socket
		if (selector.wait(sf::seconds(10.0f)))
		{
			if (selector.isReady(listener))
			{
				std::cout << "Listening for a connection...\n";
				// The listener is ready, there is a connection
				// ReSharper disable once CppTooWideScopeInitStatement

				if (state == ServerState::WaitingForP1Connexion)
				{
					if (listener.accept(playerOne) == sf::Socket::Done)
					{
						std::cout << "Connection success with P1 !\n";
						selector.add(playerOne);
						state = ServerState::WaitingForP2Connexion;
					}
				}
				else if (state == ServerState::WaitingForP2Connexion)
				{
					if (listener.accept(playerTwo) == sf::Socket::Done)
					{
						std::cout << "Connection success with P2 !\n";
						selector.add(playerTwo);
						state = ServerState::WaitingForP1Move;
					}
				}
			}
			else
			{
				if (state == ServerState::WaitingForP1Move || state == ServerState::WaitingForP2Move)
				{
					// The listener socket is not ready, test all other sockets (the clients)
					ListenSockets(selector, players);
				}
			}
		}
	}

	return EXIT_SUCCESS;
}
