#ifndef __MODULEFADETOBLACK_H__
#define __MODULEFADETOBLACK_H__

#include "Module.h"

class ModuleFadeToBlack : public Module
{
public:
	ModuleFadeToBlack();
	~ModuleFadeToBlack();

	bool Start();
	update_status Update();
	bool FadeToBlack(Module* module_off, Module* module_on, float time = 1.0f);
	bool IsFading() const;

private:

	enum fade_step
	{
		none,
		fade_to_black,
		fade_from_black
	} current_step = fade_step::none;

	Uint32 start_time = 0;
	Uint32 total_time = 0;
	SDL_Rect screen;
	Module* to_enable = nullptr;
	Module* to_disable = nullptr;
};

#endif //__MODULEFADETOBLACK_H__