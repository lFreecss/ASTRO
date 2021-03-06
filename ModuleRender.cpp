#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"

#ifdef _WIN32
	#include "SDL/include/SDL.h"
#else
	#include "SDL.h"
#endif

ModuleRender::ModuleRender() : Module()
{
	camera.x = camera.y = 0;
	camera.w = SCREEN_WIDTH;
	camera.h = SCREEN_HEIGHT;
}

// Destructor
ModuleRender::~ModuleRender()
{}

// Called before render is available
bool ModuleRender::Init()
{
	LOG("Creating Renderer context");
	bool ret = true;
	Uint32 flags = 0;

	if(REN_VSYNC == true)
	{
		flags |= SDL_RENDERER_PRESENTVSYNC;
	}

	renderer = SDL_GetRenderer(App->window->window);

	if(renderer == nullptr)
		renderer = SDL_CreateRenderer(App->window->window, -1, flags);
	
	if(renderer == NULL)
	{
		LOG("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		target = SDL_CreateTexture(
			renderer,
			SDL_GetWindowPixelFormat(App->window->window),
			SDL_TEXTUREACCESS_TARGET,
			SCREEN_WIDTH,
			SCREEN_HEIGHT);

		if (target != nullptr)
		{
			if (SDL_SetRenderTarget(renderer, target) != 0)
			{
				LOG("Could not set render target SDL_Error: %s\n", SDL_GetError());
				ret = false;
			}
			else
			{
				water = SDL_CreateTexture(
					renderer,
					SDL_GetWindowPixelFormat(App->window->window),
					SDL_TEXTUREACCESS_TARGET,
					SCREEN_WIDTH,
					WATER_TEX_HEIGHT);

				SDL_SetTextureColorMod(water, 100, 100, 255);
			}
		}

		if (SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT) != 0)
		{
			LOG("Could not set render logical size SDL_Error: %s\n", SDL_GetError());
			ret = false;
		}

		SDL_RenderSetScale(renderer, 0.5f, 0.5f);
	}

	return ret;
}

// Called every draw update
update_status ModuleRender::PreUpdate()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);

	return update_status::UPDATE_CONTINUE;
}

update_status ModuleRender::Update()	
{
	int speed = 3;
	
	if (App->input->keyboard[SDL_SCANCODE_LSHIFT] == KEY_STATE::KEY_REPEAT)
	{
		if (App->input->keyboard[SDL_SCANCODE_W] == KEY_STATE::KEY_REPEAT)
			camera.y += speed;

		if (App->input->keyboard[SDL_SCANCODE_S] == KEY_STATE::KEY_REPEAT)
			camera.y -= speed;

		if (App->input->keyboard[SDL_SCANCODE_A] == KEY_STATE::KEY_REPEAT)
			camera.x -= speed;

		if (App->input->keyboard[SDL_SCANCODE_D] == KEY_STATE::KEY_REPEAT)
			camera.x += speed;
	}

	return update_status::UPDATE_CONTINUE;
}

update_status ModuleRender::PostUpdate()
{
	FullScreenEffects();
	SDL_SetRenderTarget(renderer, NULL);
	SDL_RenderCopy(renderer, target, NULL, NULL);
	SDL_RenderPresent(renderer);
	SDL_SetRenderTarget(renderer, target);

	return update_status::UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleRender::CleanUp()
{
	LOG("Destroying renderer");
	
	SDL_DestroyTexture(water);
	SDL_DestroyTexture(target);

	//Destroy window
	if(renderer != nullptr)
	{
		SDL_DestroyRenderer(renderer);
	}

	return true;
}

// Blit to screen
bool ModuleRender::Blit(SDL_Texture* texture, int x, int y, SDL_Rect* section, float speed, bool use_camera, bool flip_horizontal)
{
	bool ret = true;
	SDL_Rect rect;

	if(use_camera)
	{
		rect.x = (int)(-camera.x * speed) + x * SCREEN_SIZE;
		rect.y = (int)(-camera.y * speed) + y * SCREEN_SIZE;
	}
	else
	{
		rect.x = x * SCREEN_SIZE;
		rect.y = y * SCREEN_SIZE;
	}

	if(section != NULL)
	{
		rect.w = section->w;
		rect.h = section->h;
	}
	else
	{
		SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	}

	rect.w *= SCREEN_SIZE;
	rect.h *= SCREEN_SIZE;

	int result = 0;
	
	if(flip_horizontal == true )
		result = SDL_RenderCopyEx(renderer, texture, section, &rect, 0.0, NULL, SDL_FLIP_HORIZONTAL);
	else
		result = SDL_RenderCopy(renderer, texture, section, &rect);
	
	if(result != 0)
	{
		LOG("Cannot blit to screen. SDL_RenderCopy error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool ModuleRender::DrawQuad(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera)
{
	bool ret = true;

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	SDL_Rect rec(rect);
	if(use_camera)
	{
		rec.x = (int)(-camera.x + rect.x * SCREEN_SIZE);
		rec.y = (int)(-camera.y + rect.y * SCREEN_SIZE);
	}
	else
	{
		rec.x *= SCREEN_SIZE;
		rec.y *= SCREEN_SIZE;
	}

	rec.w *= SCREEN_SIZE;
	rec.h *= SCREEN_SIZE;

	if(SDL_RenderFillRect(renderer, &rec) != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

void ModuleRender::FullScreenEffects()
{
	if(effect_water == true)
	{
		SDL_SetRenderTarget(renderer, water);
		SDL_Rect src = {0, SCREEN_HEIGHT - WATER_TEX_HEIGHT - WATER_HEIGHT, SCREEN_WIDTH, WATER_TEX_HEIGHT};
		SDL_RenderCopy(renderer, target, &src, NULL);

		SDL_SetRenderTarget(renderer, target);
		SDL_Rect dst = {0, SCREEN_HEIGHT - WATER_HEIGHT, SCREEN_WIDTH, WATER_HEIGHT};
		SDL_RenderCopyEx(renderer, water, NULL, &dst, 0, NULL, SDL_FLIP_VERTICAL);
	}
}