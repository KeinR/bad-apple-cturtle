 /* 
 * License: GNU GPL 3.0 or any later version
 * Author: Orion Musselman 
 */

/*
* TODO:
* - Sound
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
// #include <mutex>
#include <deque>
#include <atomic>

#define FRAME_WIDTH 480
#define FRAME_HEIGHT 360
#define FRAME_SIZE (FRAME_WIDTH * FRAME_HEIGHT)
#define NUM_FRAMES 6572

#define NDEBUG

#define MAX_QUALITY_LOSS 10

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


		std::time_t timeBegin = millis();
																													// File ID is one more than index
		snprintf(pathBuffer.data(), pathBuffer.size(), "C:\\Users\\musselmano\\source\\repos\\l1-loopy-cturtles-KeinR\\frames\\%.4i.png", f + 1);
		int ix, iy;
		unsigned char* img = stbi_load(pathBuffer.data(), &ix, &iy, nullptr, c);
		if (img == nullptr) {
			// Well, it seems we're at the end of the road
			// I think
			// Bad idea?
			std::cout << "Exiting early, load failed.\n";
			std::cout << stbi_failure_reason();
			break;
		}
		if (ix != FRAME_WIDTH || iy != FRAME_HEIGHT) {
			std::cerr << "CRITICAL: Invalid frame size\n";
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

				paths.push_back((path));
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
				pathsPruned.push_back((pruned));
			}
		}

		// std::cout << "Thread #" << std::this_thread::get_id() << " here, Paths = " << paths.size() << ", frame #" << f << " in " << (millis() - timeBegin) << "ms" << '\n';
		s->frames[f].path = pathsPruned;
		s->frames[f].done.store(true);
	}
}

int main() {

	ct::TurtleScreen screen(FRAME_WIDTH, FRAME_HEIGHT);
	screen.tracer(0, 0);
	screen.bgcolor({ "white" });
	ct::Turtle t1(screen);
	ct::Turtle t2(screen);
	std::array<ct::Turtle*, 2> tdbuf = {&t1, &t2};
	int ti = 0;

	state_t state;
	state.currentFrame.store(0);
	state.alive.store(true);
	for (frame_t& f : state.frames) {
		f.taken.store(false);
		f.done.store(false);
	}

	std::thread a(worker, &state);
	std::thread b(worker, &state);
	std::thread c(worker, &state);
	std::thread d(worker, &state);

	// 30 frames per second from original video
	constexpr std::time_t millisPerFrame = 1000 / 30;
	std::time_t nextFrameTime = 0;

	for (int f = 0; f < NUM_FRAMES; f++) {
		while (nextFrameTime > millis() || !state.frames[f].done.load());
		nextFrameTime = millis() + millisPerFrame;
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

	return 0;
}
