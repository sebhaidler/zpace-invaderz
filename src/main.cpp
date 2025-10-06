#include "main.h"
#include "components.hpp"
#include "systems.hpp"


//initializing all systems globally
MovementSystem movement_system;
ShootingSystem shooting_system;
CollisionDetectionSystem collision_detection_system;
GumbelManagementSystem gumbel_management_system;
AnimationAndRenderingSystem animation_and_rendering_system;


bool init();

bool loop(Easys::Entity& spaceship, std::vector<std::vector<Easys::Entity>>& gumbel_army, FrameCounter& frame_counter, Uint64 delta_time, Mix_Chunk* laser, Mix_Chunk* death, Mix_Chunk* spaceship_explosion);

void gumbel_cleanup(std::vector<std::vector<Easys::Entity>>& gumbel_army, Mix_Chunk* death);
void player_cleanup(Easys::Entity& spaceship, Mix_Chunk* spaceship_explosion);

void kill();

//MAIN//
int main()
{
	Easys::Entity spaceship = ecs.addEntity();
	Easys::Entity blank_round = ecs.addEntity(); //my fix to make the shooting system work. ugly, but atleast this only spawns one blank and thus I accept it for now
	Easys::Entity animator = ecs.addEntity(); // I use an entity and a system for animation. let´s discuss that

	FrameCounter frame_counter;
	frame_counter.frame_count = 0;



	ecs.addComponent<EntityType>(spaceship, { 1 }); 
	ecs.addComponent<Position>(spaceship, { 0.0f, 880.0f });
	ecs.addComponent<Alive>(spaceship, { true });
	ecs.addComponent<Lives>(spaceship, { 3 });
	ecs.addComponent<Movement>(spaceship, { (float)PIXEL_SCALE });
	ecs.addComponent<Shooting>(spaceship, { false });
	ecs.addComponent<Cooldown>(spaceship, { 1000 });
	ecs.addComponent<Score>(spaceship, { 0 });

	std::vector<std::vector<Easys::Entity>> gumbel_army;
	int gumbel_counter = 0;
	gumbel_army = gumbel_management_system.update(gumbel_counter, gumbel_army, frame_counter);

	ecs.addComponent<Position>(blank_round, { 1000.0f, 1000.0f });
	ecs.addComponent<BulletDirection>(blank_round, { BulletDirection::None });

	ecs.addComponent<Cooldown>(animator, { 4000 });

	if (!init()) return 1;

	//just here for testing, to be moved to audio system
	Mix_Music* music = NULL;
	Mix_Chunk* laser = NULL;
	Mix_Chunk* death = NULL;
	Mix_Chunk* spaceship_explosion = NULL;

	music = Mix_LoadMUS("../assets/background_music.wav");
	if (!music) {
		std::cout << "Failed to load background music! Mix_Error: " << Mix_GetError() << "\n";

	}
	Mix_FadeInMusic(music, -1, 700);

	laser = Mix_LoadWAV("../assets/laser_blast.mp3");
	if (!laser) {
		std::cout << "Failed to load chunk! Mix_Error: " << Mix_GetError() << "\n";

	}

	death = Mix_LoadWAV("../assets/death_sound_effect.wav");
	if (!death) {
		std::cerr << "Failed to load chunk! Mix_Error: " << Mix_GetError() << "\n";
	}

	spaceship_explosion = Mix_LoadWAV("../assets/spaceship_explosion.mp3");
	if (!spaceship_explosion) {
		std::cerr << "Failed to load chunk! Mix_Error: " << Mix_GetError() << "\n";
	}





	bool running = true;
	Uint64 delta_time = 0;
	while (running) {
		//FPS Calculation taken from thenumb SDL2 tutorial
		Uint64 elapsed_time_start = SDL_GetTicks64();
		running = loop(spaceship, gumbel_army, frame_counter, delta_time, laser, death, spaceship_explosion);
		gumbel_counter = 0;
		for (std::vector<Easys::Entity>& gumbel_row : gumbel_army) {
			gumbel_counter += gumbel_row.size();
		}
		gumbel_army = gumbel_management_system.update(gumbel_counter, gumbel_army, frame_counter);

		//FPS Capping


		SDL_Delay(16.666f);
		Uint64 elapsed_time_end = SDL_GetTicks64();
		delta_time = elapsed_time_end - elapsed_time_start;

		std::cout << "Current FPS: " << 1000 / (16.666f - (delta_time / 1000)) << "\n";
		std::cout << "Delta Time: " << delta_time << "\n";

		frame_counter.frame_count++;
		if (frame_counter.frame_count == 60) {
			frame_counter.frame_count = 0;
		}
		std::cout << "Frame Count: " << frame_counter.frame_count << "\n";

	}
	return 0;
}

bool init() {

	if (SDL_Init(SDL_INIT_EVERYTHING | SDL_INIT_AUDIO) < 0) {
		std::cout << "Error initializing SDL: " << SDL_GetError() << "\n"; //Gets appropiate Error via SDL
		return false; // returns false and then the main function returns false for the program.
	}
	window = SDL_CreateWindow("Zpace Invaderz", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640 * 2, 512 * 2, 0); //Since the Window is the full application size what does the POS do, place it somewhere on the screen on startup?
	if (!window) {
		std::cout << "Error initializing Window: " << SDL_GetError() << "\n";
		return false;
	}



	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); //hardware acceleration, setting was shown in tutorial
	if (!renderer) {
		std::cout << "Error initializing Renderer: " << SDL_GetError() << "\n";
		return false;
	}

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); //RGB and Alpha value
	SDL_RenderClear(renderer);

	//Initialize SDL Mixer
	if (Mix_OpenAudio(44100, AUDIO_F32SYS, 2, 4096) < 0) {
		std::cout << "Error initializing SDL_Mixer: " << Mix_GetError() << "\n";
		return false;
	}

	return true;
}

bool loop(Easys::Entity& spaceship, std::vector<std::vector<Easys::Entity>>& gumbel_army, FrameCounter& frame_counter, Uint64 delta_time, Mix_Chunk* laser, Mix_Chunk* death, Mix_Chunk* spaceship_explosion) {
	SDL_Event event;
	const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

	PlayerController input = PlayerController::None;
	PlayerController shooting_input = PlayerController::None;

	if (currentKeyStates[SDL_SCANCODE_A]) {
		input = PlayerController::Left;
	}
	if (currentKeyStates[SDL_SCANCODE_D]) {
		input = PlayerController::Right;
	}
	if (currentKeyStates[SDL_SCANCODE_SPACE]) {
		shooting_input = PlayerController::Shoot;
	}

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			return false;
		}
	}


	//PHYSICS LOOP

	movement_system.update(input, frame_counter);

	Cooldown& cooldown = ecs.getComponent<Cooldown>(spaceship);
	cooldown.cooldown -= delta_time;
	if (shooting_input == PlayerController::Shoot && cooldown.cooldown <= 0) {
		Shooting& is_shooting = ecs.getComponent<Shooting>(spaceship);
		is_shooting = true;
		cooldown.cooldown = 1000;
		Mix_PlayChannel(-1, laser, 0);
	}

	std::vector<Easys::Entity> bullets = shooting_system.update();

	collision_detection_system.update();


	//RENDER LOOP	
	animation_and_rendering_system.update(spaceship, gumbel_army, bullets, delta_time);


	//CLEANUP OPERATIONS
	for (Easys::Entity bullet : bullets) {
		Position& bullet_position = ecs.getComponent<Position>(bullet);
		if (bullet_position.y >= 1024 || bullet_position.y <= 0) {
			ecs.removeEntity(bullet);
		}
	}
	gumbel_cleanup(gumbel_army, death);
	player_cleanup(spaceship, spaceship_explosion);


	return true;
}

void gumbel_cleanup(std::vector<std::vector<Easys::Entity>>& gumbel_army, Mix_Chunk* death) {
	for (std::vector<Easys::Entity>& gumbel_row : gumbel_army) {
		for (Easys::Entity gumbel : gumbel_row) {
			Alive& alive = ecs.getComponent<Alive>(gumbel);
			if (!alive.alive) {
				Position& gumbel_pos = ecs.getComponent<Position>(gumbel);
				Easys::Entity dead_body = ecs.addEntity();
				ecs.addComponent<Position>(dead_body, { gumbel_pos });
				ecs.addComponent<Alive>(dead_body, { false });
				ecs.addComponent<Cooldown>(dead_body, { 60 }); //by now cooldown should be renamed as it is managing multiple kinds of different things

				//SOUND TO BE MOVED TO ITS OWN SYSTEM
				Mix_PlayChannel(-1, death, 0);

				ecs.removeEntity(gumbel);
				remove(gumbel_row.begin(), gumbel_row.end(), gumbel);
				gumbel_row.pop_back();
				std::vector<Easys::Entity> player_filter = ecs.getEntitiesByComponent<Score>();
				Easys::Entity& player_ship = player_filter[0];
				Score& score = ecs.getComponent<Score>(player_ship);
				score.score += GUMBEL_VALUE;
			}
		}
	}
}

void player_cleanup(Easys::Entity& spaceship, Mix_Chunk* spaceship_explosion) {
	Alive& alive = ecs.getComponent<Alive>(spaceship);
	if (!alive.alive) {
		Lives& player_lives = ecs.getComponent<Lives>(spaceship);
		player_lives.lives--;
		Mix_PlayChannel(-1, spaceship_explosion, 0);

		if (player_lives.lives == 0) {
			std::cout << "************************GAME OVER************************";
			kill();
		}
		alive.alive = true;
	}
}

void kill() {
	// Quit
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	Mix_Quit();
	SDL_Quit();

}