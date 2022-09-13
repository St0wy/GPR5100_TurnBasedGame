#include "Packet.hpp"

stw::Packet::Packet() : Packet(PacketType::None)
{
}

stw::Packet::Packet(const PacketType type)
	: type(type)
{
}

sf::Packet& stw::operator<<(sf::Packet& packet, const Packet& p)
{
	const auto type = static_cast<int>(p.type);
	return packet << type;
}

sf::Packet& stw::operator>>(sf::Packet& packet, Packet& p)
{
	int type = 0;
	packet >> type;
	p.type = static_cast<PacketType>(type);
	return packet;
}

stw::InitGamePacket::InitGamePacket()
	:InitGamePacket(stw::PlayerNumber::None)
{
}

stw::InitGamePacket::InitGamePacket(const PlayerNumber playerNumber)
	: Packet(PacketType::InitGame), playerNumber(playerNumber)
{
}

sf::Packet& stw::operator<<(sf::Packet& packet, const PlayerNumber& p)
{
	const auto number = static_cast<int>(p);
	return packet << number;
}

sf::Packet& stw::operator>>(sf::Packet& packet, PlayerNumber& p)
{
	int number = 0;
	packet >> number;
	p = static_cast<PlayerNumber>(number);
	return packet;
}

sf::Packet& stw::operator<<(sf::Packet& packet, const InitGamePacket& p)
{
	const auto type = static_cast<int>(p.type);
	const auto number = static_cast<int>(p.playerNumber);
	return packet << type << number;
}

sf::Packet& stw::operator>>(sf::Packet& packet, InitGamePacket& p)
{
	int type = 0;
	int number = 0;
	packet >> type >> number;
	p.type = static_cast<PacketType>(type);
	p.playerNumber = static_cast<PlayerNumber>(number);
	return packet;
}

stw::MovePacket::MovePacket()
	: moveVector(0, 0), playerNumber(PlayerNumber::None)
{
}

sf::Packet& stw::operator<<(sf::Packet& packet, const sf::Vector2i& moveVector)
{
	return packet << moveVector.x << moveVector.y;
}

sf::Packet& stw::operator>>(sf::Packet& packet, sf::Vector2i& moveVector)
{
	return packet >> moveVector.x >> moveVector.y;
}

sf::Packet& stw::operator<<(sf::Packet& packet, const MovePacket& p)
{
	return packet << p.moveVector;
}

sf::Packet& stw::operator>>(sf::Packet& packet, MovePacket& p)
{
	return packet >> p.moveVector;
}
