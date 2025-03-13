#pragma once
#include <iostream>
#include <cmath>
#include <vector>
#include "Windows.h"

#define Screen_Width 500
#define Screen_Height 500
#define NumPixels (Screen_Width * Screen_Height)
unsigned int Pixel_Array[NumPixels];
float* Depth_Buffer;
int cubeChange = 0;
typedef unsigned int Pixel;

struct PointXY
{
	int x;
	int y;
};


struct Vertex
{
	float x;
	float y;
	float z;
	float w;
	unsigned int color;
	float u;
	float v;
};


struct Matrix4x {
	Vertex  rowX;
	Vertex	rowY;
	Vertex	rowZ;
	Vertex	rowW;

	Vertex ColumnX() const {
		return { rowX.x, rowY.x, rowZ.x, rowW.x };
	}
	Vertex ColumnY() const {
		return { rowX.y, rowY.y, rowZ.y, rowW.y };
	}
	Vertex ColumnZ() const {
		return { rowX.z, rowY.z, rowZ.z, rowW.z };
	}
	Vertex ColumnW() const {
		return { rowX.w, rowY.w, rowZ.w, rowW.w };
	}
};