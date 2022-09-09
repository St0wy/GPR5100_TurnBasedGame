#pragma once

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
		P1,
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