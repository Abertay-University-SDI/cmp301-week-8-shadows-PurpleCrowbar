// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "TextureShader.h"
#include "ShadowShader.h"
#include "DepthShader.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void depthPass();
	void finalPass();
	void gui();

private:
	TextureShader* textureShader;
	PlaneMesh* mesh;
	OrthoMesh* orthoMesh;

	static const int TOTAL_LIGHTS = 2;
	Light* lights[TOTAL_LIGHTS];
	AModel* model;
	CubeMesh* cubeMesh;
	ShadowShader* shadowShader;
	DepthShader* depthShader;

	ShadowMap* shadowMaps[TOTAL_LIGHTS];

	float cubePos[3];
	float lightDirection[TOTAL_LIGHTS][3];
};

#endif