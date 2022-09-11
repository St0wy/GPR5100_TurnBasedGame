#pragma once

enum class ServerState
{
	WaitingForP1Connexion,
	WaitingForP2Connexion,
	WaitingForP1Move,
	WaitingForP2Move,
	P1Win,
	P2Win,
};