#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Network/Packet.hpp>

namespace stw
{
	enum class PacketType : int
	{
		None = 0,
		InitGame,
		StartGame,
		Move,
		EndGame,
	};

	enum class PlayerNumber : int
	{
		None = 0,
		/**
		 * \brief This player is the X.
		 */
		P1 = 1,

		/**
		 * \brief This player is the O.
		 */
		P2 = 2,
	};

	class Packet
	{
	public:
		Packet();
		explicit Packet(PacketType type);
		PacketType type;
	};

	sf::Packet& operator <<(sf::Packet& packet, const Packet& p);
	sf::Packet& operator >>(sf::Packet& packet, Packet& p);

	class InitGamePacket : public Packet
	{
	public:
		InitGamePacket();
		explicit InitGamePacket(PlayerNumber playerNumber);

		PlayerNumber playerNumber;
	};

	sf::Packet& operator <<(sf::Packet& packet, const PlayerNumber& p);
	sf::Packet& operator >>(sf::Packet& packet, PlayerNumber& p);

	sf::Packet& operator <<(sf::Packet& packet, const InitGamePacket& p);
	sf::Packet& operator >>(sf::Packet& packet, InitGamePacket& p);

	class MovePacket : public Packet
	{
	public:
		MovePacket();

		sf::Vector2<int> moveVector;
		PlayerNumber playerNumber;
	};

	sf::Packet& operator <<(sf::Packet& packet, const sf::Vector2i& moveVector);
	sf::Packet& operator >>(sf::Packet& packet, sf::Vector2i& moveVector);

	sf::Packet& operator <<(sf::Packet& packet, const MovePacket& p);
	sf::Packet& operator >>(sf::Packet& packet, MovePacket& p);

}
