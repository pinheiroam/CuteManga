#ifndef CUTE_MANGA_H
#define CUTE_MANGA_H

#include <SDL.h>
#include <string>

// Game state used in main loop (view/scroll, cascade, zoom, etc.)
struct GameState {
	int basex = 0;
	int basey = 0;
	bool cascade = false;
	bool separation = false;
	bool adjust = true;
	int speed = 15;
	int zoom = 1;
	float neutralsize = 1.0f;
	int quit = 0;
	std::string foldermain;
	SDL_Color textColor = { 50, 50, 50 };
};

// Initialize SDL, window, renderer, image/ttf, MuPDF. Returns false on failure.
bool initSDLAndWindow();

// Load fonts and assets, set foldermain, create CuteManga dir on Switch. Returns false on failure.
bool initFontsAndAssets(std::string& foldermain);

// Populate arraymain / arraymain_is_dir from foldermain. Sets existfoldermain on failure.
void loadFolderList(const std::string& foldermain);

// Run one frame of deferred page load (if g_pending_page_load). Call after rendering.
void doDeferredPageLoad(const std::string& foldermain);

#endif /* CUTE_MANGA_H */
