#pragma once
#include"RasterSurface.h"
#include "RasterFunc.h"


int main() {
    RS_Initialize("Osiris Castro 3D Cube", Screen_Width, Screen_Height);
    const Vertex uneditOriginalLine[2] = { {-0.5, +0.5}, {+0.5, -0.5} };

    float rotationSpeed = 0.02f;

    Vertex cubeVertices[16] = {
        {-0.25f, 0.25f, -0.25f, 1.0f, 0xff00ff00}, {0.25f, 0.25f, -0.25f, 1.0f, 0xff00ff00},
        {0.25f, -0.25f, -0.25f, 1.0f, 0xff00ff00}, {-0.25f, -0.25f, -0.25f, 1.0f, 0xff00ff00},
        {-0.25f, 0.25f, 0.25f, 1.0f, 0xff00ff00},  {0.25f, 0.25f, 0.25f, 1.0f, 0xff00ff00},
        {0.25f, -0.25f, 0.25f, 1.0f, 0xff00ff00},  {-0.25f, -0.25f, 0.25f, 1.0f, 0xff00ff00},
    };

    Matrix4x gridWorldMatrix = Identity();
    Matrix4x cubeWorldMatrix = Translation(0.0f, .25f, 0.0f, Identity());
    Matrix4x cameraMatrix = XRotationMatrix(-18.0f);
    cameraMatrix = Translation(0.0f, 0.0f, -1.0f, cameraMatrix);

    SV_View = OrthoInv(cameraMatrix);
    SV_Projection = Projection(0.1f, 10.0f, 90.0f, float(Screen_Width) / (Screen_Height));
    VertexShader = VS_World;

    do {
        DrawLine(uneditOriginalLine[0], uneditOriginalLine[1], 0xffff0000);
        for (int i = 0; i < NumPixels; i++) {
            Pixel_Array[i] = 0x00000000;
        }

        SV_WorldMatrix = gridWorldMatrix;

        PixelShader = PS_White;
        DrawGrid();
        PixelShader = PS_Green;
        cubeWorldMatrix = MultiplyMatrixByMatrix(cubeWorldMatrix, YRotationMatrix(rotationSpeed));
        SV_WorldMatrix = cubeWorldMatrix;
      
        if (cubeChange == 0 || cubeChange == 1) {
            DrawCube(cubeVertices);
        }

    } while (RS_Update(Pixel_Array, NumPixels));

    RS_Shutdown();
    return 0;
}