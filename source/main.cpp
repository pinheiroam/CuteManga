#ifdef __SWITCH__
#include <unistd.h>
#include <switch.h>
#include <dirent.h>
#endif 
#ifndef __SWITCH__
#define _CRT_SECURE_NO_WARNINGS
#endif


//test

#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <algorithm>
#include <math.h> 

#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <cmath>
#include <iostream>
#include <math.h>
#include <vector>
#include <dirent.h>
#include <mupdf/fitz.h>

#include "cute_manga.h"

// Empty strings are invalid.
#ifdef __SWITCH__
SwkbdTextCheckResult Keyboard_ValidateText(char *string, size_t size) {
	if (strcmp(string, "") == 0) {
		strncpy(string, "The name cannot be empty.", size);
		return SwkbdTextCheckResult_Bad;
	}

	return SwkbdTextCheckResult_OK;
}

const char *Keyboard_GetText(const char *guide_text, const char *initial_text) {
	Result ret = 0;
	SwkbdConfig swkbd;
	static char input_string[256];

	if (R_FAILED(ret = swkbdCreate(&swkbd, 0))) {
		swkbdClose(&swkbd);
		return "";
	}

	swkbdConfigMakePresetDefault(&swkbd);

	if (strlen(guide_text) != 0)
		swkbdConfigSetGuideText(&swkbd, guide_text);

	if (strlen(initial_text) != 0)
		swkbdConfigSetInitialText(&swkbd, initial_text);

	swkbdConfigSetTextCheckCallback(&swkbd, Keyboard_ValidateText);

	if (R_FAILED(ret = swkbdShow(&swkbd, input_string, sizeof(input_string)))) {
		swkbdClose(&swkbd);
		return "";
	}

	swkbdClose(&swkbd);
	return input_string;
}

// Search: shows system keyboard as modal; allows empty (no validation).
const char* Keyboard_GetTextSearch(const char* guide_text, const char* initial_text) {
	Result ret = 0;
	SwkbdConfig swkbd;
	static char input_string[256];
	input_string[0] = '\0';
	if (R_FAILED(ret = swkbdCreate(&swkbd, 0))) {
		swkbdClose(&swkbd);
		return "";
	}
	swkbdConfigMakePresetDefault(&swkbd);
	if (guide_text && strlen(guide_text) != 0)
		swkbdConfigSetGuideText(&swkbd, guide_text);
	if (initial_text && strlen(initial_text) != 0)
		swkbdConfigSetInitialText(&swkbd, initial_text);
	if (R_FAILED(ret = swkbdShow(&swkbd, input_string, sizeof(input_string)))) {
		swkbdClose(&swkbd);
		return "";
	}
	swkbdClose(&swkbd);
	return input_string;
}
#endif 




bool	isFileExist(const char *file)
{
	struct stat	st = { 0 };

	if (stat(file, &st) == -1) {
		return (false);
	}

	return (true);
}

//////////////////////////////////aquï¿½ empieza el pc.
//Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
// Portrait mode: logical view size (swap when portrait); render to texture then rotate to screen
int g_view_w = SCREEN_WIDTH;
int g_view_h = SCREEN_HEIGHT;
bool g_portrait_mode = false;
SDL_Texture* g_portrait_texture = NULL;
enum states { selectmanga, readmanga };
int statenow = selectmanga;
std::string  urltodownload = "";
int porcendown = 0;
int sizeestimated = 0;
std::string temporallink = "";
bool ahorro = false;
int cancelcurl = 0;
bool preview = false;
int searchchapter = 0;
int selectchapter = 0;
int selectpage = 0;
float neutralsize = 1;
int widthnew = 0;
int heightnew = 0;
int cascademode = 0;
bool helppage = false;
bool existfoldermain = true;
int basexmain = 20;
int baseymain = 20;
bool cascadeactivated = false;
// Main-loop state (used by handleInput / renderFrame)
std::string g_foldermain;
int basex = 0;
int basey = 0;
bool cascade = false;
bool separation = false;
bool adjust = true;
int speed = 15;
int zoom = 1;
int quit = 0;
SDL_Color textColor = { 50, 50, 50 };
//Texture wrapper class
class LTexture
{
public:
	//Initializes variables
	LTexture();

	//Deallocates memory
	~LTexture();

	//Loads image at specified path
	bool loadFromFile(std::string path);
	bool loadFromFileCustom(std::string path,int h, int w);
	//Loads image from memory (for CBZ/ZIP archives)
	bool loadFromMemory(const void* buf, int size);
	//Loads from raw RGB/RGBA (e.g. MuPDF pixmap: data, w, h, stride, n=3 or 4)
	bool loadFromRGB(const unsigned char* data, int w, int h, int stride, int n);
	//Creates image from font string
	bool loadFromRenderedText(TTF_Font *fuente, std::string textureText, SDL_Color textColor);
	//Creates image from font string
	bool loadFromRenderedTextWrap(TTF_Font *fuente, std::string textureText, SDL_Color textColor, Uint32 size);
	//Deallocates texture
	void free();

	//Set color modulation
	void setColor(Uint8 red, Uint8 green, Uint8 blue);

	//Set blending
	void setBlendMode(SDL_BlendMode blending);

	//Set alpha modulation
	void setAlpha(Uint8 alpha);

	//Renders texture at given point
	void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);
	//Renders texture at given point
	void customrender(int x, int y,int w,int h, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);

	//Gets image dimensions
	int getWidth();
	int getHeight();

private:
	//The actual hardware texture
	SDL_Texture* mTexture;

	//Image dimensions
	int mWidth;
	int mHeight;
};

//Frees media and shuts down SDL
void close();

// When true, footer shows "Loading" and we load the page after the frame
bool g_pending_page_load = false;

// CBZ/CBR open failure reason for UI
enum { CBZ_ERR_NONE = 0, CBZ_ERR_FILE_NOT_FOUND = 1, CBZ_ERR_NOT_ZIP = 2, CBZ_ERR_NOT_RAR = 3 };
int g_cbz_open_error = CBZ_ERR_NONE;
std::string g_cbz_mupdf_error;  // MuPDF error message when CBZ_ERR_NOT_ZIP
std::string g_cbz_open_error_msg;  // Message to show in selectmanga footer on open failure

#ifdef CBR_SUPPORT
#include <unarr.h>
#endif

void renderLoadingScreen();  // Full-screen "Loading" then present (for CBZ open)

#ifdef __SWITCH__
void handleSwitchInput(u64 kDown, u64 kHeld);
#endif
void handlePCInput(SDL_Event& e);
void renderFrame();

// MuPDF: context (created at init), current CBZ document (open while viewing)
fz_context* g_mupdf_ctx = NULL;
fz_document* g_mupdf_doc = NULL;
std::string g_mupdf_path;

#ifdef CBR_SUPPORT
// unarr: current CBR archive (open while viewing)
ar_stream* g_cbr_stream = NULL;
ar_archive* g_cbr_archive = NULL;
std::string g_cbr_path;
std::vector<int64_t> g_cbr_page_offsets;  // ar_entry_get_offset per page (off64_t)
#endif

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Globally used font
TTF_Font *gFont = NULL;
TTF_Font *gFontcapit = NULL;
TTF_Font *gFont2 = NULL;
TTF_Font *gFont3 = NULL;
//Rendered texture
LTexture gTextTexture;
LTexture Farest;
LTexture Heart;
LTexture Pagemanga;
LTexture lupa;
LTexture TPreview;

std::vector<LTexture> arraypage;
std::vector<std::string> arraymain;
std::vector<bool> arraymain_is_dir;  // true = folder, false = .cbz file
std::vector<std::string> arraychapter;

std::string g_search_filter;
std::vector<std::string> arraymain_filtered;
std::vector<bool> arraymain_is_dir_filtered;
#ifndef __SWITCH__
bool g_search_input_mode = false;
#endif

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile(std::string path)
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load((path.c_str()));
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Color key image
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}


bool LTexture::loadFromFileCustom(std::string path, int h, int w)
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Color key image
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = w;
			mHeight = h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

bool LTexture::loadFromMemory(const void* buf, int size)
{
	free();
	SDL_Texture* newTexture = NULL;
	SDL_RWops* rw = SDL_RWFromConstMem(buf, size);
	if (!rw) return false;
	SDL_Surface* loadedSurface = IMG_Load_RW(rw, 1);
	if (!loadedSurface) return false;
	SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));
	newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
	if (newTexture) {
		mWidth = loadedSurface->w;
		mHeight = loadedSurface->h;
	}
	SDL_FreeSurface(loadedSurface);
	mTexture = newTexture;
	return mTexture != NULL;
}

bool LTexture::loadFromRGB(const unsigned char* data, int w, int h, int stride, int n) {
	free();
	if (!data || w <= 0 || h <= 0 || stride < w * n) return false;
	SDL_Surface* surf = NULL;
	if (n == 3) {
		surf = SDL_CreateRGBSurfaceFrom((void*)data, w, h, 24, stride, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
	} else if (n == 4) {
		surf = SDL_CreateRGBSurfaceFrom((void*)data, w, h, 32, stride, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	} else return false;
	if (!surf) return false;
	SDL_Texture* newTexture = SDL_CreateTextureFromSurface(gRenderer, surf);
	if (newTexture) { mWidth = w; mHeight = h; }
	SDL_FreeSurface(surf);
	mTexture = newTexture;
	return mTexture != NULL;
}

bool LTexture::loadFromRenderedText(TTF_Font *fuente, std::string textureText, SDL_Color textColor)
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Blended(fuente, textureText.c_str(), textColor);

	if (textSurface == NULL)
	{
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
	}
	else
	{
		//Create texture from surface pixels
		mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
		if (mTexture == NULL)
		{
			printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface(textSurface);
	}

	//Return success
	return mTexture != NULL;
}
bool LTexture::loadFromRenderedTextWrap(TTF_Font *fuente, std::string textureText, SDL_Color textColor, Uint32 size)
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderUTF8_Blended_Wrapped(fuente, textureText.c_str(), textColor,size);

	if (textSurface == NULL)
	{
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
	}
	else
	{
		//Create texture from surface pixels
		mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
		if (mTexture == NULL)
		{
			printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface(textSurface);
	}

	//Return success
	return mTexture != NULL;
}

void LTexture::free()
{
	//Free texture if it exists
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
	//Modulate texture rgb
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void LTexture::setBlendMode(SDL_BlendMode blending)
{
	//Set blending function
	SDL_SetTextureBlendMode(mTexture, blending);
}

void LTexture::setAlpha(Uint8 alpha)
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod(mTexture, alpha);
}

void LTexture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if (clip != NULL)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}
void LTexture::customrender(int x, int y, int w,int h, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, w,h };

	//Set clip rendering dimensions
	if (clip != NULL)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

//-----------------------------------------------------------------------------
// CBZ support via MuPDF - open, list pages, render to texture
//-----------------------------------------------------------------------------

// Natural sort: "Batman 001" < "Batman 002" < "Batman 010" (numeric order, not alphabetical)
static bool naturalSortCompare(const std::string& a, const std::string& b) {
	size_t i = 0, j = 0;
	while (i < a.size() && j < b.size()) {
		bool aDig = (a[i] >= '0' && a[i] <= '9');
		bool bDig = (b[j] >= '0' && b[j] <= '9');
		if (aDig && bDig) {
			unsigned long long na = 0, nb = 0;
			while (i < a.size() && a[i] >= '0' && a[i] <= '9') { na = na * 10 + (a[i] - '0'); i++; }
			while (j < b.size() && b[j] >= '0' && b[j] <= '9') { nb = nb * 10 + (b[j] - '0'); j++; }
			if (na != nb) return na < nb;
			continue;
		}
		if (aDig != bDig) return a[i] < b[j];
		if ((unsigned char)a[i] != (unsigned char)b[j]) return (unsigned char)a[i] < (unsigned char)b[j];
		i++; j++;
	}
	/* a is prefix of b -> a < b; otherwise a >= b */
	return (i >= a.size() && j < b.size());
}

#ifdef CBR_SUPPORT
static void closeCbr();  // forward decl for openCbzWithMupdf
#endif
// Open CBZ with MuPDF; sets g_mupdf_doc and g_mupdf_path. Returns page count or -1 on error.
static int openCbzWithMupdf(const char* cbzPath) {
	g_cbz_open_error = CBZ_ERR_NONE;
	g_cbz_mupdf_error.clear();
	if (!g_mupdf_ctx) return -1;
	if (g_mupdf_doc) { fz_drop_document(g_mupdf_ctx, g_mupdf_doc); g_mupdf_doc = NULL; }
	g_mupdf_path.clear();
#ifdef CBR_SUPPORT
	closeCbr();
#endif
	const char* pathToOpen = cbzPath;
#ifdef __SWITCH__
	FILE* test = fopen(cbzPath, "rb");
	if (test) {
		fclose(test);
	} else if (strncmp(cbzPath, "sdmc:/", 6) == 0) {
		test = fopen(cbzPath + 6, "rb");
		if (!test) { g_cbz_open_error = CBZ_ERR_FILE_NOT_FOUND; return -1; }
		fclose(test);
		pathToOpen = cbzPath + 6;  // MuPDF needs the path that fopen accepts
	} else {
		g_cbz_open_error = CBZ_ERR_FILE_NOT_FOUND;
		return -1;
	}
#endif
	fz_try(g_mupdf_ctx) {
		g_mupdf_doc = fz_open_document(g_mupdf_ctx, pathToOpen);
		if (!g_mupdf_doc) fz_throw(g_mupdf_ctx, FZ_ERROR_GENERIC, "open failed");
		g_mupdf_path = cbzPath;
	}
	fz_catch(g_mupdf_ctx) {
		if (g_mupdf_doc) { fz_drop_document(g_mupdf_ctx, g_mupdf_doc); g_mupdf_doc = NULL; }
		g_cbz_open_error = CBZ_ERR_NOT_ZIP;
		const char* msg = fz_caught_message(g_mupdf_ctx);
		g_cbz_mupdf_error = msg ? msg : "unknown error";
		return -1;
	}
	return fz_count_pages(g_mupdf_ctx, g_mupdf_doc);
}

// List "pages" for CBZ: returns count; caller fills arraychapter with cbz:path:0, cbz:path:1, ...
static int listPagesFromCbz(const char* cbzPath) {
	return openCbzWithMupdf(cbzPath);
}

// Render CBZ page (0-based) into texture. g_mupdf_doc must be open for this path.
static bool loadCbzPageIntoTexture(const char* cbzPath, int pageIndex, LTexture* tex) {
	if (!g_mupdf_ctx || !g_mupdf_doc || g_mupdf_path != cbzPath) return false;
	int npages = fz_count_pages(g_mupdf_ctx, g_mupdf_doc);
	if (pageIndex < 0 || pageIndex >= npages) return false;
	fz_page* page = NULL;
	fz_pixmap* pix = NULL;
	fz_device* dev = NULL;
	bool ok = false;
	fz_try(g_mupdf_ctx) {
		page = fz_load_page(g_mupdf_ctx, g_mupdf_doc, pageIndex);
		fz_rect bounds = fz_bound_page(g_mupdf_ctx, page);
		/* Scale 2x for higher quality; device must use identity so ctm is not applied twice */
		float scale = 2.0f;
		fz_matrix ctm = fz_scale(scale, scale);
		fz_rect rect = fz_transform_rect(bounds, ctm);
		fz_irect bbox = fz_round_rect(rect);
		pix = fz_new_pixmap_with_bbox(g_mupdf_ctx, fz_device_rgb(g_mupdf_ctx), bbox, NULL, 1);
		fz_clear_pixmap_with_value(g_mupdf_ctx, pix, 0xff);
		dev = fz_new_draw_device(g_mupdf_ctx, fz_identity, pix);
		fz_run_page(g_mupdf_ctx, page, dev, ctm, NULL);
		fz_close_device(g_mupdf_ctx, dev);
		fz_drop_device(g_mupdf_ctx, dev);
		dev = NULL;
		fz_drop_page(g_mupdf_ctx, page);
		page = NULL;
		int w = fz_pixmap_width(g_mupdf_ctx, pix);
		int h = fz_pixmap_height(g_mupdf_ctx, pix);
		int stride = fz_pixmap_stride(g_mupdf_ctx, pix);
		int n = fz_pixmap_components(g_mupdf_ctx, pix);
		unsigned char* samples = fz_pixmap_samples(g_mupdf_ctx, pix);
		ok = tex->loadFromRGB(samples, w, h, stride, n);
		fz_drop_pixmap(g_mupdf_ctx, pix);
		pix = NULL;
	}
	fz_catch(g_mupdf_ctx) {
		if (dev) fz_drop_device(g_mupdf_ctx, dev);
		if (page) fz_drop_page(g_mupdf_ctx, page);
		if (pix) fz_drop_pixmap(g_mupdf_ctx, pix);
	}
	return ok;
}

//-----------------------------------------------------------------------------
// CBR support via unarr (RAR comic archives) - optional when CBR_SUPPORT defined
//-----------------------------------------------------------------------------
#ifdef CBR_SUPPORT
static void closeCbr() {
	if (g_cbr_archive) { ar_close_archive(g_cbr_archive); g_cbr_archive = NULL; }
	if (g_cbr_stream) { ar_close(g_cbr_stream); g_cbr_stream = NULL; }
	g_cbr_path.clear();
	g_cbr_page_offsets.clear();
}

static bool isImageFilename(const std::string& name) {
	std::string lower = name;
	for (size_t i = 0; i < lower.size(); i++)
		if (lower[i] >= 'A' && lower[i] <= 'Z') lower[i] += 32;
	return lower.find(".jpg") != std::string::npos || lower.find(".jpeg") != std::string::npos ||
		lower.find(".png") != std::string::npos || lower.find(".bmp") != std::string::npos ||
		lower.find(".gif") != std::string::npos || lower.find(".webp") != std::string::npos;
}

// Open CBR with unarr; sets g_cbr_*. Returns page count or -1 on error.
static int openCbrWithUnarr(const char* cbrPath) {
	g_cbz_open_error = CBZ_ERR_NONE;
	g_cbz_open_error_msg.clear();
	closeCbr();
	const char* pathToOpen = cbrPath;
#ifdef __SWITCH__
	FILE* test = fopen(cbrPath, "rb");
	if (test) {
		fclose(test);
		pathToOpen = cbrPath;
	} else if (strncmp(cbrPath, "sdmc:/", 6) == 0) {
		test = fopen(cbrPath + 6, "rb");
		if (!test) { g_cbz_open_error = CBZ_ERR_FILE_NOT_FOUND; return -1; }
		fclose(test);
		pathToOpen = cbrPath + 6;
	} else {
		g_cbz_open_error = CBZ_ERR_FILE_NOT_FOUND;
		return -1;
	}
#endif
	g_cbr_stream = ar_open_file(pathToOpen);
	if (!g_cbr_stream) { g_cbz_open_error = CBZ_ERR_FILE_NOT_FOUND; return -1; }
	g_cbr_archive = ar_open_rar_archive(g_cbr_stream);
	if (!g_cbr_archive) {
		ar_close(g_cbr_stream);
		g_cbr_stream = NULL;
		g_cbz_open_error = CBZ_ERR_NOT_RAR;
		g_cbz_open_error_msg = "Not a valid CBR (RAR) file.";
		return -1;
	}
	std::vector<std::pair<std::string, int64_t>> entries;
	while (ar_parse_entry(g_cbr_archive)) {
		const char* n = ar_entry_get_name(g_cbr_archive);
		if (!n) continue;
		std::string name(n);
		if (isImageFilename(name))
			entries.push_back(std::make_pair(name, (int64_t)ar_entry_get_offset(g_cbr_archive)));
	}
	std::sort(entries.begin(), entries.end(),
		[](const std::pair<std::string, int64_t>& a, const std::pair<std::string, int64_t>& b) {
			return naturalSortCompare(a.first, b.first);
		});
	g_cbr_page_offsets.clear();
	for (size_t i = 0; i < entries.size(); i++)
		g_cbr_page_offsets.push_back(entries[i].second);
	g_cbr_path = cbrPath;
	return (int)g_cbr_page_offsets.size();
}

static int listPagesFromCbr(const char* cbrPath) {
	return openCbrWithUnarr(cbrPath);
}

static bool loadCbrPageIntoTexture(const char* cbrPath, int pageIndex, LTexture* tex) {
	if (!g_cbr_archive || g_cbr_path != cbrPath || pageIndex < 0 || (size_t)pageIndex >= g_cbr_page_offsets.size())
		return false;
	if (!ar_parse_entry_at(g_cbr_archive, (off64_t)g_cbr_page_offsets[pageIndex]))
		return false;
	size_t sz = ar_entry_get_size(g_cbr_archive);
	void* buf = malloc(sz);
	if (!buf) return false;
	bool ok = ar_entry_uncompress(g_cbr_archive, buf, sz);
	if (ok) tex->loadFromMemory(buf, (int)sz);
	free(buf);
	return ok;
}
#endif

// Load a page from arraychapter entry (file path or "cbz:path:pageIndex", "cbr:path:pageIndex", "pdf:path:pageIndex")
static void loadPageIntoTexture(const std::string& entry, LTexture* tex) {
	if (entry.size() > 4 && (entry.compare(0, 4, "cbz:") == 0 || entry.compare(0, 4, "pdf:") == 0)) {
		size_t lastColon = entry.rfind(':');
		if (lastColon != std::string::npos && lastColon > 4) {
			std::string path = entry.substr(4, lastColon - 4);
			int pageIndex = atoi(entry.substr(lastColon + 1).c_str());
			loadCbzPageIntoTexture(path.c_str(), pageIndex, tex);
			return;
		}
	}
#ifdef CBR_SUPPORT
	if (entry.size() > 4 && entry.compare(0, 4, "cbr:") == 0) {
		size_t lastColon = entry.rfind(':');
		if (lastColon != std::string::npos && lastColon > 4) {
			std::string path = entry.substr(4, lastColon - 4);
			int pageIndex = atoi(entry.substr(lastColon + 1).c_str());
			loadCbrPageIntoTexture(path.c_str(), pageIndex, tex);
			return;
		}
	}
#endif
	tex->loadFromFile(entry);
}

//-----------------------------------------------------------------------------
// Reading progress: one file (reading_progress.txt) stores each manga's
// last page; one line per manga: "path\tpage\n". Save on page change, restore on open.
// Uses static buffer (not stack) for devkitA64 / limited stack.
//-----------------------------------------------------------------------------
extern std::vector<std::string> arraychapter;  // defined later in file
static std::string getCurrentMangaPath() {
	if (arraychapter.empty()) return "";
	const std::string& first = arraychapter[0];
	if (first.size() > 4 && (first.compare(0, 4, "cbz:") == 0 || first.compare(0, 4, "cbr:") == 0 || first.compare(0, 4, "pdf:") == 0)) {
		size_t lastColon = first.rfind(':');
		if (lastColon != std::string::npos && lastColon > 4)
			return first.substr(4, lastColon - 4);
	} else {
		size_t lastSlash = first.rfind('/');
		if (lastSlash != std::string::npos)
			return first.substr(0, lastSlash + 1);
	}
	return "";
}

#define READING_PROGRESS_MAX_ENTRIES 64
#define READING_PROGRESS_MAX_PATH 384

// Returns saved page index (0-based) or -1 if not found.
static int loadReadingProgress(const std::string& foldermain, const std::string& mangaPath) {
	char pathbuf[READING_PROGRESS_MAX_PATH];
	size_t flen = foldermain.size();
	if (flen + 24 >= sizeof(pathbuf)) return -1;
	memcpy(pathbuf, foldermain.c_str(), flen + 1);
	strcat(pathbuf, "reading_progress.txt");
	FILE* f = fopen(pathbuf, "r");
	if (!f) return -1;
	char line[1024];
	int page = -1;
	while (fgets(line, (int)sizeof(line), f)) {
		char* tab = strchr(line, '\t');
		if (!tab) continue;
		*tab = '\0';
		if (strcmp(line, mangaPath.c_str()) == 0)
			page = atoi(tab + 1);
		*tab = '\t';
	}
	fclose(f);
	return page;
}

// Static buffer so we don't use ~25KB+ stack (Switch has limited stack).
static struct { char path[READING_PROGRESS_MAX_PATH]; int page; } s_progress_entries[READING_PROGRESS_MAX_ENTRIES];

// Updates progress: read all entries, update this manga's page, write all back.
static void saveReadingProgress(const std::string& foldermain, const std::string& mangaPath, int pageIndex) {
	char pathbuf[READING_PROGRESS_MAX_PATH];
	size_t flen = foldermain.size();
	if (flen + 24 >= sizeof(pathbuf)) return;
	memcpy(pathbuf, foldermain.c_str(), flen + 1);
	strcat(pathbuf, "reading_progress.txt");
	int n_entries = 0;
	FILE* f = fopen(pathbuf, "r");
	if (f) {
		char line[1024];
		while (n_entries < READING_PROGRESS_MAX_ENTRIES && fgets(line, (int)sizeof(line), f)) {
			char* tab = strchr(line, '\t');
			if (!tab) continue;
			*tab = '\0';
			size_t plen = (size_t)(tab - line);
			if (plen >= (size_t)READING_PROGRESS_MAX_PATH) { *tab = '\t'; continue; }
			memcpy(s_progress_entries[n_entries].path, line, plen + 1);
			s_progress_entries[n_entries].page = atoi(tab + 1);
			n_entries++;
			*tab = '\t';
		}
		fclose(f);
	}
	int found = -1;
	for (int i = 0; i < n_entries; i++) {
		if (strcmp(s_progress_entries[i].path, mangaPath.c_str()) == 0) { found = i; break; }
	}
	if (found >= 0)
		s_progress_entries[found].page = pageIndex;
	else if (n_entries < READING_PROGRESS_MAX_ENTRIES && (int)mangaPath.size() < READING_PROGRESS_MAX_PATH) {
		strncpy(s_progress_entries[n_entries].path, mangaPath.c_str(), READING_PROGRESS_MAX_PATH - 1);
		s_progress_entries[n_entries].path[READING_PROGRESS_MAX_PATH - 1] = '\0';
		s_progress_entries[n_entries].page = pageIndex;
		n_entries++;
	}
	f = fopen(pathbuf, "w");
	if (!f) return;
	for (int j = 0; j < n_entries; j++)
		fprintf(f, "%s\t%d\n", s_progress_entries[j].path, s_progress_entries[j].page);
	fclose(f);
}

void renderLoadingScreen() {
	if (g_portrait_mode && g_portrait_texture != NULL)
		SDL_SetRenderTarget(gRenderer, g_portrait_texture);
	SDL_SetRenderDrawColor(gRenderer, 233, 234, 237, 0xFF);
	SDL_RenderClear(gRenderer);
	if (gFont) {
		gTextTexture.loadFromRenderedText(gFont, "Loading", { 50, 50, 50 });
		gTextTexture.render(g_view_w / 2 - gTextTexture.getWidth() / 2, g_view_h / 2 - gTextTexture.getHeight() / 2);
	}
	if (g_portrait_mode && g_portrait_texture != NULL) {
		SDL_SetRenderTarget(gRenderer, NULL);
		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
		SDL_RenderClear(gRenderer);
		SDL_Rect dest = { (1280 - 720) / 2, (720 - 1280) / 2, 720, 1280 };
		SDL_Point center = { 360, 640 };
		SDL_RenderCopyEx(gRenderer, g_portrait_texture, NULL, &dest, -90.0, &center, SDL_FLIP_NONE);
	}
	SDL_RenderPresent(gRenderer);
	SDL_PumpEvents();
}

// --- App init / load (see cute_manga.h) ---
bool initSDLAndWindow() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		return false;
	}
	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		printf("Warning: Linear texture filtering not enabled!");
#ifdef __SWITCH__
	gWindow = SDL_CreateWindow("sdl2_gles2", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
#else
	gWindow = SDL_CreateWindow("CuteManga", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
#endif
	if (gWindow == NULL) {
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}
	gRenderer = SDL_CreateRenderer(gWindow, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (gRenderer == NULL) {
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags))
		printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
	if (TTF_Init() == -1) {
		printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
		return false;
	}
	g_mupdf_ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
	if (g_mupdf_ctx) fz_register_document_handlers(g_mupdf_ctx);
	return true;
}

bool initFontsAndAssets(std::string& foldermain) {
#ifdef __SWITCH__
	gFont = TTF_OpenFont("romfs:/lazy.ttf", 16);
	gFont2 = TTF_OpenFont("romfs:/lazy2.ttf", 150);
	gFontcapit = TTF_OpenFont("romfs:/lazy2.ttf", 100);
	gFont3 = TTF_OpenFont("romfs:/lazy2.ttf", 40);
	lupa.loadFromFile("romfs:/lupa.png");
	Heart.loadFromFile("romfs:/heart.png");
	Farest.loadFromFile("romfs:/texture.png");
	foldermain = "sdmc:/CuteManga/";
#else
	gFont = TTF_OpenFont("C:\\respaldo2017\\C++\\test\\Debug\\lazy.ttf", 16);
	gFont2 = TTF_OpenFont("C:\\respaldo2017\\C++\\test\\Debug\\lazy2.ttf", 150);
	gFontcapit = TTF_OpenFont("C:\\respaldo2017\\C++\\test\\Debug\\lazy2.ttf", 100);
	gFont3 = TTF_OpenFont("C:\\respaldo2017\\C++\\test\\Debug\\lazy2.ttf", 40);
	Heart.loadFromFile("C:/respaldo2017/C++/CuteManga/Debug/heart.png");
	Farest.loadFromFile("C:/respaldo2017/C++/CuteManga/Debug/texture.png");
	lupa.loadFromFile("C:/respaldo2017/C++/CuteManga/Debug/lupa.png");
	foldermain = "C:/respaldo2017/C++/CuteManga/Debug/";
#endif
#ifdef __SWITCH__
	{
		struct stat st = { 0 };
		if (stat(foldermain.c_str(), &st) == -1)
			mkdir(foldermain.c_str(), 0755);
	}
#endif
	return true;
}

static std::string toLower(const std::string& s) {
	std::string out;
	out.reserve(s.size());
	for (unsigned char c : s)
		out += (char)((c >= 'A' && c <= 'Z') ? c + 32 : c);
	return out;
}

static void rebuildSearchFilter() {
	arraymain_filtered.clear();
	arraymain_is_dir_filtered.clear();
	if (g_search_filter.empty()) return;
	std::string needle = toLower(g_search_filter);
	for (size_t i = 0; i < arraymain.size(); i++) {
		if (toLower(arraymain[i]).find(needle) != std::string::npos) {
			arraymain_filtered.push_back(arraymain[i]);
			arraymain_is_dir_filtered.push_back(arraymain_is_dir[i]);
		}
	}
	size_t n = arraymain_filtered.size();
	if (n == 0) { selectchapter = 0; baseymain = 30; }
	else if ((size_t)selectchapter >= n) {
		selectchapter = (int)n - 1;
		baseymain = 30 - 20 * ((int)n - 1);
	}
}

static size_t getMangaListCount() {
	return g_search_filter.empty() ? arraymain.size() : arraymain_filtered.size();
}
static const std::string& getMangaName(size_t i) {
	size_t n = getMangaListCount();
	if (i >= n) {
		static const std::string empty;
		return empty;
	}
	return g_search_filter.empty() ? arraymain[i] : arraymain_filtered[i];
}
static bool getMangaIsDir(size_t i) {
	size_t n = getMangaListCount();
	if (i >= n) return false;
	return g_search_filter.empty() ? arraymain_is_dir[i] : arraymain_is_dir_filtered[i];
}

void loadFolderList(const std::string& foldermain) {
	arraymain.clear();
	arraymain_is_dir.clear();
	g_search_filter.clear();
	arraymain_filtered.clear();
	arraymain_is_dir_filtered.clear();
	DIR* dirmain;
	struct dirent* entmain;
	if ((dirmain = opendir(foldermain.c_str())) != NULL) {
		std::vector<std::pair<std::string, bool> > entries;
		while ((entmain = readdir(dirmain)) != NULL) {
			if (strcmp(entmain->d_name, ".") == 0 || strcmp(entmain->d_name, "..") == 0)
				continue;
			std::string name(entmain->d_name);
			bool is_dir = (entmain->d_type == DT_DIR);
			if (entmain->d_type == DT_UNKNOWN) {
				std::string fullpath = foldermain + name;
				struct stat st;
				if (stat(fullpath.c_str(), &st) == 0)
					is_dir = S_ISDIR(st.st_mode);
			}
			bool is_archive = (name.size() >= 4 && name.compare(name.size() - 4, 4, ".cbz") == 0)
#ifdef CBR_SUPPORT
				|| (name.size() >= 4 && name.compare(name.size() - 4, 4, ".cbr") == 0)
#endif
				|| (name.size() >= 4 && name.compare(name.size() - 4, 4, ".pdf") == 0);
			if (is_dir || is_archive) {
				printf("%s\n", entmain->d_name);
				entries.push_back(std::make_pair(name, is_dir));
			}
		}
		closedir(dirmain);
		std::sort(entries.begin(), entries.end(),
			[](const std::pair<std::string, bool>& a, const std::pair<std::string, bool>& b) {
				return naturalSortCompare(a.first, b.first);
			});
		for (size_t i = 0; i < entries.size(); i++) {
			arraymain.push_back(entries[i].first);
			arraymain_is_dir.push_back(entries[i].second);
		}
	} else {
		existfoldermain = false;
	}
}

void doDeferredPageLoad(const std::string& foldermain) {
	if (!g_pending_page_load || statenow != readmanga) return;
	Pagemanga.free();
	loadPageIntoTexture(arraychapter[selectpage], &Pagemanga);
	std::string mangaPath = getCurrentMangaPath();
	if (!mangaPath.empty())
		saveReadingProgress(foldermain, mangaPath, selectpage);
	g_pending_page_load = false;
}

void close()
{
	//Free loaded images
	gTextTexture.free();
	Farest.free();
	Heart.free();
	TPreview.free();
	Pagemanga.free();
	lupa.free();
	for (size_t x = 0; x < arraypage.size(); x++)
	{
		arraypage[x].free();
	}
	//Free global font
	TTF_CloseFont(gFont);
	gFont = NULL;

	TTF_CloseFont(gFontcapit);
	gFontcapit = NULL;
	TTF_CloseFont(gFont2);
	gFont2 = NULL;
	TTF_CloseFont(gFont3);
	gFont3 = NULL;
	//Destroy portrait render target
	if (g_portrait_texture != NULL)
	{
		SDL_DestroyTexture(g_portrait_texture);
		g_portrait_texture = NULL;
	}
	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	// MuPDF cleanup
	if (g_mupdf_doc && g_mupdf_ctx) { fz_drop_document(g_mupdf_ctx, g_mupdf_doc); g_mupdf_doc = NULL; }
	if (g_mupdf_ctx) { fz_drop_context(g_mupdf_ctx); g_mupdf_ctx = NULL; }

	//Quit SDL subsystems
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}


void replace(std::string& subject, const std::string& search,
	const std::string& replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}


void mayus(std::string &s)
{
	bool cap = true;

	for (unsigned int i = 0; i <= s.length(); i++)
	{
		if (isalpha(s[i]) && cap == true)
		{
			s[i] = toupper(s[i]);
			cap = false;
		}
		else if (isspace(s[i]))
		{
			cap = true;
		}
	}
}
int progress_func(void* ptr, double TotalToDownload, double NowDownloaded,
	double TotalToUpload, double NowUploaded)
{


	// ensure that the file to be downloaded is not empty
	// because that would cause a division by zero error later on
	if (TotalToDownload <= 0.0) {
		return 0;
	}

	// how wide you want the progress meter to be
	int totaldotz = 20;
	double fractiondownloaded = NowDownloaded / TotalToDownload;
	// part of the progressmeter that's already "full"
	int dotz = round(fractiondownloaded * totaldotz);

	// create the "meter"
	int ii = 0;
	porcendown = fractiondownloaded * 100;
	sizeestimated = TotalToDownload;
	printf("%3.0f%% [", fractiondownloaded * 100);
	// part  that's full already
	for (; ii < dotz; ii++) {
		printf("=");
	}
	// remaining part (spaces)
	for (; ii < 20; ii++) {
		printf(" ");
	}
	// and back to line begin - do not forget the fflush to avoid output buffering problems!
	printf("]\r");
	fflush(stdout);
	// if you don't return 0, the transfer will be aborted - see the documentation
	if (cancelcurl == 1)
	{
		return 1;
	}

	return 0;

}

std::string gethtml(std::string enlace)
{

	CURL *curl;
	std::string Buffer;

	curl = curl_easy_init();
	if (curl) {

		curl_easy_setopt(curl, CURLOPT_URL, enlace.c_str());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36");
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &Buffer);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		return Buffer;
	}
	return "";
}
void downloadfile(std::string enlace, std::string directorydown)
{
	
	CURL *curl;
	FILE *fp;

	curl = curl_easy_init();
	if (curl) {

		fp = fopen(directorydown.c_str(), "wb");
		curl_easy_setopt(curl, CURLOPT_URL, enlace.c_str());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36");
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		// Install the callback function
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_func);
		curl_easy_perform(curl);
		/* always cleanup */
		curl_easy_cleanup(curl);
		fclose(fp);
	}
}




#ifdef __SWITCH__
void handleSwitchInput(u64 kDown, u64 kHeld) {
	if (kDown & HidNpadButton_Plus) { quit = 1; }
	if (kDown & HidNpadButton_Y) {
		g_portrait_mode = !g_portrait_mode;
		g_view_w = g_portrait_mode ? 720 : 1280;
		g_view_h = g_portrait_mode ? 1280 : 720;
		if (g_portrait_mode && g_portrait_texture == NULL && gRenderer != NULL)
			g_portrait_texture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 720, 1280);
	}
	if (kHeld & HidNpadButton_StickLDown) {
		switch (statenow) {
		case readmanga:
			if (cascade == false) {
				if ((basey) > (g_view_h - heightnew)) {
					basey -= speed;
					if (basey <= g_view_h - heightnew) basey = g_view_h - heightnew;
				}
			} else basey -= speed;
			break;
		}
	}
	if (kDown & HidNpadButton_StickLDown) {
		switch (statenow) {
		case selectmanga:
			g_cbz_open_error_msg.clear();
			if (selectchapter < (int)getMangaListCount() - 1) {
				selectchapter++;
				baseymain = baseymain - 20;
			} else {
				selectchapter = 0;
				baseymain = 30;
			}
			break;
		}
	}
	if (kHeld & HidNpadButton_StickLUp) {
		switch (statenow) {
		case readmanga:
			if (cascade == false) {
				if (basey <= 0) {
					basey += speed;
					if (basey > 0) basey = 0;
				}
			} else {
				if (basey <= 0) {
					basey += speed;
					if (basey > 0) basey = 0;
				}
			}
			break;
		}
	}
	if (kDown & HidNpadButton_StickLUp) {
		switch (statenow) {
		case selectmanga:
			g_cbz_open_error_msg.clear();
			if (selectchapter > 0) {
				selectchapter--;
				baseymain = baseymain + 20;
			} else {
				size_t n = getMangaListCount();
				baseymain = (n > 0) ? (30 - 20 * ((int)n - 1)) : 30;
				selectchapter = (n > 0) ? (int)n - 1 : 0;
			}
			break;
		}
	}
	if (kHeld & HidNpadButton_StickLLeft) {
		switch (statenow) {
		case readmanga:
			if (zoom == 2) basex += speed;
			break;
		}
	}
	if (kHeld & HidNpadButton_StickLRight) {
		switch (statenow) {
		case readmanga:
			if (zoom == 2) basex -= speed;
			break;
		}
	}
	if (kHeld & HidNpadButton_ZL) {
		switch (statenow) {
		case readmanga:
			if (neutralsize > 0.05f) neutralsize -= 0.01f;
			break;
		}
	}
	if (kHeld & HidNpadButton_ZR) {
		switch (statenow) {
		case readmanga:
			neutralsize += 0.01f;
			break;
		}
	}
	if (kDown & HidNpadButton_L) {
		switch (statenow) {
		case readmanga:
			if (cascade == false) {
				basey = 0; basex = 0; zoom = 1;
				if (selectpage > 0) { selectpage--; g_pending_page_load = true; }
				else { selectpage = 0; g_pending_page_load = true; }
			} else zoom = 1;
			break;
		}
	}
	if (kDown & HidNpadButton_R) {
		switch (statenow) {
		case readmanga:
			if (cascade == false) {
				basey = 0; basex = 0; zoom = 1;
				if (selectpage < (int)arraychapter.size() - 1) {
					selectpage++;
					g_pending_page_load = true;
				}
			} else zoom = 1;
			break;
		}
	}
	if (kDown & HidNpadButton_StickR) {
		switch (statenow) {
		case readmanga:
			if (cascade == false) {
				if (zoom == 1) zoom = 2;
				else { zoom = 1; basex = 0; basey = 0; }
			} else {
				if (zoom == 1) zoom = 2;
				else { zoom = 1; basex = 0; }
			}
			break;
		}
	}
	if (kDown & HidNpadButton_A) {
		if (existfoldermain) {
			switch (statenow) {
			case readmanga:
				if (cascade == false) { basey = 0; adjust = !adjust; }
				else adjust = !adjust;
				break;
			case selectmanga: {
				size_t n = getMangaListCount();
				if (n == 0 || selectchapter < 0 || (size_t)selectchapter >= n)
					break;
				if (!getMangaIsDir((size_t)selectchapter)) {
					g_cbz_open_error = CBZ_ERR_NONE;
					g_cbz_open_error_msg.clear();
					std::string fname = getMangaName((size_t)selectchapter);
					size_t start = fname.find_first_not_of(" \t\r\n");
					size_t end = fname.find_last_not_of(" \t\r\n");
					if (start != std::string::npos) fname = (end != std::string::npos) ? fname.substr(start, end - start + 1) : fname.substr(start);
					std::string archPath = g_foldermain + fname;
					for (size_t i = 0; i < archPath.size(); i++)
						if (archPath[i] == '\\') archPath[i] = '/';
					renderLoadingScreen();
#ifdef CBR_SUPPORT
					bool isCbr = (fname.size() >= 4 && fname.compare(fname.size() - 4, 4, ".cbr") == 0);
					if (isCbr) {
						int pageCount = listPagesFromCbr(archPath.c_str());
						if (pageCount <= 0) {
							if (g_cbz_open_error == CBZ_ERR_FILE_NOT_FOUND)
								g_cbz_open_error_msg = "File not found. Put CBR in sdmc:/CuteManga/";
							else if (g_cbz_open_error == CBZ_ERR_NOT_RAR)
								g_cbz_open_error_msg = "Not a valid CBR (RAR) file.";
							else g_cbz_open_error_msg = "No pages in CBR.";
						} else {
							arraychapter.clear();
							for (int i = 0; i < pageCount; i++)
								arraychapter.push_back(std::string("cbr:") + archPath + ":" + std::to_string(i));
							selectpage = 0;
							int savedPage = loadReadingProgress(g_foldermain, archPath);
							if (savedPage >= 0 && savedPage < pageCount) selectpage = savedPage;
							bool ok = loadCbrPageIntoTexture(archPath.c_str(), selectpage, &Pagemanga);
							arraypage.resize(arraychapter.size());
							if (cascadeactivated) {
								for (int x = 0; x < pageCount; x++)
									loadCbrPageIntoTexture(archPath.c_str(), x, &arraypage[x]);
							}
							if (ok) { statenow = readmanga; helppage = true; }
						}
						break;
					}
#endif
					bool isPdf = (fname.size() >= 4 && fname.compare(fname.size() - 4, 4, ".pdf") == 0);
					if (isPdf) {
						int pageCount = listPagesFromCbz(archPath.c_str());
						if (pageCount <= 0) {
							if (g_cbz_open_error == CBZ_ERR_FILE_NOT_FOUND)
								g_cbz_open_error_msg = "File not found. Put PDF in sdmc:/CuteManga/";
							else if (g_cbz_open_error == CBZ_ERR_NOT_ZIP)
								g_cbz_open_error_msg = "Not a valid PDF file.";
							else g_cbz_open_error_msg = "No pages in PDF.";
						} else {
							arraychapter.clear();
							for (int i = 0; i < pageCount; i++)
								arraychapter.push_back(std::string("pdf:") + archPath + ":" + std::to_string(i));
							selectpage = 0;
							int savedPage = loadReadingProgress(g_foldermain, archPath);
							if (savedPage >= 0 && savedPage < pageCount) selectpage = savedPage;
							bool ok = loadCbzPageIntoTexture(archPath.c_str(), selectpage, &Pagemanga);
							arraypage.resize(arraychapter.size());
							if (cascadeactivated) {
								for (int x = 0; x < pageCount; x++)
									loadCbzPageIntoTexture(archPath.c_str(), x, &arraypage[x]);
							}
							if (ok) { statenow = readmanga; helppage = true; }
						}
						break;
					}
					int pageCount = listPagesFromCbz(archPath.c_str());
					if (pageCount <= 0) {
						if (g_cbz_open_error == CBZ_ERR_FILE_NOT_FOUND)
							g_cbz_open_error_msg = "File not found. Put CBZ in sdmc:/CuteManga/";
						else if (g_cbz_open_error == CBZ_ERR_NOT_ZIP) {
							if (g_cbz_mupdf_error.empty()) g_cbz_open_error_msg = "Not a valid CBZ file.";
							else if (g_cbz_mupdf_error.find("cannot recognize archive") != std::string::npos)
								g_cbz_open_error_msg = "Not a ZIP. Use ZIP-based CBZ (not RAR/CBR).";
							else g_cbz_open_error_msg = "CBZ: " + g_cbz_mupdf_error;
						} else g_cbz_open_error_msg = "No pages in CBZ.";
					} else {
						arraychapter.clear();
						for (int i = 0; i < pageCount; i++)
							arraychapter.push_back(std::string("cbz:") + archPath + ":" + std::to_string(i));
						selectpage = 0;
						int savedPage = loadReadingProgress(g_foldermain, archPath);
						if (savedPage >= 0 && savedPage < pageCount) selectpage = savedPage;
						bool ok = loadCbzPageIntoTexture(archPath.c_str(), selectpage, &Pagemanga);
						arraypage.resize(arraychapter.size());
						if (cascadeactivated) {
							for (int x = 0; x < pageCount; x++)
								loadCbzPageIntoTexture(archPath.c_str(), x, &arraypage[x]);
						}
						if (ok) { statenow = readmanga; helppage = true; }
					}
					break;
				}
				std::string foldertoread = g_foldermain + getMangaName((size_t)selectchapter) + "/";
				arraychapter.clear();
				DIR* dir; struct dirent* ent;
				if ((dir = opendir(foldertoread.c_str())) != NULL) {
					while ((ent = readdir(dir)) != NULL) {
						if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
						std::string namefile(ent->d_name);
						if (namefile.find(".jpg") != std::string::npos || namefile.find(".jpeg") != std::string::npos ||
							namefile.find(".png") != std::string::npos || namefile.find(".bmp") != std::string::npos ||
							namefile.find(".gif") != std::string::npos || namefile.find(".webp") != std::string::npos)
							arraychapter.push_back(foldertoread + namefile);
					}
					std::sort(arraychapter.begin(), arraychapter.end(), naturalSortCompare);
					closedir(dir);
				}
				if (arraychapter.size() > 0) {
					int savedPage = loadReadingProgress(g_foldermain, foldertoread);
					if (savedPage >= 0 && savedPage < (int)arraychapter.size()) selectpage = savedPage;
					loadPageIntoTexture(arraychapter[selectpage], &Pagemanga);
					arraypage.resize(arraychapter.size());
					if (cascadeactivated)
						for (size_t x = 0; x < arraychapter.size(); x++)
							loadPageIntoTexture(arraychapter[x], &arraypage[x]);
					statenow = readmanga;
					helppage = true;
				}
			break;
		}
		}
	}
	}
	if (kDown & HidNpadButton_StickL) {
		switch (statenow) {
		case readmanga: helppage = !helppage; break;
		case selectmanga: helppage = !helppage; break;
		}
	}
	if (kDown & HidNpadButton_B) {
		switch (statenow) {
		case readmanga:
			statenow = selectmanga;
			if (g_mupdf_doc && g_mupdf_ctx) { fz_drop_document(g_mupdf_ctx, g_mupdf_doc); g_mupdf_doc = NULL; }
			g_mupdf_path.clear();
#ifdef CBR_SUPPORT
			closeCbr();
#endif
			Pagemanga.free();
			for (size_t x = 0; x < arraypage.size(); x++) arraypage[x].free();
			cascade = false; separation = false; adjust = true;
			basex = 0; basey = 0; neutralsize = 1; zoom = 1; selectpage = 0;
			break;
		case selectmanga: {
			const char* result = Keyboard_GetTextSearch("Search: type name or pattern", g_search_filter.c_str());
			g_search_filter = result ? result : "";
			rebuildSearchFilter();
			break;
		}
		}
	}
}
#endif

void handlePCInput(SDL_Event& e) {
#ifndef __SWITCH__
	if (g_search_input_mode) {
		if (e.type == SDL_KEYDOWN) {
			if (e.key.keysym.sym == SDLK_ESCAPE) {
				g_search_filter.clear();
				rebuildSearchFilter();
				g_search_input_mode = false;
				SDL_StopTextInput();
				return;
			}
			if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) {
				g_search_input_mode = false;
				SDL_StopTextInput();
				return;
			}
			if (e.key.keysym.sym == SDLK_BACKSPACE) {
				if (!g_search_filter.empty()) {
					g_search_filter.pop_back();
					rebuildSearchFilter();
				}
				return;
			}
		}
		if (e.type == SDL_TEXTINPUT) {
			g_search_filter += e.text.text;
			rebuildSearchFilter();
		}
		return;
	}
#endif
	if (e.type == SDL_QUIT) { cancelcurl = 1; quit = 1; }
	else if (e.type == SDL_KEYDOWN) {
		switch (e.key.keysym.sym) {
		case SDLK_DOWN:
			switch (statenow) {
			case readmanga:
				if (cascade == false) {
					if ((basey) > (g_view_h - heightnew)) {
						basey -= speed;
						if (basey <= g_view_h - heightnew) basey = g_view_h - heightnew;
					}
				} else basey -= speed;
				break;
			case selectmanga:
				g_cbz_open_error_msg.clear();
				if (selectchapter < (int)getMangaListCount() - 1) {
					selectchapter++; baseymain = baseymain - 20;
				} else { selectchapter = 0; baseymain = 30; }
				break;
			}
			break;
		case SDLK_UP:
			switch (statenow) {
			case readmanga:
				if (cascade == false) {
					if (basey <= 0) { basey += speed; if (basey > 0) basey = 0; }
				} else {
					if (basey <= 0) { basey += speed; if (basey > 0) basey = 0; }
				}
				break;
			case selectmanga:
				g_cbz_open_error_msg.clear();
				if (selectchapter > 0) { selectchapter--; baseymain = baseymain + 20; }
				else {
					size_t n = getMangaListCount();
					baseymain = (n > 0) ? (30 - 20 * ((int)n - 1)) : 30;
					selectchapter = (n > 0) ? (int)n - 1 : 0;
				}
				break;
			}
			break;
		case SDLK_a:
			if (existfoldermain) {
				switch (statenow) {
				case readmanga:
					if (cascade == false) { basey = 0; adjust = !adjust; } else adjust = !adjust;
					break;
				case selectmanga: {
					size_t n = getMangaListCount();
					if (n == 0 || selectchapter < 0 || (size_t)selectchapter >= n)
						break;
					if (!getMangaIsDir((size_t)selectchapter)) {
						g_cbz_open_error = CBZ_ERR_NONE; g_cbz_open_error_msg.clear();
						std::string fname = getMangaName((size_t)selectchapter);
						size_t start = fname.find_first_not_of(" \t\r\n");
						size_t end = fname.find_last_not_of(" \t\r\n");
						if (start != std::string::npos) fname = (end != std::string::npos) ? fname.substr(start, end - start + 1) : fname.substr(start);
						std::string archPath = g_foldermain + fname;
						for (size_t i = 0; i < archPath.size(); i++) if (archPath[i] == '\\') archPath[i] = '/';
						renderLoadingScreen();
#ifdef CBR_SUPPORT
						bool isCbr = (fname.size() >= 4 && fname.compare(fname.size() - 4, 4, ".cbr") == 0);
						if (isCbr) {
							int pageCount = listPagesFromCbr(archPath.c_str());
							if (pageCount <= 0) {
								if (g_cbz_open_error == CBZ_ERR_FILE_NOT_FOUND) g_cbz_open_error_msg = "File not found. Put CBR in sdmc:/CuteManga/";
								else if (g_cbz_open_error == CBZ_ERR_NOT_RAR) g_cbz_open_error_msg = "Not a valid CBR (RAR) file.";
								else g_cbz_open_error_msg = "No pages in CBR.";
							} else {
								arraychapter.clear();
								for (int i = 0; i < pageCount; i++) arraychapter.push_back(std::string("cbr:") + archPath + ":" + std::to_string(i));
								selectpage = 0;
								int savedPage = loadReadingProgress(g_foldermain, archPath);
								if (savedPage >= 0 && savedPage < pageCount) selectpage = savedPage;
								bool ok = loadCbrPageIntoTexture(archPath.c_str(), selectpage, &Pagemanga);
								arraypage.resize(arraychapter.size());
								if (cascadeactivated) for (int x = 0; x < pageCount; x++) loadCbrPageIntoTexture(archPath.c_str(), x, &arraypage[x]);
								if (ok) { statenow = readmanga; helppage = true; }
							}
							break;
						}
#endif
						bool isPdf = (fname.size() >= 4 && fname.compare(fname.size() - 4, 4, ".pdf") == 0);
						if (isPdf) {
							int pageCount = listPagesFromCbz(archPath.c_str());
							if (pageCount <= 0) {
								if (g_cbz_open_error == CBZ_ERR_FILE_NOT_FOUND) g_cbz_open_error_msg = "File not found. Put PDF in sdmc:/CuteManga/";
								else if (g_cbz_open_error == CBZ_ERR_NOT_ZIP) g_cbz_open_error_msg = "Not a valid PDF file.";
								else g_cbz_open_error_msg = "No pages in PDF.";
							} else {
								arraychapter.clear();
								for (int i = 0; i < pageCount; i++) arraychapter.push_back(std::string("pdf:") + archPath + ":" + std::to_string(i));
								selectpage = 0;
								int savedPage = loadReadingProgress(g_foldermain, archPath);
								if (savedPage >= 0 && savedPage < pageCount) selectpage = savedPage;
								bool ok = loadCbzPageIntoTexture(archPath.c_str(), selectpage, &Pagemanga);
								arraypage.resize(arraychapter.size());
								if (cascadeactivated) for (int x = 0; x < pageCount; x++) loadCbzPageIntoTexture(archPath.c_str(), x, &arraypage[x]);
								if (ok) { statenow = readmanga; helppage = true; }
							}
							break;
						}
						int pageCount = listPagesFromCbz(archPath.c_str());
						if (pageCount <= 0) {
							if (g_cbz_open_error == CBZ_ERR_FILE_NOT_FOUND) g_cbz_open_error_msg = "File not found. Put CBZ in sdmc:/CuteManga/";
							else if (g_cbz_open_error == CBZ_ERR_NOT_ZIP) {
								if (g_cbz_mupdf_error.empty()) g_cbz_open_error_msg = "Not a valid CBZ file.";
								else if (g_cbz_mupdf_error.find("cannot recognize archive") != std::string::npos) g_cbz_open_error_msg = "Not a ZIP. Use ZIP-based CBZ (not RAR/CBR).";
								else g_cbz_open_error_msg = "CBZ: " + g_cbz_mupdf_error;
							} else g_cbz_open_error_msg = "No pages in CBZ.";
						} else {
							arraychapter.clear();
							for (int i = 0; i < pageCount; i++) arraychapter.push_back(std::string("cbz:") + archPath + ":" + std::to_string(i));
							selectpage = 0;
							int savedPage = loadReadingProgress(g_foldermain, archPath);
							if (savedPage >= 0 && savedPage < pageCount) selectpage = savedPage;
							bool ok = loadCbzPageIntoTexture(archPath.c_str(), selectpage, &Pagemanga);
							arraypage.resize(arraychapter.size());
							if (cascadeactivated) for (int x = 0; x < pageCount; x++) loadCbzPageIntoTexture(archPath.c_str(), x, &arraypage[x]);
							if (ok) { statenow = readmanga; helppage = true; }
						}
						break;
					}
					std::string foldertoread = g_foldermain + getMangaName((size_t)selectchapter) + "/";
					arraychapter.clear();
					DIR* dir; struct dirent* ent;
					if ((dir = opendir(foldertoread.c_str())) != NULL) {
						while ((ent = readdir(dir)) != NULL) {
							if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
							std::string namefile(ent->d_name);
							if (namefile.find(".jpg") != std::string::npos || namefile.find(".jpeg") != std::string::npos ||
								namefile.find(".png") != std::string::npos || namefile.find(".bmp") != std::string::npos ||
								namefile.find(".gif") != std::string::npos || namefile.find(".webp") != std::string::npos)
								arraychapter.push_back(foldertoread + namefile);
						}
						std::sort(arraychapter.begin(), arraychapter.end(), naturalSortCompare);
						closedir(dir);
					}
					if (arraychapter.size() > 0) {
						int savedPage = loadReadingProgress(g_foldermain, foldertoread);
						if (savedPage >= 0 && savedPage < (int)arraychapter.size()) selectpage = savedPage;
						loadPageIntoTexture(arraychapter[selectpage], &Pagemanga);
						arraypage.resize(arraychapter.size());
						if (cascadeactivated) for (size_t x = 0; x < arraychapter.size(); x++) loadPageIntoTexture(arraychapter[x], &arraypage[x]);
						statenow = readmanga; helppage = true;
					}
					break;
				}
			}
			}
			break;
		case SDLK_MINUS: if (statenow == readmanga && neutralsize > 0.05f) neutralsize -= 0.05f; break;
		case SDLK_ESCAPE: quit = 1; break;
		case SDLK_y:
			g_portrait_mode = !g_portrait_mode;
			g_view_w = g_portrait_mode ? 720 : 1280;
			g_view_h = g_portrait_mode ? 1280 : 720;
			if (g_portrait_mode && g_portrait_texture == NULL && gRenderer != NULL)
				g_portrait_texture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 720, 1280);
			break;
		case SDLK_b:
			switch (statenow) {
			case readmanga:
				statenow = selectmanga;
				if (g_mupdf_doc && g_mupdf_ctx) { fz_drop_document(g_mupdf_ctx, g_mupdf_doc); g_mupdf_doc = NULL; }
				g_mupdf_path.clear();
#ifdef CBR_SUPPORT
				closeCbr();
#endif
				Pagemanga.free();
				for (size_t x = 0; x < arraypage.size(); x++) arraypage[x].free();
				cascade = false; separation = false; adjust = true;
				basex = 0; basey = 0; neutralsize = 1; zoom = 1; selectpage = 0;
				break;
			case selectmanga:
#ifndef __SWITCH__
				g_search_input_mode = true;
				SDL_StartTextInput();
#endif
				break;
			}
			break;
		case SDLK_l:
			switch (statenow) {
			case readmanga:
				if (cascade == false) {
					basey = 0; basex = 0; zoom = 1;
					if (selectpage > 0) { selectpage--; g_pending_page_load = true; }
					else { selectpage = 0; g_pending_page_load = true; }
				} else zoom = 1;
				break;
			}
			break;
		case SDLK_r:
			switch (statenow) {
			case readmanga:
				if (cascade == false) {
					basey = 0; basex = 0; zoom = 1;
					if (selectpage < (int)arraychapter.size() - 1) { selectpage++; g_pending_page_load = true; }
				} else zoom = 1;
				break;
			}
			break;
		case SDLK_w:
			switch (statenow) {
			case readmanga:
				if (cascade == false) {
					if (zoom == 1) zoom = 2; else { zoom = 1; basex = 0; basey = 0; }
				} else {
					if (zoom == 1) zoom = 2; else { zoom = 1; basex = 0; }
				}
				break;
			}
			break;
		case SDLK_LEFT: if (statenow == readmanga && zoom == 2) basex += speed; break;
		case SDLK_RIGHT: if (statenow == readmanga && zoom == 2) basex -= speed; break;
		case SDLK_m: if (statenow == readmanga) neutralsize += 0.05f; break;
		case SDLK_n: if (statenow == readmanga && neutralsize > 0.05f) neutralsize -= 0.05f; break;
		default: break;
		}
	}
}

void renderFrame() {
	if (g_portrait_mode && g_portrait_texture != NULL)
		SDL_SetRenderTarget(gRenderer, g_portrait_texture);
	SDL_SetRenderDrawColor(gRenderer, 233, 234, 237, 0xFF);
	SDL_RenderClear(gRenderer);
	int ycascade = 0;
	switch (statenow) {
	case readmanga:
		if (cascade == true) {
			for (size_t x = 0; x < arraypage.size(); x++) {
				if (adjust) {
					widthnew = g_view_w * zoom;
					heightnew = ((arraypage[x].getHeight() * g_view_w) / arraypage[x].getWidth()) * zoom;
					arraypage[x].customrender((g_view_w / 2 - widthnew / 2) + basex, ycascade + basey, widthnew, heightnew);
				} else {
					widthnew = (int)(arraypage[x].getWidth() * zoom * neutralsize);
					heightnew = (int)(arraypage[x].getHeight() * zoom * neutralsize);
					if (widthnew >= g_view_w) {
						widthnew = g_view_w * zoom;
						heightnew = ((arraypage[x].getHeight() * g_view_w) / arraypage[x].getWidth()) * zoom;
					}
					arraypage[x].customrender((g_view_w / 2 - widthnew / 2) + basex, ycascade + basey, widthnew, heightnew);
				}
				ycascade += separation ? heightnew + 10 : heightnew;
			}
		} else {
			if (adjust) {
				widthnew = g_view_w * zoom;
				heightnew = ((Pagemanga.getHeight() * g_view_w) / Pagemanga.getWidth()) * zoom;
				Pagemanga.customrender((g_view_w / 2 - widthnew / 2) + basex, 0 + basey, widthnew, heightnew);
			} else {
				widthnew = (int)(Pagemanga.getWidth() * zoom * neutralsize);
				heightnew = (int)(Pagemanga.getHeight() * zoom * neutralsize);
				if (widthnew >= g_view_w) {
					widthnew = g_view_w * zoom;
					heightnew = ((Pagemanga.getHeight() * g_view_w) / Pagemanga.getWidth()) * zoom;
				}
				Pagemanga.customrender((g_view_w / 2 - widthnew / 2) + basex, 0 + basey, widthnew, heightnew);
			}
		}
		{
			int totalPages = (int)arraychapter.size();
			int fy = g_view_h - 28, fw = 98, fx = g_view_w - fw - 12, leftFx = 12;
			if (g_pending_page_load) {
				int loadW = 78;
				SDL_Rect leftBg = { leftFx, fy - 2, loadW + 8, 22 };
				SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 230);
				SDL_RenderFillRect(gRenderer, &leftBg);
				SDL_SetRenderDrawColor(gRenderer, 200, 200, 200, 255);
				SDL_RenderDrawRect(gRenderer, &leftBg);
				gTextTexture.loadFromRenderedText(gFont, "Loading...", { 50, 50, 50 });
				gTextTexture.render(leftFx + 4, fy);
			}
			SDL_Rect footerBg = { fx, fy - 2, fw + 8, 22 };
			SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 230);
			SDL_RenderFillRect(gRenderer, &footerBg);
			SDL_SetRenderDrawColor(gRenderer, 200, 200, 200, 255);
			SDL_RenderDrawRect(gRenderer, &footerBg);
			char footerBuf[32];
			snprintf(footerBuf, sizeof(footerBuf), "Page %d/%d", selectpage + 1, totalPages > 0 ? totalPages : 1);
			gTextTexture.loadFromRenderedText(gFont, footerBuf, { 50, 50, 50 });
			gTextTexture.render(fx + 4, fy);
		}
		if (zoom == 2) lupa.render(g_view_w - lupa.getWidth() - 10, g_view_h - lupa.getHeight() - 10);
		if (helppage) {
			int helpW = (g_view_w > 540) ? 500 : (g_view_w - 40);
			int helpH = 220, helpX = (g_view_w - helpW) / 2, helpY = (g_view_h - helpH) / 2;
			SDL_Rect fillRect2 = { helpX - 2, helpY - 2, helpW + 4, helpH + 4 };
			SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 200);
			SDL_RenderFillRect(gRenderer, &fillRect2);
			SDL_Rect fillRect = { helpX, helpY, helpW, helpH };
			SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
			SDL_RenderFillRect(gRenderer, &fillRect);
			int tx = helpX + (helpW / 2), ty = helpY + 20;
			if (cascadeactivated)
				gTextTexture.loadFromRenderedText(gFont, "Press \"X\" for Cascade Mode (Separation or not).", { 0, 0, 0 });
			else
				gTextTexture.loadFromRenderedText(gFont, "Cascade is disabled, enable it in the Main Menu.", { 0, 0, 0 });
			gTextTexture.render(tx - gTextTexture.getWidth() / 2, ty);
			ty += 28;
			gTextTexture.loadFromRenderedText(gFont, "Press \"A\" for Fit Mode (On/Off).", { 0, 0, 0 });
			gTextTexture.render(tx - gTextTexture.getWidth() / 2, ty);
			ty += 28;
			gTextTexture.loadFromRenderedText(gFont, "Press \"L\" for Previous page and \"R\" for Next page.", { 0, 0, 0 });
			gTextTexture.render(tx - gTextTexture.getWidth() / 2, ty);
			ty += 28;
			gTextTexture.loadFromRenderedText(gFont, "Press \"ZL, ZR and R3\" for Zoom Mode.", { 0, 0, 0 });
			gTextTexture.render(tx - gTextTexture.getWidth() / 2, ty);
			ty += 28;
			gTextTexture.loadFromRenderedText(gFont, "Press \"L3\" for show/hide this.", { 0, 0, 0 });
			gTextTexture.render(tx - gTextTexture.getWidth() / 2, ty);
			ty += 28;
			gTextTexture.loadFromRenderedText(gFont, "Press \"Y\" for Portrait/Landscape.", { 0, 0, 0 });
			gTextTexture.render(tx - gTextTexture.getWidth() / 2, ty);
		}
		break;
	case selectmanga:
		Farest.render(0, 0);
		if (existfoldermain == false) {
			gTextTexture.loadFromRenderedText(gFont3, "Please create the folder called \"CuteManga\" in", { 0, 0, 0 });
			gTextTexture.render(g_view_w / 2 - gTextTexture.getWidth() / 2, 100);
			gTextTexture.loadFromRenderedText(gFont3, "your SD and put your \"Comic / Manga / Webtoon\" inside", { 0, 0, 0 });
			gTextTexture.render(g_view_w / 2 - gTextTexture.getWidth() / 2, 130);
			gTextTexture.loadFromRenderedText(gFont3, "in a folder without Latin characters.", { 0, 0, 0 });
			gTextTexture.render(g_view_w / 2 - gTextTexture.getWidth() / 2, 160);
		}
		{
			size_t listCount = getMangaListCount();
			for (size_t x = 0; x < listCount; x++) {
				int ypos = baseymain + ((int)x * 22);
				if ((int)x == selectchapter) {
					Heart.render(basexmain + 12, ypos);
					gTextTexture.loadFromRenderedText(gFont, getMangaName(x), { 120, 120, 120 });
					gTextTexture.render(basexmain + 30, ypos);
				} else {
					gTextTexture.loadFromRenderedText(gFont, getMangaName(x), { 50, 50, 50 });
					gTextTexture.render(basexmain, ypos);
				}
			}
		}
		{
			SDL_Rect fillRect = { 0, g_view_h - 35, 1280, 25 };
			SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
			SDL_RenderFillRect(gRenderer, &fillRect);
		}
		textColor = { 50, 50, 50 };
		if (!g_cbz_open_error_msg.empty()) {
			gTextTexture.loadFromRenderedText(gFont, g_cbz_open_error_msg.c_str(), textColor);
			gTextTexture.render(basexmain, g_view_h - 30);
		} else {
			bool sel_is_archive = (selectchapter < (int)getMangaListCount() && !getMangaIsDir((size_t)selectchapter));
			if (sel_is_archive) {
				std::string name = (selectchapter < (int)getMangaListCount()) ? getMangaName((size_t)selectchapter) : "";
				bool is_cbz = (name.size() >= 4 && name.compare(name.size() - 4, 4, ".cbz") == 0);
				bool is_pdf = (name.size() >= 4 && name.compare(name.size() - 4, 4, ".pdf") == 0);
#ifdef CBR_SUPPORT
				bool is_cbr = (name.size() >= 4 && name.compare(name.size() - 4, 4, ".cbr") == 0);
				bool is_archive_read = is_cbz || is_cbr || is_pdf;
#else
				bool is_archive_read = is_cbz || is_pdf;
#endif
				gTextTexture.loadFromRenderedText(gFont, is_archive_read ? "Press \"A\" to read. Press \"B\" to search. " : "Press \"A\" to open. Press \"B\" to search. ", textColor);
				gTextTexture.render(basexmain, g_view_h - 30);
			} else if (!cascadeactivated)
				gTextTexture.loadFromRenderedText(gFont, "Press \"A\" to read folder. Press \"B\" to search. ", textColor);
			else
				gTextTexture.loadFromRenderedText(gFont, "Press \"A\" to read folder. Press \"B\" to search. ", textColor);
			gTextTexture.render(basexmain, g_view_h - 30);
		}
		break;
	}
#ifndef __SWITCH__
	// PC: modal search dialog when typing
	if (g_search_input_mode && statenow == selectmanga) {
		SDL_Rect overlay = { 0, 0, g_view_w, g_view_h };
		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 180);
		SDL_RenderFillRect(gRenderer, &overlay);
		int boxW = 400, boxH = 80, boxX = (g_view_w - boxW) / 2, boxY = (g_view_h - boxH) / 2;
		SDL_Rect box = { boxX, boxY, boxW, boxH };
		SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
		SDL_RenderFillRect(gRenderer, &box);
		SDL_SetRenderDrawColor(gRenderer, 100, 100, 100, 255);
		SDL_RenderDrawRect(gRenderer, &box);
		std::string prompt = "Search: " + g_search_filter + "|";
		gTextTexture.loadFromRenderedText(gFont, prompt.c_str(), { 50, 50, 50 });
		gTextTexture.render(boxX + 8, boxY + (boxH - gTextTexture.getHeight()) / 2);
		gTextTexture.loadFromRenderedText(gFont, "Enter = done, Escape = cancel", { 80, 80, 80 });
		gTextTexture.render(boxX + 8, boxY + boxH - 18);
	}
#endif
	if (g_portrait_mode && g_portrait_texture != NULL) {
		SDL_SetRenderTarget(gRenderer, NULL);
		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
		SDL_RenderClear(gRenderer);
		SDL_Rect dest = { (1280 - 720) / 2, (720 - 1280) / 2, 720, 1280 };
		SDL_Point center = { 360, 640 };
		SDL_RenderCopyEx(gRenderer, g_portrait_texture, NULL, &dest, -90.0, &center, SDL_FLIP_NONE);
	}
	SDL_RenderPresent(gRenderer);
}

int main(int argc, char **argv)

{
#ifdef __SWITCH__
	romfsInit();
	socketInitializeDefault();
	nxlinkStdio();
	// Mount SD card so sdmc:/ paths work (required for CuteManga folder)
	fsdevMountSdmc();

#endif
	if (!initSDLAndWindow())
		return -1;

	std::string foldermain;
	if (!initFontsAndAssets(foldermain))
		return -1;

	loadFolderList(foldermain);
	g_foldermain = foldermain;

	SDL_Event e;
#ifdef __SWITCH__
	for (int i = 0; i < 2; i++) {
		if (SDL_JoystickOpen(i) == NULL) {
			SDL_Log("SDL_JoystickOpen: %s\n", SDL_GetError());
			SDL_Quit();
			return -1;
		}
	}


	// Configure our supported input layout: a single player with standard controller styles
	padConfigureInput(1, HidNpadStyleSet_NpadStandard);

	// Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
	PadState pad;
	padInitializeDefault(&pad);
#endif // SWITCH

	//While application is running
#ifdef __SWITCH__
	while ( appletMainLoop() && !quit)
#else
	while (!quit)
#endif // SWITCH
	{
#ifdef __SWITCH__
		padUpdate(&pad);
		u64 kDown = padGetButtonsDown(&pad);
		u64 kHeld = padGetButtons(&pad);
		handleSwitchInput(kDown, kHeld);
#endif
		while (SDL_PollEvent(&e))
			handlePCInput(e);
		renderFrame();
		doDeferredPageLoad(foldermain);
	}

	//Free resources and close SDL
#ifdef __SWITCH__
	socketExit();
	romfsExit();
#endif // SWITCH


	close();

	return 0;

}
