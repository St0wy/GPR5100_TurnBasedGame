#pragma once
#include <array>

#include "Consts.hpp"

namespace stw
{
	enum class PacketType : char
	{
		None = 0,
		InitGame,
		Move,
		EndGame,
	};

	enum class PlayerNumber : char
	{
		None = 0,
		/**
		 * \brief This player is the X.
		 */
		P1,

		/**
		 * \brief This player is the O.
		 */
		P2,
	};



	class Packet
	{
	public:
		Packet();
		explicit Packet(PacketType type);

		[[nodiscard]] PacketType Type() const;
	private:
		PacketType _packetType;
	};

	class InitGamePacket : Packet
	{
	public:
		explicit InitGamePacket(PlayerNumber playerNumber);

		[[nodiscard]] PlayerNumber Player() const;
	private:
		PlayerNumber _playerNumber;
	};
}
