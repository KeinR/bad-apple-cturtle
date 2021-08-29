 /* 
 * License: GNU GPL 3.0 or any later version
 * Author: Orion Musselman 
 */

#define STB_IMAGE_IMPLEMENTATION

#include "include/stb_image.h"
#include "include/CTurtle.hpp";

#include <cstdio>
#include <array>
#include <vector>
#include <iostream>
#include <utility>

#define FRAME_WIDTH 480
#define FRAME_HEIGHT 360
#define FRAME_SIZE (FRAME_WIDTH * FRAME_HEIGHT)

#define NDEBUG

namespace ct = cturtle;

std::array<std::pair<int, int>, 8> matrix = {
	std::pair<int, int>(1, 1),
	std::pair<int, int>(1, -1),
	std::pair<int, int>(-1, 1),
	std::pair<int, int>(-1, -1),
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

bool isBlack(int r) {
	return r < 10;
}

// Get Adjacent differ ONLY IF this node is black
bool getAdjacentDiffer(unsigned char* img, int i) {
	int x = i % FRAME_WIDTH;
	int y = i / FRAME_WIDTH;
	bool black = isBlack(img[i * c]);
	// We only target black nodes, to prevent jaggidynessssrewgww
	if (!black) return false;
	for (std::size_t m = 0; m < matrix.size(); m++) {
		std::pair<int, int> p(x + matrix[m].first, y + matrix[m].second);
		if (p.first >= 0 && p.second >= 0 && p.first < FRAME_WIDTH && p.second < FRAME_HEIGHT &&
			black != isBlack(img[(p.first + p.second * FRAME_WIDTH) * c])) {
			return true;
		}
	}
	return false;
}

std::vector<int> getAdjacent(int i) {
	int x = i % FRAME_WIDTH;
	int y = i / FRAME_WIDTH;
	std::vector<int> result;
	result.reserve(8);
	for (std::size_t m = 0; m < matrix.size(); m++) {
		std::pair<int, int> p(x + matrix[m].first, y + matrix[m].second);
		if (p.first >= 0 && p.second >= 0 && p.first < FRAME_WIDTH && p.second < FRAME_HEIGHT) {
			result.push_back(p.first + p.second * FRAME_WIDTH);
		}
	}
	return result;
}

int main() {

	ct::TurtleScreen screen(FRAME_WIDTH, FRAME_HEIGHT);
	screen.bgcolor({ "white" });
	ct::Turtle t(screen);

	std::array<bool, FRAME_SIZE> setMap;
	std::array<char, 128> pathBuffer;
	std::vector<std::vector<int>> paths;

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
					for (int n : getAdjacent(i)) {
						if (!setMap[n] && getAdjacentDiffer(img, i)) {
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

		std::cout << "Begin draw...\n";

		t.reset();
		t.hideturtle();
		t.speed(ct::TS_FASTEST);

		for (std::vector<int>& p : paths) {
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

		std::cout << "Paths = " << paths.size() << ", frame #" << f << '\n';
	}

	return 0;
}
