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

unsigned BGRAtoARGB(unsigned pixel) {
	unsigned int alpha = pixel & 0x000000ff;
	unsigned int red = (pixel & 0x0000ff00) >> 8;
	unsigned int green = (pixel & 0x00ff0000) >> 16;
	unsigned int blue = (pixel & 0xff000000) >> 24;
	unsigned int ARGB = (alpha << 24) | (red << 16) | (green << 8) | (blue);
	return ARGB;
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
	float spacing = 1.0f / gridSize;

	for (int i = 0; i < numLines; ++i) {
		float z = -0.5f + i * spacing;
		gridVertices.push_back({ -0.5f, 0.0f, z, 1 });
		gridVertices.push_back({ 0.5f, 0.f, z, 1 });
	}

	for (int i = 0; i < numLines; ++i) {
		float x = -0.5f + i * spacing;
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

void FillTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2, Pixel color) {
	Vertex copyV0 = v0;
	Vertex copyV1 = v1;
	Vertex copyV2 = v2;
	Pixel copyColor = color;
	VertexShader(copyV0);
	VertexShader(copyV1);
	VertexShader(copyV2);
	PointXY p0 = NDC2Screen(copyV0);
	PointXY p1 = NDC2Screen(copyV1);
	PointXY p2 = NDC2Screen(copyV2);
	float z0 = copyV0.z;
	float z1 = copyV1.z;
	float z2 = copyV2.z;
	for (int y = 0; y <= Screen_Height; ++y) {
		for (int x = 0; x <= Screen_Width; ++x) {
			PointXY P = { x, y };

			float denom = (p1.y - p2.y) * (p0.x - p2.x) + (p2.x - p1.x) * (p0.y - p2.y);
			float alpha = ((p1.y - p2.y) * (P.x - p2.x) + (p2.x - p1.x) * (P.y - p2.y)) / denom;
			float beta = ((p2.y - p0.y) * (P.x - p2.x) + (p0.x - p2.x) * (P.y - p2.y)) / denom;
			float gamma = 1.0f - alpha - beta;

			if (alpha >= 0 && beta >= 0 && gamma >= 0) {
				float interpolatedZ = z0 * alpha + z1 * beta + z2 * gamma;
				if (cubeChange == 2) {
					drawPixel(x, y, color, int(interpolatedZ));
				}
				if (cubeChange == 3) {
					if (interpolatedZ < Depth_Buffer[y * Screen_Width + x]) {
						drawPixel(x, y, color, interpolatedZ);
					}

				}
			}
		}
	}
}

void ImageTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2) {

	Vertex copyV0 = v0;
	Vertex copyV1 = v1;
	Vertex copyV2 = v2;
	VertexShader(copyV0);
	VertexShader(copyV1);
	VertexShader(copyV2);
	PointXY p0 = NDC2Screen(copyV0);
	PointXY p1 = NDC2Screen(copyV1);
	PointXY p2 = NDC2Screen(copyV2);
	float z0 = copyV0.z;
	float z1 = copyV1.z;
	float z2 = copyV2.z;
	for (int y = 0; y <= Screen_Height; ++y) {
		for (int x = 0; x <= Screen_Width; ++x) {
			PointXY P = { x, y };

			float denom = (p1.y - p2.y) * (p0.x - p2.x) + (p2.x - p1.x) * (p0.y - p2.y);
			float alpha = ((p1.y - p2.y) * (P.x - p2.x) + (p2.x - p1.x) * (P.y - p2.y)) / denom;
			float beta = ((p2.y - p0.y) * (P.x - p2.x) + (p0.x - p2.x) * (P.y - p2.y)) / denom;
			float gamma = 1.0f - alpha - beta;

			if (alpha >= 0 && beta >= 0 && gamma >= 0) {
				float interpolatedZ = z0 * alpha + z1 * beta + z2 * gamma;
				float u = v0.u * alpha + v1.u * beta + v2.u * gamma;
				float v = v0.v * alpha + v1.v * beta + v2.v * gamma;

				int texX = static_cast<int>(max(0.0f, min(u * (treeolife_width - 1), static_cast<float>(treeolife_width - 1))));
				int texY = static_cast<int>(max(0.0f, min(v * (treeolife_height - 1), static_cast<float>(treeolife_height - 1))));

				unsigned int texColor = treeolife_pixels[texY * treeolife_width + texX];

				unsigned int r = (texColor & 0x0000ff00) >> 8;
				unsigned int g = (texColor & 0x00ff0000) >> 16;
				unsigned int b = (texColor & 0xff000000) >> 24;
				unsigned int a = (texColor & 0x000000ff);

				unsigned int finalA = a << 24;
				unsigned int finalR = r << 16;
				unsigned int finalG = g << 8;
				unsigned int finalB = b;

				unsigned int finalColor = finalA | finalR | finalG | finalB;

				if (interpolatedZ < Depth_Buffer[y * Screen_Width + x]) {
					drawPixel(x, y, finalColor, interpolatedZ);
				}
			}
		}
	}
}

void DrawColoredCube(Vertex arr[]) {
	FillTriangle(arr[0], arr[1], arr[2], 0xffff0000);
	FillTriangle(arr[0], arr[2], arr[3], 0xffff0000);

	FillTriangle(arr[4], arr[5], arr[6], 0xff00ff00);
	FillTriangle(arr[4], arr[6], arr[7], 0xff00ff00);

	FillTriangle(arr[0], arr[3], arr[7], 0xff0000ff);
	FillTriangle(arr[0], arr[7], arr[4], 0xff0000ff);

	FillTriangle(arr[1], arr[2], arr[6], 0xffffff00);
	FillTriangle(arr[1], arr[6], arr[5], 0xffffff00);

	FillTriangle(arr[0], arr[1], arr[5], 0xffff00ff);
	FillTriangle(arr[0], arr[5], arr[4], 0xffff00ff);

	FillTriangle(arr[2], arr[3], arr[7], 0xffff00ff);
	FillTriangle(arr[2], arr[7], arr[6], 0xffff00ff);
}

void DrawImageCube(Vertex arr[]) {
	ImageTriangle(arr[0], arr[1], arr[2]);
	ImageTriangle(arr[0], arr[2], arr[3]);

	ImageTriangle(arr[4], arr[5], arr[6]);
	ImageTriangle(arr[4], arr[6], arr[7]);

	ImageTriangle(arr[8], arr[9], arr[10]);
	ImageTriangle(arr[8], arr[10], arr[11]);

	ImageTriangle(arr[12], arr[13], arr[14]);
	ImageTriangle(arr[12], arr[14], arr[15]);

}
