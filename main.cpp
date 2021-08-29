 /* 
 * License: GNU GPL 3.0 or any later version
 * Author: Orion Musselman 
 */

/*
* TODO:
* - Fix disconnecting line ends when quality loss is high 
* - Speed up CTurtle drawing
*/

#define STB_IMAGE_IMPLEMENTATION

#include "include/stb_image.h"
#include "include/CTurtle.hpp";

#include <cstdio>
#include <cmath>
#include <array>
#include <vector>
#include <iostream>
#include <utility>

#define FRAME_WIDTH 480
#define FRAME_HEIGHT 360
#define FRAME_SIZE (FRAME_WIDTH * FRAME_HEIGHT)

#define NDEBUG

#define MAX_QUALITY_LOSS 40 

struct vec_t {
	double x;
	double y;
};

struct line_t {
	double m;
	double b;
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

void toCturtle(int i, int* x, int* y) {
	*x = i % FRAME_WIDTH - FRAME_WIDTH / 2;
	*y = FRAME_HEIGHT - (i / FRAME_WIDTH + FRAME_HEIGHT / 2);
}

void toCoords(int i, double* x, double* y) {
	*x = i % FRAME_WIDTH;
	*y = i / FRAME_WIDTH;
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

int main() {

	ct::TurtleScreen screen(FRAME_WIDTH, FRAME_HEIGHT);
	screen.bgcolor({ "white" });
	ct::Turtle t1(screen);
	ct::Turtle t2(screen);
	std::array<ct::Turtle*, 2> tdbuf = {&t1, &t2};
	int ti = 0;

	
	std::array<bool, FRAME_SIZE> setMap;
	std::array<char, 128> pathBuffer;
	std::vector<std::vector<int>> paths;
	std::vector<std::vector<int>> pathsPruned;

	for (int f = 1;; f++) {
		snprintf(pathBuffer.data(), pathBuffer.size(), "C:\\Users\\musselmano\\source\\repos\\l1-loopy-cturtles-KeinR\\frames\\%.4i.png", f);
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
			return 1;
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

		std::cout << (1 / std::nextafter(0, 1.0)) << "Begin draw...\n";

		if (paths.size() == 0) continue;

		
		// Prune path length
		for (std::vector<int>& p : paths) {
			if (p.size() <= 1) continue;
			std::vector<int> pruned;

			vec_t lo, enp;
			line_t line;
			mkLocus(p, 0, &lo, &enp, &line);
			int cx = 0;
			
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


				if (loss > MAX_QUALITY_LOSS || i + 1 > p.size()) {
					pruned.push_back(p[i]);
					if (i + 1 < p.size()) {
						mkLocus(p, i, &lo, &enp, &line);
						cx = i;
						i++;
					}
				}
			}
			if (pruned.size() > 1) {
				pathsPruned.push_back((pruned));
			}
		}

		int tin = (ti + 1) % tdbuf.size();
		ct::Turtle& t = *tdbuf[tin];

		for (std::vector<int>& p : pathsPruned) {
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

		std::cout << "Paths = " << paths.size() << ", frame #" << f << '\n';
	}

	return 0;
}
