#pragma once
#include "main.h"

//My Systems
class MovementSystem {

public:

	void update(PlayerController input, FrameCounter& frame_counter) {
		std::vector<Easys::Entity> entities_to_be_moved = ecs.getEntitiesByComponent<Movement>();
		for (Easys::Entity entity : entities_to_be_moved) {
			Movement& movement = ecs.getComponent<Movement>(entity);
			Position& position = ecs.getComponent<Position>(entity);
			EntityType& ent_type = ecs.getComponent<EntityType>(entity);
			if (ent_type == 1 && input == PlayerController::Left && position.x >= 0) {
				position.x -= movement.acceleration;
			}
			else if (ent_type == 1 && input == PlayerController::Right && position.x <= 640 * 2 - PIXEL_SCALE * 8) {
				position.x += movement.acceleration;
			}
			else if (ent_type == 0 && frame_counter.frame_count % 60 == 0) {
				GumbelDirection& gumbel_direction = ecs.getComponent<GumbelDirection>(entity);
				if (gumbel_direction == GumbelDirection::Right) {
					position.x += movement.acceleration;
				}
				else if (gumbel_direction == GumbelDirection::Left) {
					position.x -= movement.acceleration;
				}
			}
		}
	}
};


class ShootingSystem {

public:

	std::vector<Easys::Entity> update() {
		std::vector<Easys::Entity> entities_shooting = ecs.getEntitiesByComponent<Shooting>();
		for (Easys::Entity entity : entities_shooting) {
			Shooting& is_shooting = ecs.getComponent<Shooting>(entity);
			Position& position = ecs.getComponent<Position>(entity);
			EntityType& entity_type = ecs.getComponent<EntityType>(entity);
			if (is_shooting) {
				Easys::Entity bullet = shots_fired(position, entity_type);
			}
			is_shooting = false;
		}
		//stuck here regarding scope and not wanting to spawn bullets just to make it work



		std::vector<Easys::Entity> bullets = ecs.getEntitiesByComponent<BulletDirection>();
		for (Easys::Entity bullet : bullets) {
			BulletDirection bullet_direction = ecs.getComponent<BulletDirection>(bullet);
			Position& bullet_position = ecs.getComponent<Position>(bullet); //got stuck multiple times because I wanted to access non initialized/added components. what is the standard way of doing things in this case. (try and catch blocks?)
			if (bullet_direction == BulletDirection::Up) {
				bullet_position.y -= PIXEL_SCALE; //speed to be rethought maybe, depends how it will tie into timing on a whole.
			}
			else if (bullet_direction == BulletDirection::Down) {
				bullet_position.y += PIXEL_SCALE; //speed to be rethought maybe, depends how it will tie into timing on a whole.
			}
		}
		return bullets;
	}

private:

	Easys::Entity shots_fired(Position position, EntityType entity_type) {//feeding components instead of entities into this function, what is better?, Using no reference here as I am not updating position
		Easys::Entity bullet = ecs.addEntity();
		if (entity_type == 1) {
			Position bullet_spawn_position{ position.x + 3 * PIXEL_SCALE, position.y };
			BulletDirection bullet_direction = BulletDirection::Up;
			ecs.addComponent<Position>(bullet, bullet_spawn_position);
			ecs.addComponent<BulletDirection>(bullet, bullet_direction);
		}
		else if (entity_type == 0) {
			Position bullet_spawn_position{ position.x + 3 * PIXEL_SCALE, position.y + 7 * PIXEL_SCALE };
			BulletDirection bullet_direction = BulletDirection::Down;
			ecs.addComponent<Position>(bullet, bullet_spawn_position);
			ecs.addComponent<BulletDirection>(bullet, bullet_direction);
		}

		return bullet;
	}

};


class CollisionDetectionSystem {

public:

	void update() {
		bool collision = false;
		std::vector<Easys::Entity> bodies;
		std::vector<Easys::Entity> bullets = ecs.getEntitiesByComponent<BulletDirection>();
		std::set<Easys::Entity> bodies_all = ecs.getEntities();
		for (Easys::Entity body : bodies_all) {
			if (ecs.hasComponent<Alive>(body) && ecs.hasComponent<Position>(body)) {
				Alive& alive = ecs.getComponent<Alive>(body);
				if (alive) {
					bodies.push_back(body);
				}
			}
		}

		for (Easys::Entity body : bodies) {
			Position& body_pos = ecs.getComponent<Position>(body);
			for (Easys::Entity bullet : bullets) {
				Position& bullet_pos = ecs.getComponent<Position>(bullet);
				if ((body_pos.x + (PIXEL_SCALE * 8)) >= bullet_pos.x && body_pos.x <= (bullet_pos.x + (PIXEL_SCALE * 2)) && body_pos.y + (PIXEL_SCALE * 7) >= bullet_pos.y && body_pos.y <= bullet_pos.y + (PIXEL_SCALE * 3)) {//will do size component next time
					EntityType& ent_type = ecs.getComponent<EntityType>(body);
					BulletDirection& bullet_dir = ecs.getComponent<BulletDirection>(bullet);
					if (ent_type == 0 && bullet_dir == BulletDirection::Up) {
						bullet_pos = { -16.0f, -16.0f }; //since we move the bullet out of bounds it will despawn in cleanup function
						Alive& alive = ecs.getComponent<Alive>(body);
						alive = false;
					}
					else if (ent_type == 1 && bullet_dir == BulletDirection::Down) {
						bullet_pos = { -16.0f, -16.0f };
						Alive& alive = ecs.getComponent<Alive>(body);
						alive = false;
					}
				}
			}
		}
	};
};

class GumbelManagementSystem {

public:

	std::vector<std::vector<Easys::Entity>> update(int& gumbel_counter, std::vector<std::vector<Easys::Entity>>& gumbel_army, FrameCounter& frame_counter) {
		//Spawning new gumbels
		if (gumbel_counter == 0) {
			gumbel_army = spawn_gumbels(gumbel_counter);
		}

		//accessing top row for movement and shooting
		std::vector<Easys::Entity>gumbel_top_row = gumbel_army[0];
		if (frame_counter.frame_count % 60 == 0 || frame_counter.frame_count % 40 == 0) {
			for (Easys::Entity gumbel : gumbel_top_row) {
				Shooting& possible_shooter = ecs.getComponent<Shooting>(gumbel);
				Position& possible_shooter_pos = ecs.getComponent<Position>(gumbel);
				std::vector<Easys::Entity> filter_for_player = ecs.getEntitiesByComponent<EntityType>();
				for (Easys::Entity to_be_filtered : filter_for_player) {
					EntityType& ent_type = ecs.getComponent<EntityType>(to_be_filtered);
					if (ent_type == 1) {
						Position& player_pos = ecs.getComponent<Position>(to_be_filtered);
						if (possible_shooter_pos.x + 40.00f >= player_pos.x && possible_shooter_pos.x - 40.00f <= player_pos.x) {
							possible_shooter.is_shooting = true;
						}
					}
				}
			}
		}
		//moving gumbels
		int leftmost_gumbel_index = 0;
		int rightmost_gumbel_index = (gumbel_top_row.size() - 1);
		Position& leftmost_gumbel_pos = ecs.getComponent<Position>(gumbel_top_row[leftmost_gumbel_index]);
		Position& rightmost_gumbel_pos = ecs.getComponent<Position>(gumbel_top_row[rightmost_gumbel_index]);
		GumbelDirection& gumbel_direction = ecs.getComponent<GumbelDirection>(gumbel_top_row[rightmost_gumbel_index]);
		if (gumbel_direction == GumbelDirection::Right && rightmost_gumbel_pos.x == float(640 * 2 - PIXEL_SCALE * 8)) {
			for (std::vector<Easys::Entity> gumbel_row : gumbel_army) {
				for (Easys::Entity gumbel : gumbel_row) {
					Position& gumbel_pos = ecs.getComponent<Position>(gumbel);
					GumbelDirection& gumbel_direction = ecs.getComponent<GumbelDirection>(gumbel);
					gumbel_pos.y += PIXEL_SCALE * 8;
					gumbel_direction = GumbelDirection::Left;
				}
			}
		}
		if (gumbel_direction == GumbelDirection::Left && leftmost_gumbel_pos.x == 0.00f) {
			for (std::vector<Easys::Entity> gumbel_row : gumbel_army) {
				for (Easys::Entity gumbel : gumbel_row) {
					Position& gumbel_pos = ecs.getComponent<Position>(gumbel);
					GumbelDirection& gumbel_direction = ecs.getComponent<GumbelDirection>(gumbel);
					gumbel_pos.y += PIXEL_SCALE * 8;
					gumbel_direction = GumbelDirection::Right;
				}
			}
		}


		return gumbel_army;
	};

private:

	std::vector<std::vector<Easys::Entity>> spawn_gumbels(int& gumbel_counter) {
		std::vector<std::vector<Easys::Entity>> gumbel_army;
		std::vector<Easys::Entity> gumbel_row;
		for (int outer_index = 0; outer_index < 4; outer_index++) { //hardcoding as this does not change
			for (int inner_index = 0; inner_index < 12; inner_index++) {
				Easys::Entity gumbel = ecs.addEntity();
				float position_x = 0.00f + inner_index * PIXEL_SCALE * 8 + PIXEL_SCALE * 4 * inner_index;
				float position_y = 32.00f + outer_index * PIXEL_SCALE * 8 + PIXEL_SCALE * 4 * outer_index;
				ecs.addComponent<Position>(gumbel, { position_x , position_y });
				ecs.addComponent<Alive>(gumbel, { true });
				ecs.addComponent<Movement>(gumbel, { (float)PIXEL_SCALE });
				ecs.addComponent<EntityType>(gumbel, { 0 });
				ecs.addComponent<GumbelDirection>(gumbel, { GumbelDirection::Right });
				if (outer_index == 0) {
					ecs.addComponent<Shooting>(gumbel, { false });
				}
				gumbel_row.push_back(gumbel);
				gumbel_counter++;
			}
			gumbel_army.push_back(gumbel_row);
			gumbel_row = {};
		}
		return gumbel_army;
	};
};

//not used in in final version
class SoundSystem {

public:

};

class AnimationAndRenderingSystem {

public:
	AnimationAndRenderingSystem() {
		//having to init here as we now store font to get rid of flimmering, hacky fix
		if (TTF_Init() == -1) {
			std::cout << "Error intializing TTf: " << TTF_GetError() << "\n";
		}
		font = TTF_OpenFont("../assets/sixtyfour_convergence_regular.ttf", 48);
		if (!font) {
			std::cout << "Failed to load font: " << TTF_GetError() << "\n";
		}
	}

	void update(Easys::Entity& spaceship, std::vector<std::vector<Easys::Entity>>& gumbel_army,
		std::vector<Easys::Entity>& bullets, Uint64 delta_time) {

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		render_spaceship(spaceship);
		render_gumbels(gumbel_army, delta_time);

		for (Easys::Entity bullet : bullets) {
			BulletDirection direction = ecs.getComponent<BulletDirection>(bullet);
			if (direction != BulletDirection::None) {
				render_bullets(bullet);
			}
		}

		std::vector<Easys::Entity> possible_casualties = ecs.getEntitiesByComponent<Alive>();
		for (Easys::Entity possible_casualty : possible_casualties) {
			Alive& alive = ecs.getComponent<Alive>(possible_casualty);
			if (!alive && ecs.hasComponent<Cooldown>(possible_casualty)) {
				render_death(possible_casualty, delta_time);
			}
		}


		// Get current score and lives
		std::vector<Easys::Entity> player_filter = ecs.getEntitiesByComponent<Score>();
		Easys::Entity& player_ship = player_filter[0];
		Score& score = ecs.getComponent<Score>(player_ship);
		Lives& lives = ecs.getComponent<Lives>(player_ship);

		if (score.score != cached_score) {
			if (score_texture) SDL_DestroyTexture(score_texture);

			std::string score_text = "SCORE|" + std::to_string(score.score);
			SDL_Color color = { 255, 255, 255 };
			SDL_Surface* scoreboard = TTF_RenderText_Solid(font, score_text.c_str(), color);

			if (scoreboard) {
				score_texture = SDL_CreateTextureFromSurface(renderer, scoreboard);
				SDL_FreeSurface(scoreboard);
			}
			cached_score = score.score;
		}

		if (lives.lives != cached_lives) {
			if (lives_texture) SDL_DestroyTexture(lives_texture);

			std::string lives_text = "LIVES|" + std::to_string(lives.lives);
			SDL_Color color = { 255, 255, 255 };
			SDL_Surface* health_bar = TTF_RenderText_Solid(font, lives_text.c_str(), color);

			if (health_bar) {
				lives_texture = SDL_CreateTextureFromSurface(renderer, health_bar);
				SDL_FreeSurface(health_bar);
			}
			cached_lives = lives.lives;
		}

		// Render the cached textures
		if (score_texture) {
			int w, h;
			SDL_QueryTexture(score_texture, NULL, NULL, &w, &h);
			SDL_Rect dest_score = { 0, 960, w, h };
			SDL_RenderCopy(renderer, score_texture, NULL, &dest_score);
		}

		if (lives_texture) {
			int w, h;
			SDL_QueryTexture(lives_texture, NULL, NULL, &w, &h);
			SDL_Rect dest_lives = { 940, 960, w, h };
			SDL_RenderCopy(renderer, lives_texture, NULL, &dest_lives);
		}

		SDL_RenderPresent(renderer);
	}

private:

	//for font texture caching
	TTF_Font* font = nullptr;
	SDL_Texture* score_texture = nullptr;
	SDL_Texture* lives_texture = nullptr;
	int cached_score = -1;
	int cached_lives = -1;

	void render_spaceship(Easys::Entity& spaceship) {
		SDL_Rect front;
		SDL_Rect cockpit;
		SDL_Rect body;
		SDL_Rect engine;
		SDL_Rect cockpit_window;
		SDL_Rect left_exhaust;
		SDL_Rect right_exhaust;

		Position& pos = ecs.getComponent<Position>(spaceship);
		Lives& player_lives = ecs.getComponent<Lives>(spaceship);


		front.x = pos.x + PIXEL_SCALE * 3;
		front.y = pos.y;
		front.h = PIXEL_SCALE;
		front.w = PIXEL_SCALE * 2;

		cockpit.x = pos.x + PIXEL_SCALE * 2;
		cockpit.y = pos.y + PIXEL_SCALE;
		cockpit.h = PIXEL_SCALE * 2;
		cockpit.w = PIXEL_SCALE * 4;

		body.x = pos.x + PIXEL_SCALE;
		body.y = pos.y + PIXEL_SCALE * 3;
		body.h = PIXEL_SCALE * 2;
		body.w = PIXEL_SCALE * 6;

		engine.x = pos.x;
		engine.y = pos.y + PIXEL_SCALE * 5;
		engine.h = PIXEL_SCALE * 3;
		engine.w = PIXEL_SCALE * 8;

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

		if (player_lives.lives == 3) {
			SDL_RenderFillRect(renderer, &front);
		}
		SDL_RenderFillRect(renderer, &cockpit);
		SDL_RenderFillRect(renderer, &body);
		if (player_lives.lives >= 2) {
			SDL_RenderFillRect(renderer, &engine);
		}


		cockpit_window.x = pos.x + PIXEL_SCALE * 3;
		cockpit_window.y = pos.y + PIXEL_SCALE * 2;
		cockpit_window.h = PIXEL_SCALE;
		cockpit_window.w = PIXEL_SCALE * 2;

		left_exhaust.x = pos.x + PIXEL_SCALE;
		left_exhaust.y = pos.y + PIXEL_SCALE * 7;
		left_exhaust.h = PIXEL_SCALE;
		left_exhaust.w = PIXEL_SCALE * 2;

		right_exhaust.x = pos.x + PIXEL_SCALE * 5;
		right_exhaust.y = pos.y + PIXEL_SCALE * 7;
		right_exhaust.h = PIXEL_SCALE;
		right_exhaust.w = PIXEL_SCALE * 2;

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderFillRect(renderer, &cockpit_window);
		SDL_RenderFillRect(renderer, &left_exhaust);
		SDL_RenderFillRect(renderer, &right_exhaust);
	}

	void render_gumbels(std::vector<std::vector<Easys::Entity>>& gumbel_army, Uint64 delta_time) {
		std::vector<Easys::Entity> entities_with_cooldown = ecs.getEntitiesByComponent<Cooldown>();
		for (Easys::Entity entity : entities_with_cooldown) {
			if (!ecs.hasComponent<Alive>(entity)) {
				Cooldown& cooldown = ecs.getComponent<Cooldown>(entity);
				for (std::vector<Easys::Entity> gumbel_row : gumbel_army) {
					for (Easys::Entity gumbel : gumbel_row) {
						if (cooldown.cooldown >= 3000) {
							render_gumbels_frame_one(gumbel);
						}
						else if (cooldown.cooldown >= 2000 && cooldown.cooldown < 3000) {
							render_gumbels_frame_two(gumbel);
						}
						else if (cooldown.cooldown >= 1000 && cooldown.cooldown < 2000) {
							render_gumbels_frame_one(gumbel);
						}
						else if (cooldown.cooldown < 1000 && cooldown.cooldown >= 0) {
							render_gumbels_frame_three(gumbel);
						}
						else {
							render_gumbels_frame_one(gumbel);
							cooldown.cooldown = 4000;
						}
					}
				}
				cooldown.cooldown -= delta_time;
			}
		}
	}

	void render_gumbels_frame_one(Easys::Entity& gumbel) {
		SDL_Rect outline;
		SDL_Rect left_eye;
		SDL_Rect right_eye;
		SDL_Rect left_leg;
		SDL_Rect right_leg;
		Position& pos = ecs.getComponent<Position>(gumbel);

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

		outline.x = pos.x;
		outline.y = pos.y;
		outline.h = PIXEL_SCALE * 8;
		outline.w = PIXEL_SCALE * 8;

		SDL_RenderFillRect(renderer, &outline);

		left_eye.x = pos.x + (PIXEL_SCALE * 2);
		left_eye.y = pos.y + PIXEL_SCALE;
		left_eye.h = PIXEL_SCALE * 2;
		left_eye.w = PIXEL_SCALE;


		right_eye.x = pos.x + (PIXEL_SCALE * 5);
		right_eye.y = pos.y + PIXEL_SCALE;
		right_eye.h = PIXEL_SCALE * 2;
		right_eye.w = PIXEL_SCALE;

		left_leg.x = pos.x + (PIXEL_SCALE * 2);
		left_leg.y = pos.y + (PIXEL_SCALE * 5);
		left_leg.h = PIXEL_SCALE * 3;
		left_leg.w = PIXEL_SCALE;

		right_leg.x = pos.x + (PIXEL_SCALE * 5);
		right_leg.y = pos.y + (PIXEL_SCALE * 5);
		right_leg.h = PIXEL_SCALE * 3;
		right_leg.w = PIXEL_SCALE;


		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderFillRect(renderer, &left_eye);
		SDL_RenderFillRect(renderer, &right_eye);
		SDL_RenderFillRect(renderer, &left_leg);
		SDL_RenderFillRect(renderer, &right_leg);

	}

	void render_gumbels_frame_two(Easys::Entity& gumbel) {
		SDL_Rect outline;
		SDL_Rect left_eye;
		SDL_Rect right_eye;
		SDL_Rect left_leg;
		SDL_Rect mid_leg;
		SDL_Rect right_leg;
		Position& pos = ecs.getComponent<Position>(gumbel);

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

		outline.x = pos.x;
		outline.y = pos.y;
		outline.h = PIXEL_SCALE * 8;
		outline.w = PIXEL_SCALE * 8;

		SDL_RenderFillRect(renderer, &outline);

		left_eye.x = pos.x + (PIXEL_SCALE * 2);
		left_eye.y = pos.y + (PIXEL_SCALE * 2);
		left_eye.h = PIXEL_SCALE;
		left_eye.w = PIXEL_SCALE;


		right_eye.x = pos.x + (PIXEL_SCALE * 5);
		right_eye.y = pos.y + (PIXEL_SCALE * 2);
		right_eye.h = PIXEL_SCALE;
		right_eye.w = PIXEL_SCALE;

		left_leg.x = pos.x + PIXEL_SCALE;
		left_leg.y = pos.y + (PIXEL_SCALE * 5);
		left_leg.h = PIXEL_SCALE * 3;
		left_leg.w = PIXEL_SCALE;

		mid_leg.x = pos.x + (PIXEL_SCALE * 4);
		mid_leg.y = pos.y + (PIXEL_SCALE * 5);
		mid_leg.h = PIXEL_SCALE * 3;
		mid_leg.w = PIXEL_SCALE;

		right_leg.x = pos.x + (PIXEL_SCALE * 7);
		right_leg.y = pos.y + (PIXEL_SCALE * 5);
		right_leg.h = PIXEL_SCALE * 3;
		right_leg.w = PIXEL_SCALE;


		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderFillRect(renderer, &left_eye);
		SDL_RenderFillRect(renderer, &right_eye);
		SDL_RenderFillRect(renderer, &left_leg);
		SDL_RenderFillRect(renderer, &mid_leg);
		SDL_RenderFillRect(renderer, &right_leg);

	}

	void render_gumbels_frame_three(Easys::Entity& gumbel) {
		SDL_Rect outline;
		SDL_Rect left_eye;
		SDL_Rect right_eye;
		SDL_Rect left_leg;
		SDL_Rect mid_leg;
		SDL_Rect right_leg;
		Position& pos = ecs.getComponent<Position>(gumbel);

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

		outline.x = pos.x;
		outline.y = pos.y;
		outline.h = PIXEL_SCALE * 8;
		outline.w = PIXEL_SCALE * 8;

		SDL_RenderFillRect(renderer, &outline);

		left_eye.x = pos.x + (PIXEL_SCALE * 2);
		left_eye.y = pos.y + (PIXEL_SCALE * 2);
		left_eye.h = PIXEL_SCALE;
		left_eye.w = PIXEL_SCALE;


		right_eye.x = pos.x + (PIXEL_SCALE * 5);
		right_eye.y = pos.y + (PIXEL_SCALE * 2);
		right_eye.h = PIXEL_SCALE;
		right_eye.w = PIXEL_SCALE;

		left_leg.x = pos.x;
		left_leg.y = pos.y + (PIXEL_SCALE * 5);
		left_leg.h = PIXEL_SCALE * 3;
		left_leg.w = PIXEL_SCALE;

		mid_leg.x = pos.x + (PIXEL_SCALE * 3);
		mid_leg.y = pos.y + (PIXEL_SCALE * 5);
		mid_leg.h = PIXEL_SCALE * 3;
		mid_leg.w = PIXEL_SCALE;

		right_leg.x = pos.x + (PIXEL_SCALE * 6);
		right_leg.y = pos.y + (PIXEL_SCALE * 5);
		right_leg.h = PIXEL_SCALE * 3;
		right_leg.w = PIXEL_SCALE;


		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderFillRect(renderer, &left_eye);
		SDL_RenderFillRect(renderer, &right_eye);
		SDL_RenderFillRect(renderer, &left_leg);
		SDL_RenderFillRect(renderer, &mid_leg);
		SDL_RenderFillRect(renderer, &right_leg);
	}

	void render_bullets(Easys::Entity& bullet) {
		SDL_Rect bulletrect;
		Position& pos = ecs.getComponent<Position>(bullet);
		BulletDirection& dir = ecs.getComponent<BulletDirection>(bullet);
		if (dir != BulletDirection::None) {
			bulletrect.x = pos.x;
			bulletrect.y = pos.y;
			bulletrect.h = PIXEL_SCALE * 3;
			bulletrect.w = PIXEL_SCALE * 2;

			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderFillRect(renderer, &bulletrect);
		}
	}

	void render_death(Easys::Entity& dead_body, Uint64& delta_time) {
		SDL_Rect top_left;
		SDL_Rect top_right;
		SDL_Rect left_top_inter;
		SDL_Rect right_top_inter;
		SDL_Rect center;
		SDL_Rect bottom_left;
		SDL_Rect bottom_right;
		SDL_Rect left_bottom_inter;
		SDL_Rect right_bottom_inter;
		Position& pos = ecs.getComponent<Position>(dead_body);
		Cooldown& cooldown = ecs.getComponent<Cooldown>(dead_body);



		top_left.x = pos.x;
		top_left.y = pos.y;
		top_left.h = PIXEL_SCALE * 2;
		top_left.w = PIXEL_SCALE * 2;

		top_right.x = pos.x + PIXEL_SCALE * 6;
		top_right.y = pos.y;
		top_right.h = PIXEL_SCALE * 2;
		top_right.w = PIXEL_SCALE * 2;

		if (cooldown.cooldown > 20) {
			right_top_inter.x = pos.x + PIXEL_SCALE * 5;
			right_top_inter.y = pos.y + PIXEL_SCALE;
			right_top_inter.h = PIXEL_SCALE * 2;
			right_top_inter.w = PIXEL_SCALE * 2;
		}

		if (cooldown.cooldown > 20) {
			left_top_inter.x = pos.x + PIXEL_SCALE;
			left_top_inter.y = pos.y + PIXEL_SCALE;
			left_top_inter.h = PIXEL_SCALE * 2;
			left_top_inter.w = PIXEL_SCALE * 2;
		}

		if (cooldown.cooldown > 40) {
			center.x = pos.x + PIXEL_SCALE * 2;
			center.y = pos.y + PIXEL_SCALE * 2;
			center.h = PIXEL_SCALE * 4;
			center.w = PIXEL_SCALE * 4;
		}

		if (cooldown.cooldown > 20) {
			left_bottom_inter.x = pos.x + PIXEL_SCALE;
			left_bottom_inter.y = pos.y + PIXEL_SCALE * 5;
			left_bottom_inter.h = PIXEL_SCALE * 2;
			left_bottom_inter.w = PIXEL_SCALE * 2;
		}

		if (cooldown.cooldown > 20) {
			right_bottom_inter.x = pos.x + PIXEL_SCALE * 5;
			right_bottom_inter.y = pos.y + PIXEL_SCALE * 5;
			right_bottom_inter.h = PIXEL_SCALE * 2;
			right_bottom_inter.w = PIXEL_SCALE * 2;

		}

		bottom_left.x = pos.x;
		bottom_left.y = pos.y + PIXEL_SCALE * 6;
		bottom_left.h = PIXEL_SCALE * 2;
		bottom_left.w = PIXEL_SCALE * 2;

		bottom_right.x = pos.x + PIXEL_SCALE * 6;
		bottom_right.y = pos.y + PIXEL_SCALE * 6;
		bottom_right.h = PIXEL_SCALE * 2;
		bottom_right.w = PIXEL_SCALE * 2;

		cooldown.cooldown -= delta_time;
		if (cooldown.cooldown <= 0 && !ecs.hasComponent<Lives>(dead_body)) {
			ecs.removeEntity(dead_body); //this causes issues and should not be hidden here
		}

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(renderer, &top_left);
		SDL_RenderFillRect(renderer, &top_right);
		SDL_RenderFillRect(renderer, &right_top_inter);
		SDL_RenderFillRect(renderer, &left_top_inter);
		SDL_RenderFillRect(renderer, &center);
		SDL_RenderFillRect(renderer, &left_bottom_inter);
		SDL_RenderFillRect(renderer, &right_bottom_inter);
		SDL_RenderFillRect(renderer, &bottom_left);
		SDL_RenderFillRect(renderer, &bottom_right);
	}

	void render_scoreboard() {


		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	};


};