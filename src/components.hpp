#pragma once

// Components are simple structs or classes that contain data.
struct EntityType {
	int type; //0 = gumbels, 1 = spaceship

	bool operator==(const int t) const { //not sure if that is correct
		return type == t;
	}
};

struct Position {
	float x, y;

	bool operator==(const Position& p) const { //second const is saying that this function is not changing the position struct when calling it, first const is used because we pass a reference (because it is faster) but don´t want to mutate the value
		return p.x == x && p.y == y;
	}
};

struct Alive {
	bool alive;

	Alive& operator=(bool state) { // I dont understand this
		alive = state;
		return *this;
	}

	explicit operator bool() const {
		return alive;
	}
};

struct Movement {
	float acceleration;
};

struct Shooting {
	bool is_shooting;

	explicit operator bool() const {  
		return is_shooting;
	}
	Shooting& operator=(bool state) { // I dont understand this
		is_shooting = state;
		return *this;
	}

};

struct FrameCounter {
	int frame_count;

	bool operator==(const FrameCounter& f) const { 
		return f.frame_count == frame_count;
	}
};

struct Lives {
	int lives;
};

struct Cooldown {
	int cooldown;
};

struct Score {
	int score;
};

enum class PlayerController {
	Left,
	Right,
	Shoot,
	End,
	None
};

enum class GumbelDirection {
	Left,
	Right,
	Down
};

enum class BulletDirection {
	Up,
	Down,
	None //not happy with this
};