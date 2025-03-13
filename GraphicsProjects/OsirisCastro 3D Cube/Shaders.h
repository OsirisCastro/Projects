#pragma once
#include "MyMath.h"


void (*VertexShader)(Vertex&) = 0;
void (*PixelShader)(Pixel&) = 0;

Matrix4x SV_WorldMatrix;
Matrix4x SV_View;
Matrix4x SV_Projection;


void VS_World(Vertex& multiplyMe) {
	Vertex W = MultiplyMatrixByVertex(SV_WorldMatrix, multiplyMe);
	Vertex V = MultiplyMatrixByVertex(SV_View, W);
	Vertex P = MultiplyMatrixByVertex(SV_Projection, V);
	multiplyMe = Perspective(P);

}

void PS_White(Pixel& makeWhite) {
	makeWhite = 0xffffffff;
}

void PS_Green(Pixel& makeGreen) {
	makeGreen = 0xff00ff00;
}