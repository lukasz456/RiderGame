#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include<windows.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define maxEnemies 5
#define maxAmmo 3
#define maxNpc 4

// narysowanie napisu txt na powierzchni screen, zaczynajπc od punktu (x, y)
// charset to bitmapa 128x128 zawierajπca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images

typedef struct Entity
{
	int x;
	int y;
	int Speed;
}Entity;

void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt úrodka obrazka sprite na ekranie
// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};


// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};


// rysowanie linii o d≥ugoúci l w pionie (gdy dx = 0, dy = 1) 
// bπdü poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};


// rysowanie prostokπta o d≥ugoúci bokÛw l i k
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};

void gameOver(char *text, SDL_Surface* screen, SDL_Surface *charset, int czerwony, int niebieski, double worldTime, SDL_Renderer* renderer, SDL_Texture* scrtex, char *info, double score)
{
	DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
	sprintf(text, "The end!, time: %.1lf s,score: %.0lf", worldTime, score);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
	DrawString(screen, screen->w / 2 - strlen(info) * 8 / 2, 26, info, charset);
	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
	Sleep(3000);
}

bool sideroad(int x, const int width)
{
	if (x <= 120 || x + width >= SCREEN_WIDTH - 120)
	{
		return true;
	}
	return false;
}

bool colision(int x1, int y1, const int width1, const int height1, int x2, int y2, const int width2, const int height2)
{
	if (x1 < x2 + width2 && x1 + width1 > x2 && y1 < y2 + height2 && height1 + y1 > y2)
		return true;

	return false;
}

void newGame(int *playerX, int *playerY, Entity enemies[maxEnemies], double* score, double* worldTime, int *t1, Entity npc[maxNpc], Entity bullets[maxAmmo])
{
	*playerX = 300;
	*playerY = 300;
	*score = 0;
	*worldTime = 0;
	*t1 = SDL_GetTicks();

	for (int i = 0; i < maxEnemies; i++)
	{
		enemies[i] = { 0,0,0 };
	}

	for (int i = 0; i < maxAmmo; i++)
	{
		bullets[i] = { -100,-100,0 };
	}

	for (int i = 0; i < maxNpc; i++)
	{
		bullets[i] = { 0,0,0 };
	}
}

void timeStart(int *t1, int *quit, int *frames, int *rc, double *fpsTimer, double *fps, double *worldTime, double *distance, double *playerSpeed)
{
	*t1 = SDL_GetTicks();

	*frames = 0;
	*fpsTimer = 0;
	*fps = 0;
	*quit = 0;
	*worldTime = 0;
	*distance = 0;
	*playerSpeed = 5;
}

void timeSetting(double *delta, int *t1, int *t2, bool pause, double *worldTime, double *distance, double *playerSpeed)
{
	*t2 = SDL_GetTicks();

	// w tym momencie t2-t1 to czas w milisekundach,
	// jaki uplynal od ostatniego narysowania ekranu
	// delta to ten sam czas w sekundach
	// here t2-t1 is the time in milliseconds since
	// the last screen was drawn
	// delta is the same time in seconds
	*delta = (*t2 - *t1) * 0.001;
	*t1 = *t2;

	if (pause == false)
	{
		*worldTime += *delta;  
	}

	*distance += *playerSpeed * *delta;
}

void Scoring(double *score, bool pause, int *scoreBlock)
{
	static time_t seconds_score = time(NULL);
	
	if (seconds_score != time(NULL) && pause == false)
	{
		seconds_score = time(NULL);
		if (*scoreBlock == 0)
		{
			*score += 10;
		}
		else
		{
			*scoreBlock -= 1;
		}
	}

}

void mapScrolling(bool pause, int *roadOffset)
{
	if (pause == false)
	{
		(*roadOffset)++;
	}
	if (*roadOffset > 700)
	{
		*roadOffset = 0;
	}
}

void FpsManaging(double *fpsTimer, double *delta, int *frames, double *fps)
{
	*fpsTimer += *delta;
	if (*fpsTimer > 0.5) {
		*fps = *frames * 2;
		*frames = 0;
		*fpsTimer -= 0.5;
	};
}

void informationText(SDL_Surface* screen, int czerwony, int niebieski, char *text, double worldTime, double fps, SDL_Surface* charset, double score, SDL_Texture* scrtex, SDL_Renderer* renderer)
{
	// tekst informacyjny / info text
	DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
	//            "template for the second project, elapsed time = %.1lf s  %.0lf frames / s"
	sprintf(text, "Lukasz Kornacki 195613 , time = %.1lf s  %.0lf fps", worldTime, fps);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
	//	      "Esc - exit, \030 - faster, \031 - slower"
	sprintf(text, "Esc - exit, \030 - acceleration, \031 - slowing down, score: %.0lf", score);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);

	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	//		SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
}

SDL_Surface *imageLoad(const char* fileName, SDL_Texture* scrtex, SDL_Surface* screen, SDL_Window* window, SDL_Renderer* renderer)
{
	SDL_Surface* sprite;
	sprite = SDL_LoadBMP(fileName);
	if (sprite == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return NULL;
	}
	return sprite;
}

void eventManaging(SDL_Event *event, int *quit, int *playerX, int *playerY, int playerSpeed, bool *pause, Entity enemies[maxEnemies], double *score, double *worldTime, int *t1, Entity bullets[maxAmmo], Entity npc[maxNpc], const int carHeight, SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Surface* screen, SDL_Surface* charset, int czerwony, int niebieski, char *death_info, char *text)
{
	while (SDL_PollEvent(event)) {
		switch (event->type) {
		case SDL_KEYDOWN:
			if (*pause == false)
			{
				if (event->key.keysym.sym == SDLK_ESCAPE) *quit = 1;
				if (event->key.keysym.sym == SDLK_UP) *playerY -= playerSpeed;
				if (event->key.keysym.sym == SDLK_DOWN) *playerY += playerSpeed;
				if (event->key.keysym.sym == SDLK_RIGHT) *playerX += playerSpeed;
				if (event->key.keysym.sym == SDLK_LEFT) *playerX -= playerSpeed;
				if (event->key.keysym.sym == SDLK_n) newGame(playerX, playerY, enemies, score, worldTime, t1, bullets, npc);
				if (event->key.keysym.sym == SDLK_SPACE)
				{
					for (int i = 0; i < maxAmmo; i++)
					{
						if (bullets[i].Speed == 0)
						{
							bullets[i].x = *playerX;
							bullets[i].y = *playerY - carHeight / 2;
							bullets[i].Speed = 6;
							break;
						}
					}
				}
				if (event->key.keysym.sym == SDLK_f)
				{
					strcpy(death_info, "Gracz zakonczyl gre");
					gameOver(text, screen, charset, czerwony, niebieski, *worldTime, renderer, scrtex, death_info, *score);
					*quit = true;
				}
			}
			if (event->key.keysym.sym == SDLK_p)
			{
				*pause = !(*pause);
			}
			break;
		case SDL_KEYUP:

			break;
		case SDL_QUIT:
			*quit = 1;
			break;
		};
	};
}

//main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	int t1, t2, quit, frames, rc;
	double delta, worldTime, fpsTimer, fps, distance, playerSpeed;
	double delta_t;
	double score = 0;
	double buff;
	SDL_Event event;
	SDL_Surface *screen, *charset;
	SDL_Surface *player, *enemySprite, *road, *ammo, *NPC, *implement;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;
	int playerX = 300;
	int playerY = 300;
	int random;
	int scoreBlock = 0;
	int roadOffset = 0;
	const int carWidth = 30;
	const int carHeight = 50;
	const int bulletWidth = 4;
	const int bulletHeight = 16;
	Entity enemies[maxEnemies]; //enemies[0] = {100, 100, 1}
	Entity bullets[maxAmmo];
	Entity npc[maxNpc];
	for (int i = 0; i < maxAmmo; i++)
	{
		bullets[i] = { -100, -100, 0 };
	}
	time_t seconds_enemies = time(NULL);
	time_t seconds_npc = time(NULL);
	char death_info[128];
	bool pause = false;
	
	//enemies[0].x = 50
	// jesli auto ma speed 0 to nie istnieje
	// okno konsoli nie jest widoczne, jeøeli chcemy zobaczyÊ
	// komunikaty wypisywane printf-em trzeba w opcjach:
	// project -> szablon2 properties -> Linker -> System -> Subsystem
	// zmieniÊ na "Console"
	// console window is not visible, to see the printf output
	// the option:
	// project -> szablon2 properties -> Linker -> System -> Subsystem
	// must be changed to "Console"
	/*printf("wyjscie printfa trafia do tego okienka\n");
	printf("printf output goes here\n");*/

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
		}

	// tryb pe≥noekranowy / fullscreen mode
//	rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
//	                                 &window, &renderer);
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
	if(rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
		};
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "Szablon do zdania drugiego 2017");


	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
	                              0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
	                           SDL_TEXTUREACCESS_STREAMING,
	                           SCREEN_WIDTH, SCREEN_HEIGHT);


	// wy≥πczenie widocznoúci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	// wczytanie obrazka cs8x8.bmp
	charset = imageLoad("./cs8x8.bmp", scrtex, screen, window, renderer);
	if (charset == NULL) {
		return 1;
	};
	SDL_SetColorKey(charset, true, 0x000000);

	player = imageLoad("./car1.bmp", scrtex, screen, window, renderer);
	if (player == NULL) {
		return 1;
	};

	enemySprite = imageLoad("./car2.bmp", scrtex, screen, window, renderer);
	if (enemySprite == NULL) {
		return 1;
	};

	road = imageLoad("./road.bmp", scrtex, screen, window, renderer);
	if (road == NULL) {
		return 1;
	};

	ammo = imageLoad("./ammo.bmp", scrtex, screen, window, renderer);
	if (ammo == NULL) {
		return 1;
	};

	NPC = imageLoad("./car3.bmp", scrtex, screen, window, renderer);
	if (NPC == NULL) {
		return 1;
	};

	implement = imageLoad("./implement.bmp", scrtex, screen, window, renderer);
	if (implement == NULL) {
		return 1;
	};

	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	timeStart(&t1, &quit, &frames, &rc, &fpsTimer, &fps, &worldTime, &distance, &playerSpeed);

	while (!quit) {

		timeSetting(&delta, &t1, &t2, pause, &worldTime, &distance, &playerSpeed);
		Scoring(&score, pause, &scoreBlock);

		SDL_FillRect(screen, NULL, czarny); //change of background colour

		mapScrolling(pause, &roadOffset);
		DrawSurface(screen, road, 320, roadOffset);
		DrawSurface(screen, player, playerX, playerY);
		DrawSurface(screen, implement, 730, 500);

		if (sideroad(playerX, carWidth) == true)
		{
			strcpy(death_info, "You got off the road!");
			gameOver(text, screen, charset, czerwony, niebieski, worldTime, renderer, scrtex, death_info, score);
			newGame(&playerX, &playerY, enemies, &score, &worldTime, &t1, bullets, npc);
		}
		else
		{
			for (int i = 0; i < maxEnemies; i++)
			{
				if (colision(playerX, playerY, carWidth, carHeight, enemies[i].x, enemies[i].y, carWidth, carHeight))
				{
					strcpy(death_info, "Colision!");
					gameOver(text, screen, charset, czerwony, niebieski, worldTime, renderer, scrtex, death_info, score);
					newGame(&playerX, &playerY, enemies, &score, &worldTime, &t1, bullets, npc);
				}
			}

			for (int i = 0; i < maxNpc; i++)
			{
				if (colision(playerX, playerY, carWidth, carHeight, npc[i].x, npc[i].y, carWidth, carHeight))
				{
					npc[i] = { 0,0,0 };
					scoreBlock += 3;
				}
				else
				{
					for (int j = 0; j < maxEnemies; j++)
					{
						if (colision(enemies[j].x, enemies[j].y, carWidth, carHeight, npc[i].x, npc[i].y, carWidth, carHeight))
						{
							npc[i] = { 0,0,0 };
							enemies[j] = { 0,0,0 };
						}
					}
				}
			}
		}

		for (int i = 0; i < maxAmmo; i++)
		{
			for (int j = 0; j < maxEnemies; j++)
			{
				if (colision(enemies[j].x, enemies[j].y, carWidth, carHeight, bullets[i].x, bullets[i].y, carWidth, bulletHeight) && bullets[i].Speed != 0)
				{
					bullets[i] = { -100,-100,0 };
					enemies[j] = { 0,0,0 };
					score += 25;
				}
			}
		}
		for (int i = 0; i < maxAmmo; i++)
		{
			for (int j = 0; j < maxNpc; j++)
			{
				if (colision(npc[j].x, npc[j].y, carWidth, carHeight, bullets[i].x, bullets[i].y, carWidth, bulletHeight) && bullets[i].Speed != 0)
				{
					bullets[i] = { -100,-100,0 };
					npc[j] = { 0,0,0 };
					score -= 25;
				}
			}
		}

		for (int i = 0; i < maxEnemies; i++)
		{
			if (enemies[i].y < 0)
			{
				enemies[i] = { 0, 0, 0 };
			}
			else if (enemies[i].Speed != 0)
			{
				if (frames % 5 == 0 && pause == false)
				{
					enemies[i].y -= enemies[i].Speed;
				}
				DrawSurface(screen, enemySprite, enemies[i].x, enemies[i].y);
			}
			else if (seconds_enemies != time(NULL)) //max 1 car per second
			{
				seconds_enemies = time(NULL);
				srand(time(NULL));
				random = rand() % SCREEN_WIDTH / 2 + 1;

				if (random % 5 == 0)
				{
					if(pause == false)
					{
						enemies[i] = { (random % 40 + 1) * 10 + 130, SCREEN_HEIGHT, random % 4 + 1 };
					}
				}
			}
		}

		for (int i = 0; i < maxAmmo; i++)
		{
			if (bullets[i].y + bulletHeight < 0)
			{
				bullets[i] = { -100, -100, 0 };
			}
			else if (bullets[i].Speed != 0)
			{
				if (frames % 5 == 0 && pause == false)
				{
					bullets[i].y -= bullets[i].Speed;
				}
				DrawSurface(screen, ammo,bullets[i].x, bullets[i].y);
			}
		}

		for (int i = 0; i < maxNpc; i++)
		{
			if (npc[i].y > SCREEN_HEIGHT)
			{
				npc[i] = { 0, 0, 0 };
			}
			else if (npc[i].Speed != 0)
			{
				if (frames % 5 == 0 && pause == false)
				{
					npc[i].y += npc[i].Speed;
				}
				DrawSurface(screen, NPC, npc[i].x, npc[i].y);
			}
			else if (seconds_npc != time(NULL))
			{
				seconds_npc = time(NULL);
				srand(time(NULL));
				random = rand() % SCREEN_WIDTH / 2 + 1;

				if (random % 6 == 0)
				{
					if (pause == false)
					{
						buff = (random % 40 + 1) * 10 + 130;
						if (buff < 320)
						{
							buff += 200;
						}
						else
						{
							buff -= 200;
						}
						npc[i] = {(int)buff , 0, random % 4 + 1};
					}
				}
			}
		}

		FpsManaging(&fpsTimer, &delta, &frames, &fps);

		informationText(screen, czerwony, niebieski, text, worldTime, fps, charset, score, scrtex, renderer);

		// obs≥uga zdarzeÒ (o ile jakieú zasz≥y) / handling of events (if there were any)
		eventManaging(&event, &quit, &playerX, &playerY, playerSpeed, &pause, enemies, &score, &worldTime, &t1, bullets, npc, carHeight, scrtex, renderer, screen, charset, czerwony, niebieski, death_info, text);
		frames++;
};

	// zwolnienie powierzchni / freeing all surfaces
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
	};
