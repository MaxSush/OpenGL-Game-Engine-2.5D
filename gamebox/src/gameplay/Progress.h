#pragma once

enum class GameState
{
	PLAYING,
	DEAD,
	WIN
};

inline const char* ToString(GameState state)
{
	switch (state)
	{
	case GameState::PLAYING:
		return "PLAYING";
	case GameState::DEAD:
		return "DEAD";
	case GameState::WIN:
		return "WIN";
	default:
		return "UNKNOWN";
	}
}

enum class DeathCause
{
	NONE,
	HAZARD,
	ENEMY,
	FALL
};

inline const char* ToString(DeathCause cause)
{
	switch (cause)
	{
	case DeathCause::NONE:
		return "NONE";
	case DeathCause::HAZARD:
		return "HAZARD";
	case DeathCause::ENEMY:
		return "ENEMY";
	case DeathCause::FALL:
		return "FALL";
	default:
		return "UNKNOWN";
	}
}

struct Progress
{
	GameState state = GameState::PLAYING;
	DeathCause cause = DeathCause::NONE;
	int coins = 0;
};
