#include "Packet.hpp"

stw::Packet::Packet() : Packet(PacketType::None)
{
}

stw::Packet::Packet(const PacketType type)
	: _packetType(type)
{
}

stw::PacketType stw::Packet::Type() const
{
	return _packetType;
}

stw::InitGamePacket::InitGamePacket(const PlayerNumber playerNumber)
	: Packet(PacketType::InitGame), _playerNumber(playerNumber)
{
}

stw::PlayerNumber stw::InitGamePacket::Player() const
{
	return _playerNumber;
}
