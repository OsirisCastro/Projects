#pragma once
#include"RasterSurface.h"
#include "RasterFunc.h"



int main() {
    RS_Initialize("Osiris Castro 3D Cube", Screen_Width, Screen_Height);
    const Vertex UneditableOriginalLine[2] = { {-0.5, +0.5}, {+0.5, -0.5} };

    initializeZBuff();

    float rotationSpeed = .05f;


    Vertex cubeVertices[16] = {
        { -0.25f,  0.25f, -0.25f, 1.0f, 0xff00ff00, 1.0f, 0.0f },
        { 0.25f,   0.25f, -0.25f, 1.0f, 0xff00ff00, 0.0f, 0.0f },
        { 0.25f,  -0.25f, -0.25f, 1.0f, 0xff00ff00, 0.0f, 1.0f },
        { -0.25f, -0.25f, -0.25f, 1.0f, 0xff00ff00, 1.0f, 1.0f },
        { -0.25f,  0.25f,  0.25f, 1.0f, 0xff00ff00, 0.0f, 0.0f },
        { 0.25f,   0.25f,  0.25f, 1.0f, 0xff00ff00, 1.0f, 0.0f },
        { 0.25f,  -0.25f,  0.25f, 1.0f, 0xff00ff00, 1.0f, 1.0f },
        { -0.25f, -0.25f,  0.25f, 1.0f, 0xff00ff00, 0.0f, 1.0f },
        { -0.25f,  0.25f,  0.25f, 1.0f, 0xff00ff00, 1.0f, 0.0f },
        { -0.25f,  0.25f, -0.25f, 1.0f, 0xff00ff00, 0.0f, 0.0f },
        { -0.25f, -0.25f, -0.25f, 1.0f, 0xff00ff00, 0.0f, 1.0f },
        { -0.25f, -0.25f,  0.25f, 1.0f, 0xff00ff00, 1.0f, 1.0f },
        { 0.25f,  0.25f,  0.25f, 1.0f, 0xff00ff00, 0.0f, 0.0f },
        { 0.25f,  0.25f, -0.25f, 1.0f, 0xff00ff00, 1.0f, 0.0f },
        { 0.25f, -0.25f, -0.25f, 1.0f, 0xff00ff00, 1.0f, 1.0f },
        { 0.25f, -0.25f,  0.25f, 1.0f, 0xff00ff00, 0.0f, 1.0f }
    };

    Matrix4x GridWorldMatrix = Identity();
    Matrix4x CubeWorldMatrix = Translation(0.0f, .25f, 0.0f, Identity());
    Matrix4x CameraMatrix = XRotationMatrix(-18.0f);
    CameraMatrix = Translation(0.0f, 0.0f, -1.0f, CameraMatrix);

    SV_View = OrthoInv(CameraMatrix);
    SV_Projection = Projection(0.1f, 10.0f, 90.0f, float(Screen_Width) / (Screen_Height));
    

    Vertex transformedCubeVertex[8];
    VertexShader = VS_World;
    do {
        DrawLine(UneditableOriginalLine[0], UneditableOriginalLine[1], 0xffff0000);
        for (int i = 0; i < NumPixels; i++) {
            Pixel_Array[i] = 0x00000000;
        }
      
        clearZBuffer();
        SV_WorldMatrix = GridWorldMatrix;

        PixelShader = PS_White;
        DrawGrid();
        PixelShader = PS_Green;
        CubeWorldMatrix = MultiplyMatrixByMatrix(CubeWorldMatrix, YRotationMatrix(rotationSpeed));

        SV_WorldMatrix = CubeWorldMatrix;
        if (GetAsyncKeyState(0x31) & 0x8000) {
            rotationSpeed = 0.05f;
            cubeChange = 1;
        }

        if (GetAsyncKeyState(0x32) & 0x8000) {
            rotationSpeed = .5f;
            cubeChange = 2;
        }
        if (GetAsyncKeyState(0x33) & 0x8000) {
            rotationSpeed = .5f;
            cubeChange = 3;
        }
        if (GetAsyncKeyState(0x34) & 0x8000) {
            rotationSpeed = .5f;
            cubeChange = 4;
        }
        if (cubeChange == 0 || cubeChange == 1) {
            DrawCube(cubeVertices);
        }
        if (cubeChange == 2 || cubeChange == 3) {
            DrawColoredCube(cubeVertices);
        }
        if (cubeChange == 4) {
            DrawImageCube(cubeVertices);
        }
    } 
    
    while (RS_Update(Pixel_Array, NumPixels));
    RS_Shutdown();
    delete[] Depth_Buffer;
    return 0;
}