// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Create Mesh object and shader object
	mesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	model = new AModel(renderer->getDevice(), "res/teapot.obj");
	cubeMesh = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());
	cubePos[0] = -10.f; cubePos[1] = 5.f; cubePos[2] = 0.f; // (-10.f, 5.f, 0.f);
	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 2, screenHeight / 2, -screenWidth / 1.5f, -screenHeight / 1.5f);
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");

	// initial shaders
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	shadowShader = new ShadowShader(renderer->getDevice(), hwnd);

	// Variables for defining shadow map
	int shadowmapWidth = 5096;
	int shadowmapHeight = 5096;
	int sceneWidth = 250;
	int sceneHeight = 250;

	// Shadow maps for both lights
	shadowMaps[0] = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);
	shadowMaps[1] = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);

	// Configure first directional light (blue)
	lights[0] = new Light();
	lights[0]->setAmbientColour(0.1f, 0.1f, 0.1f, 1.0f);
	lights[0]->setDiffuseColour(0.f, 0.f, 1.0f, 1.0f);
	lightDirection[0][0] = -3.5f; lightDirection[0][1] = -3.5f; lightDirection[0][2] = 3.5f; // (-3.5f, -3.5f, 3.5f)
	lights[0]->setDirection(lightDirection[0][0], lightDirection[0][1], lightDirection[0][2]);
	lights[0]->setPosition(0.f, 10.75f, 0.f); // irrelevant?? directional lights don't have positions.
	lights[0]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);

	// Configure second directional light (red)
	lights[1] = new Light();
	lights[1]->setAmbientColour(0.1f, 0.1f, 0.1f, 1.0f);
	lights[1]->setDiffuseColour(1.0f, 0.0f, 0.0f, 1.0f);
	lightDirection[1][0] = 3.5f; lightDirection[1][1] = -3.5f; lightDirection[1][2] = 3.5f; // (3.5f, -3.5f, 3.5f)
	lights[1]->setDirection(lightDirection[1][0], lightDirection[1][1], lightDirection[1][2]);
	lights[1]->setPosition(0.f, 10.0f, 0.f); // irrelevant?? directional lights don't have positions.
	lights[1]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.

}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	// Update lights
	for (int i = 0; i < TOTAL_LIGHTS; i++) {
		lights[i]->setDirection(lightDirection[i][0], lightDirection[i][1], lightDirection[i][2]);
	}

	return true;
}

bool App1::render()
{

	// Perform depth pass
	depthPass();
	// Render scene
	finalPass();

	return true;
}

void App1::depthPass()
{
	// Set the render target to be the render to texture.
	shadowMaps[0]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	// get the world, view, and projection matrices from the camera and d3d objects.
	lights[0]->generateViewMatrix();
	XMMATRIX lightViewMatrix = lights[0]->getViewMatrix();
	XMMATRIX lightProjectionMatrix = lights[0]->getOrthoMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	// Render floor
	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// Render teapot model
	worldMatrix = XMMatrixTranslation(0.f, 7.f, 5.f);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// Render cube mesh
	worldMatrix = XMMatrixTranslation(cubePos[0], cubePos[1], cubePos[2]); // -12.f, 5.f, 0.f
	cubeMesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), cubeMesh->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();

	// Second shadow map and red light
	shadowMaps[1]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	lights[1]->generateViewMatrix();
	lightViewMatrix = lights[1]->getViewMatrix();
	lightProjectionMatrix = lights[1]->getOrthoMatrix();
	worldMatrix = renderer->getWorldMatrix();

	// Render floor
	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// Render teapot model
	worldMatrix = XMMatrixTranslation(0.f, 7.f, 5.f);
	scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// Set back buffer as render target and reset viewport
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	camera->update();

	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	// Render floor
	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	mesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, 
		textureMgr->getTexture(L"brick"), shadowMaps, lights);
	shadowShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// Render teapot model
	worldMatrix = XMMatrixTranslation(0.f, 7.f, 5.f);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	model->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
		textureMgr->getTexture(L"brick"), shadowMaps, lights);
	shadowShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// Render cube mesh
	worldMatrix = XMMatrixTranslation(cubePos[0], cubePos[1], cubePos[2]);
	cubeMesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
		textureMgr->getTexture(L"brick"), shadowMaps, lights);
	shadowShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// Render ortho mesh with light POV
	renderer->setZBuffer(false);
	worldMatrix = XMMatrixTranslation(1.f, 1.f, 0.f); // final val irrelevant as zbuffer disabled
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();
	orthoMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, shadowMaps[0]->getDepthMapSRV());
	textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);

	// Render GUI and present rendered scene to screen
	gui();
	renderer->endScene();
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	ImGui::SliderFloat3("Cube Position", cubePos, -10, 10, "% .1f");
	ImGui::SliderFloat3("Blue light direction", lightDirection[0], -5, 5, "%.2f");
	ImGui::SliderFloat3("Red light direction", lightDirection[1], -5, 5, "%.2f");

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
