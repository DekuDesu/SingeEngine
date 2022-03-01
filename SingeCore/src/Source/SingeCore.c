// SingeCore.c
#include <stdio.h>
#include <stdlib.h>

#include "csharp.h"
#include "singine/memory.h"

#include "GL/glew.h"

#include "GLFW/glfw3.h"

#include "graphics/window.h"

#include "graphics/window.h"
#include "graphics/imaging.h"

#include "graphics/shadercompiler.h"
#include "cglm/cam.h"
#include "cglm/mat4.h"

#include "input.h"
#include "modeling/importer.h"
#include "modeling/model.h"
#include "singine/parsing.h"
#include "graphics/renderMesh.h"
#include "graphics/camera.h"
#include "input.h"
#include "cglm/affine.h"
#include "math/quaternions.h"
#include "cglm/quat.h"
#include "singine/time.h"
#include "helpers/quickmask.h"
#include "singine/gameobjectHelpers.h"
#include "graphics/texture.h"
#include "graphics/colors.h"
#include "graphics/font.h"
#include <string.h>
#include "graphics/text.h"
#include "graphics/recttransform.h"
#include "singine/strings.h"
#include "graphics/graphicsDevice.h"
#include "tests.h"
#include "graphics/scene.h"

#include "graphics/renderbuffers.h"
#include "graphics/framebuffers.h"
#include "singine/defaults.h"

// scripts (not intrinsically part of the engine)
#include "scripts/fpsCamera.h"

Window window;

void DebugCameraPosition(Camera camera);
void ToggleNormalShaders(GameObject* gameobjects, size_t size, bool enabled);

int main()
{
	Tests.RunAll();

	Windows.StartRuntime();

	Windows.SetHint(WindowHints.MSAASamples, 4);

	// use opengl 3.3
	Windows.SetHint(ContextHints.VersionMajor, 4);
	Windows.SetHint(ContextHints.VersionMinor, 4);

	Windows.SetHint(OpenGLHints.ForwardCompatibility, true); // To make MacOS happy; should not be needed
	Windows.SetHint(OpenGLHints.Profile, GLFW_OPENGL_CORE_PROFILE);

	// allow it to be windowed and resize-able
	Windows.SetHint(WindowHints.Resizable, true);
	Windows.SetHint(WindowHints.Decorated, true);

	window = Windows.Create(DEFAULT_VIEWPORT_RESOLUTION_X, DEFAULT_VIEWPORT_RESOLUTION_Y, "Singine");

	SetInputWindow(window);

	//sWindow.SetMode(window, WindowModes.FullScreen);

	Windows.SetClearColor(0.4f, 0.4f, 0.0f, 0.0f);

	// bind graphics card with GDI
	Windows.SetCurrent(window);

	// initiliaze GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		glfwTerminate();
		throw(IndexOutOfRangeException);
	}

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	glEnable(GL_STENCIL_TEST);

	//glEnable(GL_FRAMEBUFFER_SRGB);


	SetCursorMode(CursorModes.Disabled);

	Image icon = Images.LoadImage("assets/textures/icon.png");

	Windows.SetIcon(window, icon);

	Windows.SetMode(window, WindowModes.Windowed);

	glfwSwapInterval(0);

	Images.Dispose(icon);

	GameObject skybox = GameObjects.Load("assets/prefabs/skybox.gameobject");

	// load the default material so we can render gameobjects that have no set material
	Material defaultMaterial = Materials.Load("assets/materials/default.material");
	GameObjects.SetDefaultMaterial(defaultMaterial);

	Material textMaterial = Materials.Load("assets/materials/textMaterial.material");

	Material textureMaterial = Materials.Load("assets/materials/textMaterial.material");

	Material outlineMaterial = Materials.Load("assets/materials/outline.material");

	Camera camera = Cameras.Create();

	// bind a vertex array for OpenGL this is required to render objects
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	GameObject room = GameObjects.Load("assets/prefabs/room.gameobject");

	GameObject cube = GameObjects.Load("assets/prefabs/cube.gameobject");

	Font font = Fonts.Import("assets/fonts/ComicMono.obj", FileFormats.Obj);
	Fonts.SetMaterial(font, textMaterial);

	GameObject ball = GameObjects.Load("assets/prefabs/ball.gameobject");

	GameObject otherBall = GameObjects.Duplicate(ball);
	GameObject car = GameObjects.Load("assets/prefabs/car.gameobject");

	GameObject city = GameObjects.Load("assets/prefabs/house.gameobject");


	float speed = 10.0f;

	float rotateAmount = 0.0f;

	vec3 position;

	vec3 positionModifier;

	Transforms.SetParent(otherBall->Transform, ball->Transform);
	Transforms.SetScales(otherBall->Transform, 0.5, 0.5, 0.5);
	Transforms.SetPositions(otherBall->Transform, 0, 0, 3);
	Transforms.SetPositions(car->Transform, -7, 0, -7);
	Transforms.SetPositions(ball->Transform, 5, 1, 5);

	Mesh squareMesh = Meshes.Create();

	float verts[18] = {
		-1, 1, 0,
		-1, -1, 0,
		1, -1, 0,
		1, 1, 0,
		-1, 1, 0,
		1, -1, 0
	};

	float textures[12] = {
		0,0,
		0,1,
		1,1,
		1,0,
		0,0,
		1,1,
	};

	squareMesh->VertexCount = 3 * 2 * 3;
	squareMesh->Vertices = verts;

	squareMesh->TextureCount = 12;
	squareMesh->TextureVertices = textures;

	GameObject square = CreateGameObjectFromMesh(squareMesh);

	Transforms.ScaleAll(square->Transform, 0.5f);

	SafeFree(squareMesh);

	GameObject subSquare = GameObjects.Duplicate(square);

	GameObjects.SetMaterial(subSquare, textureMaterial);

	Materials.SetColor(subSquare->Material, Colors.White);

	Transforms.SetParent(subSquare->Transform, square->Transform);

	RectTransforms.SetTransform(subSquare->Transform, Anchors.LowerLeft, Pivots.UpperLeft, 0, 0, 0.5f, 0.5f);


	// orient the camera so we see some geometry without moving the camera
	//Transforms.SetPositions(camera->Transform, 2.11f, 1.69f, 8.39f);
	Transforms.SetPositions(camera->Transform, 0, 0, 0);

	Text text = Texts.CreateEmpty(font, 512);

	float fontSize = 0.06125f;

	RectTransforms.SetTransform(text->GameObject->Transform, Anchors.UpperLeft, Pivots.UpperLeft, 0, 0, fontSize, fontSize);

	float amount = 0;

	Materials.SetColor(car->Material, Colors.Green);

	GameObject otherCube = GameObjects.Duplicate(cube);

	GameObjects.SetMaterial(otherCube, defaultMaterial);
	Transforms.ScaleAll(otherCube->Transform, 1.2f);
	Materials.SetColors(otherCube->Material, 1, 1, 1, 1);


	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);

	Scene scene = Scenes.Create();

	scene->MainCamera = camera;

	// create a test light for manual testing
	Light light = Lights.Create(LightTypes.Directional);
	Light otherLight = Lights.Create(LightTypes.Point);

	Light spotLight = Lights.Create(LightTypes.Spot);

	SetVector4Macro(spotLight->Ambient, 0, 0, 0, 0);

	// light body
	Transforms.SetPositions(light->Transform, 0, 10, 10);
	Transforms.SetPositions(otherLight->Transform, 0, 3, 0);

	Transforms.SetPositions(camera->Transform, -3, 3, 3);

	GameObject lightMarker = GameObjects.Duplicate(cube);

	Transforms.SetPositions(lightMarker->Transform, 0, 0, 0);

	GameObjects.SetMaterial(lightMarker, outlineMaterial);

	Transforms.ScaleAll(lightMarker->Transform, 0.25f);

	GameObject otherLightMarker = GameObjects.Duplicate(lightMarker);

	Transforms.SetParent(otherLightMarker->Transform, otherLight->Transform);
	Transforms.SetParent(lightMarker->Transform, light->Transform);

	light->Enabled = true;
	light->Radius = 50.0f;
	light->Range = 100.0f;
	light->Intensity = 0.5f;

	otherLight->Enabled = true;
	otherLight->Radius = 10.0f;
	otherLight->Range = 10.0f;
	otherLight->Intensity = 1.0f;

	// point directional light at center
	Transforms.LookAt(light->Transform, Vector3.Zero);

	// add the light to the scene
	//Scenes.AddLight(scene, light);
	Scenes.AddLight(scene, otherLight);
	//Scenes.AddLight(scene, spotLight);

	spotLight->EdgeSoftness = 0.5f;
	spotLight->Enabled = false;

	GameObject plane = GameObjects.Load("assets/prefabs/plane.gameobject");

	Transforms.RotateOnAxis(plane->Transform, 0.5f * (float)GLM_PI, Vector3.Forward);

	Materials.SetAreaTexture(cube->Material, skybox->Material->MainTexture);
	Materials.SetReflectionTexture(cube->Material, cube->Material->SpecularTexture);

	Materials.SetAreaTexture(ball->Material, skybox->Material->MainTexture);
	Materials.SetAreaTexture(otherBall->Material, skybox->Material->MainTexture);

	Materials.SetAreaTexture(car->Material, skybox->Material->MainTexture);
	Materials.SetAreaTexture(plane->Material, skybox->Material->MainTexture);

	GameObject sphere = GameObjects.Load("assets/prefabs/sphere.gameobject");

	Material depthMaterial = Materials.Load("assets/materials/depthMap.material");

	GameObjects.SetMaterial(square, depthMaterial);

	Materials.SetMainTexture(square->Material, light->FrameBuffer->Texture);

	Materials.Dispose(depthMaterial);

	GameObject gameobjects[] = {
		lightMarker,
		otherLightMarker,
		cube,
		sphere,
		car,
		ball,
		otherBall,
		room,
		city,
		plane
	};

	size_t gameobjectCount = sizeof(gameobjects) / sizeof(GameObject);

	Camera shadowCamera = Cameras.Create();

	Material shadowMapMaterial = Materials.Load("assets/materials/shadow.material");

	Material shadowCubeMapMaterial = Materials.Load("assets/materials/shadowCubemap.material");

	RectTransforms.SetTransform(square->Transform, Anchors.LowerRight, Pivots.LowerRight, 0, 0, 0.25, 0.25);

	bool showNormals = false;

	ToggleNormalShaders(gameobjects, gameobjectCount, showNormals);

	// we update time once before the start of the program becuase if startup takes a long time delta time may be large for the first call
	UpdateTime();
	do {
		UpdateTime();

		Vectors3CopyTo(camera->Transform->Position, position);

		float modifier = speed * (float)DeltaTime();

		rotateAmount += modifier;

		vec3 ballPosition = { 5,2 + (float)sin(rotateAmount), 5 };

		Transforms.SetPosition(ball->Transform, ballPosition);

		Transforms.SetRotationOnAxis(ball->Transform, rotateAmount / (float)GLM_PI, Vector3.Up);

		Transforms.SetRotationOnAxis(otherBall->Transform, -rotateAmount / (float)GLM_PI, Vector3.Up);

		// drive car
		vec3 carDirection;
		Transforms.GetDirection(car->Transform, Directions.Forward, carDirection);

		ScaleVector3(carDirection, modifier);

		Transforms.AddPosition(car->Transform, carDirection);

		Transforms.RotateOnAxis(car->Transform, ((float)GLM_PI / 8.0f) * modifier, Vector3.Up);

		if (GetKey(KeyCodes.A))
		{
			Transforms.GetDirection(camera->Transform, Directions.Left, positionModifier);
			ScaleVector3(positionModifier, modifier);
			AddVectors3(position, positionModifier);
		}
		if (GetKey(KeyCodes.D))
		{
			Transforms.GetDirection(camera->Transform, Directions.Right, positionModifier);
			ScaleVector3(positionModifier, modifier);
			AddVectors3(position, positionModifier);
		}
		if (GetKey(KeyCodes.W))
		{
			Transforms.GetDirection(camera->Transform, Directions.Forward, positionModifier);
			ScaleVector3(positionModifier, modifier);
			SubtractVectors3(position, positionModifier);
		}
		if (GetKey(KeyCodes.S))
		{
			Transforms.GetDirection(camera->Transform, Directions.Back, positionModifier);
			ScaleVector3(positionModifier, modifier);
			SubtractVectors3(position, positionModifier);
		}
		if (GetKey(KeyCodes.Space))
		{
			Transforms.GetDirection(camera->Transform, Directions.Up, positionModifier);
			ScaleVector3(positionModifier, modifier);
			AddVectors3(position, positionModifier);
		}
		if (GetKey(KeyCodes.LeftShift))
		{
			Transforms.GetDirection(camera->Transform, Directions.Down, positionModifier);
			ScaleVector3(positionModifier, modifier);
			AddVectors3(position, positionModifier);
		}
		if (GetAxis(Axes.Horizontal) < 0)
		{
			spotLight->Range += (float)DeltaTime();
		}
		else if (GetAxis(Axes.Horizontal) > 0)
		{
			spotLight->Range -= (float)DeltaTime();
		}
		if (GetAxis(Axes.Vertical) < 0)
		{
			++amount;
			otherLight->Intensity = amount / 1000;
		}
		else if (GetAxis(Axes.Vertical) > 0)
		{
			--amount;
			otherLight->Intensity = amount / 1000;
		}
		if (GetKey(KeyCodes.L))
		{
			light->Enabled = !light->Enabled;
			otherLight->Enabled = !otherLight->Enabled;
		}

		// toggle debug normals
		if (GetKey(KeyCodes.N))
		{
			ToggleNormalShaders(gameobjects, gameobjectCount, !showNormals);

			showNormals = !showNormals;
		}

		Transforms.SetPositions(otherCube->Transform, (float)(3 * cos(Time())), 3, 3);

		Transforms.SetRotationOnAxis(cube->Transform, (float)(3 * cos(Time())), Vector3.Up);

		int count = sprintf_s(text->Text, text->Length, 
			"%2.4lf ms (high:%2.4lf ms avg:%2.4lf)\n%4.1lf FPS\n%s: %f\nIntensity: %f", 
			FrameTime(), 
			HighestFrameTime(), 
			AverageFrameTime(),
			1.0 / FrameTime(), 
			"amount", 
			amount / 1000, 
			otherLight->Intensity);

		Texts.SetText(text, text->Text, count);

		// update the FPS camera
		FPSCamera.Update(camera);
		Transforms.SetPosition(camera->Transform, position);

		GameObjects.GenerateShadowMaps(gameobjects, sizeof(gameobjects) / sizeof(GameObject), scene, shadowMapMaterial, shadowCubeMapMaterial, shadowCamera);

		// draw scene
		FrameBuffers.ClearAndUse(FrameBuffers.Default);

		scene->MainCamera = camera;

		GameObjects.DrawMany(gameobjects, sizeof(gameobjects) / sizeof(GameObject), scene, null);

		GameObjects.Draw(skybox, scene);

		GameObjects.Draw(square, scene);
		Texts.Draw(text, scene);

		// swap the back buffer with the front one
		glfwSwapBuffers(window->Handle);

		//DebugCameraPosition(camera);

		PollInput();

	} while (GetKey(KeyCodes.Escape) != true && Windows.ShouldClose(window) != true);

	Lights.Dispose(light);
	Lights.Dispose(otherLight);
	Lights.Dispose(spotLight);

	Texts.Dispose(text);

	Fonts.Dispose(font);

	GameObjects.Destroy(ball);
	GameObjects.Destroy(otherBall);
	GameObjects.Destroy(car);
	GameObjects.Destroy(room);
	GameObjects.Destroy(cube);
	GameObjects.Destroy(square);
	GameObjects.Destroy(otherCube);
	GameObjects.Destroy(subSquare);
	GameObjects.Destroy(lightMarker);
	GameObjects.Destroy(plane);
	GameObjects.Destroy(otherLightMarker);
	GameObjects.Destroy(skybox);
	GameObjects.Destroy(sphere);

	Materials.Dispose(textMaterial);
	Materials.Dispose(defaultMaterial);
	Materials.Dispose(textureMaterial);
	Materials.Dispose(outlineMaterial);
	Materials.Dispose(shadowMapMaterial);
	Materials.Dispose(shadowCubeMapMaterial);

	Cameras.Dispose(camera);

	Scenes.Dispose(scene);

	Windows.Dispose(window);

	GameObjects.Destroy(city);

	Windows.StopRuntime();



	// ensure leak free
	PrintAlloc(stdout);
	PrintFree(stdout);

	if (AllocCount() > FreeCount())
	{
		//throw(MemoryLeakException);
	}

	if (GraphicsDevice.TryVerifyCleanup() is false)
	{
		throw(MemoryLeakException);
	}
}

void DebugCameraPosition(Camera camera)
{
	fprintf(stdout, "Position: ");
	PrintVector3(camera->Transform->Position, stdout);
	fprintf(stdout, " Rotation: [ x: %0.2fpi, y: %0.2fpi ] Forward: ", FPSCamera.State.HorizontalAngle / GLM_PI, FPSCamera.State.VerticalAngle / GLM_PI);
	vec3 forwardVector;
	Transforms.GetDirection(camera->Transform, Directions.Forward, forwardVector);
	PrintVector3(forwardVector, stdout);
	fprintf(stdout, NEWLINE);
}

void ToggleNormalShaders(GameObject* gameobjects, size_t size, bool enabled)
{
	for (size_t i = 0; i < size; i++)
	{
		// get the material
		Material material = gameobjects[i]->Material;

		// iterate the shaders and disable any normal shaders by name
		for (size_t ithShader = 0; ithShader < material->Count; ithShader++)
		{
			static const char* path = "assets/shaders/debug_normal.shader";
			static const size_t length = sizeof("assets/shaders/debug_normal.shader") - 1;

			Shader shader = material->Shaders[ithShader];

			if (Strings.Equals(path, length, shader->Name, strlen(shader->Name)))
			{
				shader->Enabled = enabled;
			}
		}
	}
}