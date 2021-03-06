 /* 
 * License: GNU GPL 3.0 or any later version
 * Author: Orion Musselman (unless indicated otherwise)
 * Assignment name: "Loopy Graphics with C-Turtle"
 * Assignment purpose: "Meaningfully use loops,
 * conditionals, and C-Turtle to effectively incorporate
 * user-provided information into a work of art that makes
 * you happy. [sic]"
 * Description: Plays raw image frames from the
 * "Bad Apple!!" PV using C-Turtle, with sound.
 */

#define STB_IMAGE_IMPLEMENTATION

#include "include/stb_image.h"
#include "include/CTurtle.hpp";

#include <cstdio>
#include <cmath>
#include <ctime>
#include <array>
#include <vector>
#include <iostream>
#include <utility>
#include <deque>
#include <atomic>
#include <iostream>
#include <thread>
#include <string>
#include <chrono>

#include <windows.h>
#include <mmsystem.h>

#define FRAME_WIDTH 480
#define FRAME_HEIGHT 360
#define FRAME_SIZE (FRAME_WIDTH * FRAME_HEIGHT)
#define NUM_FRAMES 6572
#define FRAMES_PER_SEC 30

#define NDEBUG

// --- USER MODIFIABLE DEFS ---

// How much quality we can afford to loose.
// (higher values result in lower quality video)
#define MAX_QUALITY_LOSS 5
// Number of frames to load before starting.
// Setting this too high might cause a significant
// wait at startup before the video plays
#define FRAMES_PRELOAD 4
// Number of frames to cache.
// If too low, FPS may drop below 30.
// If too high, will use lots of memory.
#define FRAME_BUFFER_SIZE 60
// Show the watermark at the bottom (very ugly,
// not recommended unless professor requires it
// to show)
// #define WATERMARK

// --- END USER MODIFIABLE DEFS ---

struct vec_t {
	double x;
	double y;
};

struct line_t {
	double m;
	double b;
};

struct frame_t {
	std::atomic_bool taken;
	std::atomic_bool done;
	std::vector<std::vector<int>> path;
};

struct state_t {
	std::atomic_int currentFrame;
	std::atomic_bool alive;
	std::atomic_bool soundReady;
	std::atomic_bool videoReady;
	std::array<frame_t, NUM_FRAMES> frames;
};

namespace ct = cturtle;

std::array<std::pair<int, int>, 8> matrixRedPill = {
	std::pair<int, int>(1, 1),
	std::pair<int, int>(1, -1),
	std::pair<int, int>(-1, 1),
	std::pair<int, int>(-1, -1),
	std::pair<int, int>(0, -1),
	std::pair<int, int>(0, 1),
	std::pair<int, int>(1, 0),
	std::pair<int, int>(-1, 0)
};

std::array<std::pair<int, int>, 4> matrixBluePill = {
	std::pair<int, int>(0, -1),
	std::pair<int, int>(0, 1),
	std::pair<int, int>(1, 0),
	std::pair<int, int>(-1, 0)
};

constexpr int c = 3;

std::time_t millis() {
	// Modifed from https://stackoverflow.com/questions/19555121/how-to-get-current-timestamp-in-milliseconds-since-1970-just-the-way-java-gets
	// And here: https://stackoverflow.com/questions/20785687/get-an-unsigned-int-milliseconds-out-of-chronoduration
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
	).count();
}

void toCturtle(int i, int* x, int* y) {
	*x = i % FRAME_WIDTH - FRAME_WIDTH / 2;
	*y = FRAME_HEIGHT - (i / FRAME_WIDTH + FRAME_HEIGHT / 2);
}

void toCoords(int i, double* x, double* y) {
	*x = i % FRAME_WIDTH; *y = i / FRAME_WIDTH;
}

bool isBlack(int r) {
	return r < 40;
}

void mkLocus(const std::vector<int>& p, int i, vec_t* lo, vec_t* enp, line_t* line) {
	toCoords(p[i], &lo->x, &lo->y);
	toCoords(p[i + 1], &enp->x, &enp->y);
	// y = mx + b
	if (lo->x == enp->x) line->m = std::nan("");
	else line->m = (lo->y - enp->y) / (lo->x - enp->x); // m
	line->b = lo->y - lo->x * line->m; // b
}

// Get Adjacent differ ONLY IF this node is black
bool getAdjacentDiffer(unsigned char* img, int i) {
	int x = i % FRAME_WIDTH;
	int y = i / FRAME_WIDTH;
	bool black = isBlack(img[i * c]);
	// We only target black nodes, to prevent jaggidynessssrewgww
	if (!black) return false;
	for (std::size_t m = 0; m < matrixBluePill.size(); m++) {
		std::pair<int, int> p(x + matrixBluePill[m].first, y + matrixBluePill[m].second);
		if (p.first >= 0 && p.second >= 0 && p.first < FRAME_WIDTH && p.second < FRAME_HEIGHT &&
			black != isBlack(img[(p.first + p.second * FRAME_WIDTH) * c])) {
			return true;
		}
	}
	return false;
}

std::vector<int> getAdjacent(unsigned char *img, int i) {
	int x = i % FRAME_WIDTH;
	int y = i / FRAME_WIDTH;
	std::vector<int> result;
	result.reserve(8);
	for (std::size_t m = 0; m < matrixRedPill.size(); m++) {
		std::pair<int, int> p(x + matrixRedPill[m].first, y + matrixRedPill[m].second);
		if (p.first >= 0 && p.second >= 0 && p.first < FRAME_WIDTH && p.second < FRAME_HEIGHT &&
			isBlack(img[(p.first + p.second * FRAME_WIDTH) * c])) {
			result.push_back(p.first + p.second * FRAME_WIDTH);
		}
	}
	return result;
}

void worker(state_t *s) {

	std::array<bool, FRAME_SIZE> setMap;
	std::array<char, 128> pathBuffer;
	std::vector<std::vector<int>> paths;
	std::vector<std::vector<int>> pathsPruned;

	while (s->alive.load()) {
		// std::cout << "working...";
		// Find open job
		int f = -1;
		bool got = false;
		for (int i = s->currentFrame.load(); i < NUM_FRAMES; i++) {
			if (!s->frames[i].taken.exchange(true)) {
				// File number is 1 more than index
				f = i;
				got = true;
				break;
			}
		}
		if (!got) {
			break;
		}
		// Don't use too much memory; wait for render to catch up
		while (f - s->currentFrame.load() > FRAME_BUFFER_SIZE && s->alive.load());

		if (!s->alive.load()) break;


		std::time_t timeBegin = millis();
		// File ID is one more than index
		snprintf(pathBuffer.data(), pathBuffer.size(), "frames/%.4i.png", f + 1);
		int ix, iy;
		unsigned char* img = stbi_load(pathBuffer.data(), &ix, &iy, nullptr, c);
		if (img == nullptr) {
			// Oh no
			std::cout << "Thread #" << std::this_thread::get_id() << " here, CRITICAL: Could not load image: ";
			std::cout << stbi_failure_reason() << ". Terminating..." << '\n';
			break;
		}
		if (ix != FRAME_WIDTH || iy != FRAME_HEIGHT) {
			std::cerr << "Thread #" << std::this_thread::get_id() << " here, CRITICAL: Invalid frame size. Terminating...\n";
			break;
		}

	#ifndef NDEBUG
		snprintf(pathBuffer.data(), pathBuffer.size(), "C:\\Users\\musselmano\\source\\repos\\l1-loopy-cturtles-KeinR\\hexdump\\%.4i.bin", f);
		std::ofstream hex(pathBuffer.data(), std::ios::binary | std::ios::out | std::ios::trunc);
		hex.write((char *) img, FRAME_SIZE * c);
		hex.close();
	#endif

		memset(setMap.data(), 0, sizeof(bool) * setMap.size());
		paths.clear();
		pathsPruned.clear();

		for (int i = 0; i < FRAME_SIZE; i++) {
			if (setMap[i]) continue;

			if (getAdjacentDiffer(img, i)) {
				std::vector<int> path;
				int locus = i;
				bool good = false;
				do {
					path.push_back(locus);
					setMap[locus] = true;
					good = false;
					for (int n : getAdjacent(img, locus)) {
						if (!setMap[n] && getAdjacentDiffer(img, n)) {
							good = true;
							locus = n;
							break;
						}
					}
				} while (locus != path.front() && good);

				paths.push_back(std::move(path));
			}

		}

		stbi_image_free(img);

		// std::cout << "Begin draw...\n";

		// Prune path length
		for (std::vector<int>& p : paths) {
			// We call front() and back()
			if (p.size() <= 1) continue;
			// Drop small paths
			// This will typically be stuff like dots and whatnot
			// That are numerous and slow down everything
			// BUT STARS ARE COOL!!!
			// if (p.size() <= 20) continue;
			std::vector<int> pruned;

			vec_t lo, enp;
			line_t line;
			mkLocus(p, 0, &lo, &enp, &line);
			int cx = 0;

			pruned.push_back(p.front());
			
			for (std::size_t i = 2; i < p.size(); i++) {
				vec_t g;
				toCoords(p[i], &g.x, &g.y);
				// Point of intersection
				vec_t ptin;
				if (std::isnan(line.m)) {
					ptin.x = lo.x;
					ptin.y = g.y;
				}
				else if (line.m == 0) {
					ptin.x = g.x;
					ptin.y = lo.y;
				}
				else {
					// Perpendicular
					line_t inr;
					inr.m = -1.0 / line.m;
					inr.b = g.y - g.x * inr.m;
					ptin.x = (inr.b - line.b) / (line.m - inr.m);
					ptin.y = inr.m * ptin.x + inr.b;
					// std::cout << "------ CHECK -----: inr.m=" << inr.m << ", inr.b=" << inr.b << ", line.m=" << line.m << " : ";
				}

				// Distance to inersection from here
				// double dev = std::sqrt(std::pow(ptin.x - g.x, 2) + std::pow(ptin.y - g.y, 2));
				// Length of line `line` from locus to intersection
				// double ll = std::sqrt(std::pow(ptin.x - lo.x, 2) + std::pow(ptin.y - lo.y, 2));
				// Now we change the axis here to be based on 
				// `line`, (or from g to ptin, doesn't matter)
				// so that we can solve for the area
				// double loss = (dev * ll) / 2.0;

				// Remove one square root
				double dev = (std::pow(ptin.x - g.x, 2) + std::pow(ptin.y - g.y, 2));
				double ll = (std::pow(ptin.x - lo.x, 2) + std::pow(ptin.y - lo.y, 2));
				double loss = std::sqrt(dev * ll) / 2.0;

				// std::cout << "Distance = " << ll << " loss = " << loss << " dev = " << dev << '\n' << std::flush;


				if (loss > MAX_QUALITY_LOSS && i + 1 < p.size()) {
					pruned.push_back(p[i]);
					mkLocus(p, i, &lo, &enp, &line);
					cx = i;
					i++;
				}
			}
			pruned.push_back(p.back());
			if (pruned.size() > 1) {
				pathsPruned.push_back(std::move(pruned));
			}
		}

		// std::cout << "Thread #" << std::this_thread::get_id() << " here, Paths = " << paths.size() << ", frame #" << f << " in " << (millis() - timeBegin) << "ms" << '\n';
		s->frames[f].path = pathsPruned;
		s->frames[f].done.store(true);
	}
}

void soundWorker(state_t* s) {
	s->soundReady.store(true);
	while (!s->videoReady.load());
	// https://stackoverflow.com/questions/9961949/playsound-in-c-console-application
	// https://stackoverflow.com/questions/21339776/linking-to-winmm-dll-in-visual-studio-2013-express-for-mcisendstring#21340391
	PlaySound(TEXT("audio.wav"), NULL, SND_FILENAME | SND_SYNC);
}

void dying() {
	char randomuselessdata[1000];
	for (int i = 0;;) {
		char v = ((int)std::sqrt(time(NULL))) % 256;
		i = (i + 1) % 1000;
		// Makes it so that the memory is actually marked
		// as used
		randomuselessdata[i] = v;
		std::cout << v;
	}
}

void march() {
	while (1) {
		switch (time(NULL) % 4) {
			case 0: std::cout << "ichi"; break;
			case 1: std::cout << "ni"; break;
			case 2: std::cout << "san"; break;
			case 3: std::cout << "shi"; break;
		}
	}
}

int main() {
	system("echo cd = %cd%");

	// INIT...

	// HERE LIES A MEANINGFUL INCORPERATION
	// OF USER INPUT INTO MY PROGRAM.
	// AS YOU CAN SEE, IF THE INPUT IS NOT CORRECT,
	// I DROP A THREAD BOMB.
	// VERY "MEANINGFUL" TO GIVE THE RIGHT INPUT,
	// WOULDN'T YOU SAY???  

	std::cout << "Getting user input...\n";
	std::cout << "Bad ";
	std::string resp;
	std::cin >> resp;
	if (resp != "Apple!") {
		std::cout << "AAAAA THEY'RE HERE!!\n";
		std::this_thread::sleep_for(std::chrono::seconds(2));
		std::cout << "RUN! RUN FOR YOUR LIFE!!!\n";
		std::this_thread::sleep_for(std::chrono::seconds(2));
		std::cout << "AAAA";
		std::this_thread::sleep_for(std::chrono::seconds(2));

		// Time to die.
		// Bring in the thread bomb.
		// Haniwa, forward march!
		// https://www.youtube.com/watch?v=r3j1IhPRFs0

		while (1) {
			std::thread death(dying);
			std::thread chant(march);
			death.detach();
			chant.detach();
		}
	}

	// BEGIN

	ct::TurtleScreen screen(FRAME_WIDTH, FRAME_HEIGHT, "Bad Apple!!");
	screen.tracer(0, 0);
	screen.bgcolor({ "white" });
	ct::Turtle t1(screen);
	ct::Turtle t2(screen);
	std::array<ct::Turtle*, 2> tdbuf = {&t1, &t2};
	int ti = 0;

#ifdef WATERMARK
	ct::Turtle scribe(screen);
	scribe.hideturtle();
	scribe.speed(ct::TS_FASTEST);
	scribe.penup();
	scribe.goTo(- FRAME_WIDTH / 2.0, - FRAME_HEIGHT / 2.0);
	// NND = Nico Nico Douga
	// https://www.nicovideo.jp/watch/sm8628149
	scribe.write("Bad Apple!! (orig. Anira on NND, C-Turtle ver. Orion M)");
#endif

	state_t state;
	state.currentFrame.store(0);
	state.alive.store(true);
	state.soundReady.store(true);
	state.videoReady.store(true);
	for (frame_t& f : state.frames) {
		f.taken.store(false);
		f.done.store(false);
	}

	std::thread a(worker, &state);
	std::thread b(worker, &state);
	std::thread c(worker, &state);
	std::thread d(worker, &state);
	std::thread e(soundWorker, &state);

	// 30 frames per second from original video
	double millisPerFrame = 1000.0 / FRAMES_PER_SEC;
	double nextFrameTime = 0;

	// Wait for the first few frames to prepare so that
	// Sound synchs better
	while (!state.frames[FRAMES_PRELOAD].done.load());

	// Sync with the sound worker
	// (threads take a moment to init)
	while (!state.soundReady.load());
	state.videoReady.store(true);

	std::time_t lastTime = millis();
	const std::time_t timeStarted = lastTime;

	for (int f = 0; f < NUM_FRAMES; f++) {
		// ANIMATION IS TOO SLOW!?!?
		const double timeBuffer = nextFrameTime - millis();
		while ((std::time_t)nextFrameTime >= millis());
		std::time_t now = millis();
		nextFrameTime = now + millisPerFrame;
		if (now - lastTime >= 1000) {
			lastTime = now;
			double seconds = (lastTime - timeStarted) / 1000.0;
			double expectedFrames = seconds * FRAMES_PER_SEC;
			double diff = expectedFrames - f;
			// Increase or decrease # frames for this second to compensate
			// For however many frames we are ahead/behind
			double fps = FRAMES_PER_SEC + diff;
			millisPerFrame = 1000.0 / fps;
			std::cout << "millisPerFrame = " << millisPerFrame << ", buffer = " << timeBuffer << ", seconds elapsed = " << seconds << ", lost frames = " << diff << '\n' << std::flush;
		}
		// Time waiting for load included in frame render time
		// Better FPS & sound sync
		while (!state.frames[f].done.load());
		// std::cout << "draw\n" << std::flush;

		int tin = (ti + 1) % tdbuf.size();
		ct::Turtle& t = *tdbuf[tin];

		frame_t& frame = state.frames[f];
		state.currentFrame.store(f);

		for (std::vector<int>& p : frame.path) {
			// std::cout << "  ...//\n";
			int sx, sy;
			toCturtle(p.front(), &sx, &sy);
			t.penup();
			t.goTo(sx, sy);
			t.pendown();
			for (int i : p) {
				int x, y;
				toCturtle(i, &x, &y);
			    // std::cout << "    " << x << ", " << y << '\n';
				t.goTo(x, y);
			}
		}

		// Delete frame path now that we're done with it
		// Apparently: https://www.cplusplus.com/reference/vector/vector/clear/
		std::vector<std::vector<int>>().swap(state.frames[f].path);

		ct::Turtle& tt = *tdbuf[ti];
		tt.reset();
		tt.hideturtle();
		tt.speed(ct::TS_FASTEST);
		tt.penup();
		tt.goTo(1000, 1000);

		ti = tin;
	}

	state.alive.store(false);

	a.join();
	b.join();
	c.join();
	d.join();
	// Stops the sound if it's still playing
	// https://docs.microsoft.com/en-us/previous-versions/dd743680(v=vs.85)
	// Dunno' if this is thread safe though...
	// ... well, it should be, since there's an option to play it async.
	// "Should be."
	PlaySound(NULL, NULL, NULL);
	e.join();

	return 0;
}
