#pragma once
#include "Shaders.h"

void plotPixel(int x, int y, unsigned int color) {
	int arrPoint = y * Screen_Width + x;
	Pixel_Array[arrPoint] = color;
}


void drawPixel(int x, int y, unsigned int color, float depth) {
	if (depth < Depth_Buffer[y * Screen_Width + x]) {

		Depth_Buffer[y * Screen_Width + x] = depth;

		Pixel_Array[y * Screen_Width + x] = color;
	}
}


void ParametricLine(unsigned int _x, unsigned int _y, unsigned int _x1, unsigned int _y1, unsigned int color) {
	int dx = static_cast<int>(_x1) - static_cast<int>(_x);
	int dy = static_cast<int>(_y1) - static_cast<int>(_y);
	unsigned int steps = max(abs(dx), abs(dy));
	for (unsigned int i = 0; i <= steps; ++i) {
		float t = static_cast<float>(i) / steps;
		float interpX = _x + t * dx;
		float interpY = _y + t * dy;
		unsigned int pixelX = static_cast<unsigned int>(round(interpX));
		unsigned int pixelY = static_cast<unsigned int>(round(interpY));
		if (pixelX < Screen_Width && pixelY < Screen_Height) {
			plotPixel(pixelX, pixelY, color);
		}
	}
}


void DrawLine(const Vertex& start, const Vertex& end, unsigned int currColor = 0xffffffff)
{
	Vertex copy_start = start;
	Vertex copy_end = end;
	if (VertexShader) {
		VertexShader(copy_start);
		VertexShader(copy_end);
	}
	Pixel copyColor;
	copyColor = currColor;

	if (PixelShader) {
		PixelShader(copyColor);
	}

	PointXY Screen_Start = NDC2Screen(copy_start);
	PointXY Screen_End = NDC2Screen(copy_end);
	ParametricLine(Screen_Start.x, Screen_Start.y, Screen_End.x, Screen_End.y, copyColor);
}


std::vector<Vertex> GenerateGridVertices() {
	std::vector<Vertex> gridVertices;
	const int gridSize = 10;
	const int numLines = gridSize + 1;
	float espacing = 1.0f / gridSize;
	for (int i = 0; i < numLines; ++i) {
		float z = -0.5f + i * espacing;
		gridVertices.push_back({ -0.5f, 0.0f, z, 1 });
		gridVertices.push_back({ 0.5f, 0.f, z, 1 });
	}
	for (int i = 0; i < numLines; ++i) {
		float x = -0.5f + i * espacing;
		gridVertices.push_back({ x, 0.0f, -0.5f, 1 });
		gridVertices.push_back({ x, 0.0f, 0.5f, 1 });
	}

	return gridVertices;
}


void DrawGrid() {
	std::vector<Vertex> gridLines;
	gridLines = GenerateGridVertices();
	for (int i = 0; i < gridLines.size(); i += 2) {
		if (i + 1 < gridLines.size()) {
			Vertex& start = gridLines[i];
			Vertex& end = gridLines[i + 1];
			DrawLine(start, end);
		}
	}
}


void DrawCube(Vertex arr[]) {

	DrawLine(arr[0], arr[1]);
	DrawLine(arr[1], arr[2]);
	DrawLine(arr[2], arr[3]);
	DrawLine(arr[3], arr[0]);
	DrawLine(arr[4], arr[5]);
	DrawLine(arr[5], arr[6]);
	DrawLine(arr[6], arr[7]);
	DrawLine(arr[7], arr[4]);
	DrawLine(arr[0], arr[4]);
	DrawLine(arr[1], arr[5]);
	DrawLine(arr[2], arr[6]);
	DrawLine(arr[3], arr[7]);
}





