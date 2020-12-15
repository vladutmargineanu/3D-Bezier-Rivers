#include "Tema1.h"

#include <vector>
#include <iostream>

#include <stb/stb_image.h>
#include <Core/Engine.h>

using namespace std;

// Order of function calling can be seen in "Source/Core/World.cpp::LoopUpdate()"
// https://github.com/UPB-Graphics/SPG-Framework/blob/master/Source/Core/World.cpp

Tema1::Tema1()
{
}

Tema1::~Tema1()
{
}

void Tema1::DefaultParameters()
{
	// River control
	controlPointsCount = 4;
	controlPointsCountOne = 4;
	smoothness = 0.5f;
	animationSpeed = 0.1f;
	tilingFactor = 5.0f;
	maxAnimationSpeed = 4.0f;

	// Curve generation
	instanceCount = 1;
	generatedPoints = 30;
	riverWidth = 4.5f;

	// Control points
	controlPoints.push_back(glm::vec3((rand() % (max - min + 1) + min) * (-1), 0, -16));
	controlPoints.push_back(glm::vec3((rand() % (max - min + 1) + min) * (-1), 0, -3));
	controlPoints.push_back(glm::vec3((rand() % (max - min + 1) + min) * (-1), 0, -7));
	firstPoint = rand() % (max_left - min_left + 1) + min_left;
	controlPoints.push_back(glm::vec3(firstPoint, 0, 0));

	// Control points first river
	controlPointsOne.push_back(glm::vec3(firstPoint, 0, 0));
	controlPointsOne.push_back(glm::vec3(rand() % ((max_right - min_right + 1) + min_right) * (-1), 0, 4));
	controlPointsOne.push_back(glm::vec3((rand() % (max_right - min_right + 1) + min_right) * (-1), 0, 7));
	controlPointsOne.push_back(glm::vec3((rand() % (max_right - min_right + 1) + min_right) * (-1), 0, 16));

	// Control points two river
	controlPointsTwo.push_back(glm::vec3(firstPoint, 0, 0)); 
	controlPointsTwo.push_back(glm::vec3(rand() %(max_middle - min_middle + 1) + min_middle, 0, 3));
	controlPointsTwo.push_back(glm::vec3(rand() % (max_middle - min_middle + 1) + min_middle, 0, 9));
	controlPointsTwo.push_back(glm::vec3(rand() % (max_middle - min_middle + 1) + min_middle, 0, 16));

	// Control point third river
	controlPointsThree.push_back(glm::vec3(firstPoint, 0, 0));
	controlPointsThree.push_back(glm::vec3(rand() %(max_left - min_left + 1) + min_left, 0, 2));
	controlPointsThree.push_back(glm::vec3(rand() % (max_left - min_left + 1) + min_left, 0, 6));
	controlPointsThree.push_back(glm::vec3(rand() % (max_left - min_left + 1) + min_left, 0, 16));

	// Calculate points for mountains
	CommonPoint = glm::vec3(firstPoint, 0, 0);

	leftFirst = glm::vec3((controlPointsOne[1].x + controlPointsThree[1].x) / 2,
		(controlPointsOne[1].y + controlPointsThree[1].y) / 2,
		(controlPointsOne[1].z + controlPointsThree[1].z) / 2);

	leftSecond = glm::vec3((controlPointsOne[2].x + controlPointsThree[2].x) / 2,
		(controlPointsOne[2].y + controlPointsThree[2].y) / 2,
		(controlPointsOne[2].z + controlPointsThree[2].z) / 2);

	leftThird = glm::vec3((controlPointsOne[3].x + controlPointsThree[3].x) / 2,
		(controlPointsOne[3].y + controlPointsThree[3].y) / 2,
		(controlPointsOne[3].z + controlPointsThree[3].z) / 2);

	rightFirst = glm::vec3((controlPointsTwo[1].x + controlPointsThree[1].x) / 2,
		(controlPointsTwo[1].y + controlPointsThree[1].y) / 2,
		(controlPointsTwo[1].z + controlPointsThree[1].z) / 2);

	rightSecond = glm::vec3((controlPointsTwo[2].x + controlPointsThree[2].x) / 2,
		(controlPointsTwo[2].y + controlPointsThree[2].y) / 2,
		(controlPointsTwo[2].z + controlPointsThree[2].z) / 2);

	rightThird = glm::vec3((controlPointsTwo[3].x + controlPointsThree[3].x) / 2,
		(controlPointsTwo[3].y + controlPointsThree[3].y) / 2,
		(controlPointsTwo[3].z + controlPointsThree[3].z) / 2);

	// initialize tx, ty and tz (the translation steps)
	translateX = 0;
	translateY = 0.0f;
	translateZ = 0;

	// initialize angularSteps
	angularStepOX = 0;
	angularStepOY = 0;
	angularStepOZ = 0;
	// Number of particles
	nrParticles = 50'000;
}

void Tema1::Init()
{
	auto camera = GetSceneCamera();
	camera->SetPositionAndRotation(CommonPoint + glm::vec3(0,5.0f, 0), glm::quat(glm::vec3(-30 * TO_RADIANS, 0, 0)));
	camera->Update();

	// ToggleGroundPlane();

	DefaultParameters();

	// Boat ----------------------------------------------------------------------
	{
		Mesh* mesh = new Mesh("boat");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Models", "kapal.fbx");
		mesh->UseMaterials(false);
		meshes[mesh->GetMeshID()] = std::shared_ptr<Mesh>(mesh);
	}

	// Cube ----------------------------------------------------------------------
	std::string texturePath = RESOURCE_PATH::TEXTURES + "Cube/";
	{
		Mesh* mesh = new Mesh("cube");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "box.obj");
		mesh->UseMaterials(false);
		meshes[mesh->GetMeshID()] = std::shared_ptr<Mesh>(mesh);
	}

	cubeMapTextureID = UploadCubeMapTexture(
		texturePath + "posx.png",
		texturePath + "posy.png",
		texturePath + "posz.png",
		texturePath + "negx.png",
		texturePath + "negy.png",
		texturePath + "negz.png"
	);

	// Mountain mesh left -----------------------------------------------------------------
	glm::vec3 offsetX = glm::vec3(riverWidth / 2, 0, 0);
	glm::vec3 offsetZ = glm::vec3(0, 0, riverWidth / 2);
	glm::vec3 offsetY = glm::vec3(0, 2.0f * riverWidth, 0);

	{
		std::vector<VertexFormat> vertices =
		{
			VertexFormat(CommonPoint + offsetZ + offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(0.0f, 0.0f)),
			VertexFormat(controlPointsThree[1] + offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 0.0f)),
			VertexFormat(rightFirst + offsetY, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 1.0f)),
			VertexFormat(controlPointsTwo[1] - offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(0.0f, 1.0f)),
			VertexFormat(controlPointsTwo[2] - offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 1.0f)),

			VertexFormat(rightSecond + offsetY, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(0.0f, 0.0f)),
			VertexFormat(controlPointsThree[2] + offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 0.0f)),
			VertexFormat(controlPointsThree[3] + offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 1.0f)),
			VertexFormat(rightThird + offsetY, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(0.0f, 1.0f)),
			VertexFormat(controlPointsTwo[3] - offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 1.0f))
		};

		std::vector<unsigned short> indices =
		{
			0, 2, 1,
			0, 3, 2,
			3, 5, 2,
			3, 4, 5,
			2, 5, 6,
			2, 6, 1,
			5, 7, 6,
			5, 8, 7,
			5, 4, 8,
			9, 8, 4
		};

		meshes["mountain_left"] = std::shared_ptr<Mesh>(new Mesh("mountain_left"));
		meshes["mountain_left"]->InitFromData(vertices, indices);
	}

	// Mountain mesh right -----------------------------------------------------------------
	{
		std::vector<VertexFormat> vertices =
		{
			VertexFormat(CommonPoint + offsetZ - offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(0.0f, 1.0f)),
			VertexFormat(controlPointsOne[1] + offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 1.0f)),
			VertexFormat(leftFirst + offsetY, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 0.0f)),
			VertexFormat(controlPointsThree[1] - offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(0.0f, 0.0f)),
			VertexFormat(controlPointsThree[2] - offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 0.0f)),

			VertexFormat(leftSecond + offsetY, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(0.0f, 1.0f)),
			VertexFormat(controlPointsOne[2] + offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 1.0f)),
			VertexFormat(controlPointsOne[3] + offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 0.0f)),
			VertexFormat(leftThird + offsetY, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(0.0f, 0.0f)),
			VertexFormat(controlPointsThree[3] - offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(0.0f, 1.0f))
		};

		std::vector<unsigned short> indices =
		{
			0, 2, 1,
			0, 3, 2,
			3, 5, 2,
			3, 4, 5,
			2, 5, 6,
			2, 6, 1,
			5, 7, 6,
			5, 8, 7,
			5, 4, 8,
			9, 8, 4
		};

		meshes["mountain_right"] = std::shared_ptr<Mesh>(new Mesh("mountain_right"));
		meshes["mountain_right"]->InitFromData(vertices, indices);
	}

	// Grand Mountain mesh left -----------------------------------------------------------------
	glm::vec3 downLeft = glm::vec3(14, 0, -14);
	glm::vec3 downRight = glm::vec3(-14, 0, -14);
	glm::vec3 upLeft = glm::vec3(14, 0, 14);
	glm::vec3 upRight = glm::vec3(-14, 0, 14);

	{
		std::vector<VertexFormat> vertices =
		{
			VertexFormat(downLeft + offsetY, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(0.0f, 0.0f)),
			VertexFormat(controlPoints[0] + offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 0.0f)),
			VertexFormat(controlPoints[1] + offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 1.0f)),
			VertexFormat(controlPoints[2] + offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 0.0f)),
			VertexFormat(controlPoints[3] + offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 1.0f)),
			VertexFormat(controlPointsTwo[1] + offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 0.0f)),
			VertexFormat(controlPointsTwo[2] + offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 1.0f)),
			VertexFormat(controlPointsTwo[3] + offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 0.0f)),
			VertexFormat(upLeft + offsetY, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(0.0f, 1.0f)),
		};

		std::vector<unsigned short> indices =
		{
			0, 1, 2,
			0, 2, 3,
			0, 3, 4,
			0, 4, 5,
			0, 5, 6,
			0, 6, 7,
			0, 7, 8
		};

		meshes["grand_mountain_left"] = std::shared_ptr<Mesh>(new Mesh("grand_mountain_left"));
		meshes["grand_mountain_left"]->InitFromData(vertices, indices);
	}

	// Grand Mountain mesh right -----------------------------------------------------------------
	{
		std::vector<VertexFormat> vertices =
		{
			VertexFormat(downRight + offsetY, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(0.0f, 0.0f)),
			VertexFormat(controlPoints[0] - offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 0.0f)),
			VertexFormat(controlPoints[1] - offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 1.0f)),
			VertexFormat(controlPoints[2] - offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 0.0f)),
			VertexFormat(controlPoints[3] - offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 1.0f)),
			VertexFormat(controlPointsOne[1] - offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 0.0f)),
			VertexFormat(controlPointsOne[2] - offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 1.0f)),
			VertexFormat(controlPointsOne[3] - offsetX, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 0.0f)),
			VertexFormat(upRight + offsetY, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(0.0f, 1.0f)),
		};

		std::vector<unsigned short> indices =
		{
			0, 1, 2,
			0, 2, 3,
			0, 3, 4,
			0, 4, 5,
			0, 5, 6,
			0, 6, 7,
			0, 7, 8
		};

		meshes["grand_mountain_right"] = std::shared_ptr<Mesh>(new Mesh("grand_mountain_right"));
		meshes["grand_mountain_right"]->InitFromData(vertices, indices);
	}

	// Quad mesh -----------------------------------------------------------------
	{
		std::vector<VertexFormat> vertices =
		{
			VertexFormat(glm::vec3(-0.5, 0.5, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(0.0f, 1.0f)),
			VertexFormat(glm::vec3(0.5, 0.5, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 1.0f)),
			VertexFormat(glm::vec3(0.5, -0.5, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 0.0f)),
			VertexFormat(glm::vec3(-0.5, -0.5, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(0.0f, 0.0f))
		};

		std::vector<unsigned short> indices =
		{
			0, 1, 2,
			0, 2, 3
		};

		meshes["quad"] = std::shared_ptr<Mesh>(new Mesh("quad"));
		meshes["quad"]->InitFromData(vertices, indices);
	}

	// River mesh
	{
		std::vector<VertexFormat> vertices =
		{
			VertexFormat(controlPoints[0], glm::vec3(0, 1, 0)),
			VertexFormat(controlPoints[controlPointsCount - 1], glm::vec3(0, 1, 0))
		};

		std::vector<unsigned short> indices =
		{
			0, 1
		};

		meshes["river"] = std::shared_ptr<Mesh>(new Mesh("generated initial surface points"));
		meshes["river"]->InitFromData(vertices, indices);
		meshes["river"]->SetDrawMode(GL_LINES);
	}

	// River mesh first
	{
		std::vector<VertexFormat> vertices =
		{
			VertexFormat(controlPointsOne[0], glm::vec3(0, 1, 0)),
			VertexFormat(controlPointsOne[controlPointsCountOne - 1], glm::vec3(0, 1, 0))
		};

		std::vector<unsigned short> indices =
		{
			0, 1
		};

		meshes["river_first"] = std::shared_ptr<Mesh>(new Mesh("generated initial surface points"));
		meshes["river_first"]->InitFromData(vertices, indices);
		meshes["river_first"]->SetDrawMode(GL_LINES);
	}
	// River mesh second
	{
		std::vector<VertexFormat> vertices =
		{
			VertexFormat(controlPointsTwo[0], glm::vec3(0, 1, 0)),
			VertexFormat(controlPointsTwo[controlPointsCountOne - 1], glm::vec3(0, 1, 0))
		};

		std::vector<unsigned short> indices =
		{
			0, 1
		};

		meshes["river_second"] = std::shared_ptr<Mesh>(new Mesh("generated initial surface points"));
		meshes["river_second"]->InitFromData(vertices, indices);
		meshes["river_second"]->SetDrawMode(GL_LINES);
	}
	// River mesh third
	{
		std::vector<VertexFormat> vertices =
		{
			VertexFormat(controlPointsThree[0], glm::vec3(0, 1, 0)),
			VertexFormat(controlPointsThree[controlPointsCountOne - 1], glm::vec3(0, 1, 0))
		};

		std::vector<unsigned short> indices =
		{
			0, 1
		};

		meshes["river_third"] = std::shared_ptr<Mesh>(new Mesh("generated initial surface points"));
		meshes["river_third"]->InitFromData(vertices, indices);
		meshes["river_third"]->SetDrawMode(GL_LINES);
	}

	// Create a shader program for rendering to texture
	{
		Shader* shader = new Shader("CubeMap");
		shader->AddShader(RESOURCE_PATH::SHADERS + "Tema1/CubeMap.VS.glsl", GL_VERTEX_SHADER);
		shader->AddShader(RESOURCE_PATH::SHADERS + "Tema1/CubeMap.FS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = std::shared_ptr<Shader>(shader);
	}

	// Cube Shader ---------------------------------------------------------------
	{
		Shader* shader = new Shader("ShaderNormal");
		shader->AddShader(RESOURCE_PATH::SHADERS + "Tema1/Normal.VS.glsl", GL_VERTEX_SHADER);
		shader->AddShader(RESOURCE_PATH::SHADERS + "Tema1/Normal.FS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = std::shared_ptr<Shader>(shader);
	}

	// Particle Shader -----------------------------------------------------------
	{
		Shader* shader = new Shader("Particle");
		shader->AddShader(RESOURCE_PATH::SHADERS + "Tema1/Particle.VS.glsl", GL_VERTEX_SHADER);
		shader->AddShader(RESOURCE_PATH::SHADERS + "Tema1/Particle.GS.glsl", GL_GEOMETRY_SHADER);
		shader->AddShader(RESOURCE_PATH::SHADERS + "Tema1/Simple.FS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = std::shared_ptr<Shader>(shader);
	}

	// Default Shader ------------------------------------------------------------
	{
		Shader* shader = new Shader("Simple");
		shader->AddShader(RESOURCE_PATH::SHADERS + "Tema1/Simple.VS.glsl", GL_VERTEX_SHADER);
		shader->AddShader(RESOURCE_PATH::SHADERS + "Tema1/Simple.FS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = std::shared_ptr<Shader>(shader);
	}

	// Curve Shader --------------------------------------------------------------
	{
		Shader* shader = new Shader("BezierCurve");
		shader->AddShader(RESOURCE_PATH::SHADERS + "Tema1/Pass.VS.glsl", GL_VERTEX_SHADER);
		shader->AddShader(RESOURCE_PATH::SHADERS + "Tema1/Bezier.GS.glsl", GL_GEOMETRY_SHADER);
		shader->AddShader(RESOURCE_PATH::SHADERS + "Tema1/Simple.FS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = std::shared_ptr<Shader>(shader);
	}

	// Water Texture -------------------------------------------------------------
	TextureManager::LoadTexture(RESOURCE_PATH::TEXTURES + "Tema1", "water.png", "water");

	// Particle Texture ----------------------------------------------------------
	TextureManager::LoadTexture(RESOURCE_PATH::TEXTURES, "particle.png", "water_splash");

	// Control Point Texture -----------------------------------------------------
	TextureManager::LoadTexture(RESOURCE_PATH::TEXTURES + "Tema1", "test.png", "button");

	// Mountain Texture ----------------------------------------------------------
	TextureManager::LoadTexture(RESOURCE_PATH::TEXTURES + "Tema1", "mountain.png", "mountain");

	// Boat Texture ----------------------------------------------------------
	TextureManager::LoadTexture(RESOURCE_PATH::TEXTURES + "Tema1", "RockCliff.png", "boat");

	// Create particles
	particleEffect = new ParticleEffect<Particle>();
	particleEffect->Generate(nrParticles, true);

	auto particleSSBO = particleEffect->GetParticleBuffer();
	Particle* data = const_cast<Particle*>(particleSSBO->GetBuffer());

	int cubeSize = 20;
	int hSize = cubeSize / 2;

	for (unsigned int i = 0; i < nrParticles; i++)
	{
		glm::vec4 pos(1);
		pos.x = (rand() % cubeSize - hSize) / 10.0f;
		pos.y = (rand() % cubeSize - hSize) / 10.0f;
		pos.z = (rand() % cubeSize - hSize) / 10.0f;

		glm::vec4 speed(0);
		speed.x = (rand() % 20 - 10) / 10.0f;
		speed.z = (rand() % 20 - 10) / 10.0f;
		speed.y = rand() % 2 + 2.0f;

		data[i].SetInitial(pos, speed);
	}
	particleSSBO->SetBufferData(data);

}

void Tema1::FrameStart()
{
	// clears the color buffer (using the previously set color) and depth buffer
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::ivec2 resolution = window->GetResolution();
	// sets the screen area where to draw
	glViewport(0, 0, resolution.x, resolution.y);
}


void Tema1::Update(float deltaTimeSeconds)
{
	ClearScreen();

	auto camera = GetSceneCamera();
	// Particles
	glLineWidth(3);

	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);

	{
		auto shader = shaders["Particle"];
		if (shader->GetProgramID())
		{
			shader->Use();
			TextureManager::GetTexture("particle2.png")->BindToTextureUnit(GL_TEXTURE0);
			particleEffect->Render(GetSceneCamera(), shader.get());
		}
	}
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	//pe tot ecranul
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::vec3 planeOffset = glm::vec3(0.0f, 0.0f, 0.0f);

	// draw the cubemap
	RenderCube(meshes["cube"], shaders["ShaderNormal"], cubeMapTextureID);
	
	// Render control points 
	Texture2D* texture = TextureManager::GetTexture("button");
	for (auto& point : controlPoints)
	{
		RenderMeshObject(meshes["quad"], shaders["Simple"], texture, point + planeOffset, glm::vec3(0.2), glm::vec3(1, 0, 0));
	}

	for (auto& point : controlPointsOne)
	{
		RenderMeshObject(meshes["quad"], shaders["Simple"], texture, point + planeOffset, glm::vec3(0.2), glm::vec3(1, 0, 0));
	}

	for (auto& point : controlPointsTwo)
	{
		RenderMeshObject(meshes["quad"], shaders["Simple"], texture, point + planeOffset, glm::vec3(0.2), glm::vec3(1, 0, 0));
	}

	for (auto& point : controlPointsThree)
	{
		RenderMeshObject(meshes["quad"], shaders["Simple"], texture, point + planeOffset, glm::vec3(0.2), glm::vec3(1, 0, 0));
	}

	// draw the boat
	RenderBoat(meshes["boat"], shaders["CubeMap"]);

	// Render river curve
	RenderRiver(TextureManager::GetTexture("water"));
	RenderRiverFirst(TextureManager::GetTexture("water"));
	RenderRiverSecond(TextureManager::GetTexture("water"));
	RenderRiverThird(TextureManager::GetTexture("water"));

	// Render mountains
	texture = TextureManager::GetTexture("mountain");
	RenderMountain(meshes["mountain_left"], shaders["Simple"], texture);
	RenderMountain(meshes["mountain_right"], shaders["Simple"], texture);
	RenderMountain(meshes["grand_mountain_left"], shaders["Simple"], texture);
	RenderMountain(meshes["grand_mountain_right"], shaders["Simple"], texture);

}

void Tema1::RenderBoat(std::shared_ptr<Mesh>& mesh, std::shared_ptr<Shader>& shader)
{
	auto camera = GetSceneCamera();
	if (!mesh || !shader || !shader->GetProgramID())
		return;

	shader->Use();
	glm::mat4 model = glm::mat4(1);

	// translate the boat
	model = glm::translate(glm::mat4(1), glm::vec3(CommonPoint) + glm::vec3(0, 0.05f, 0));
	model = glm::scale(model, glm::vec3(0.1));
	model = glm::rotate(model, RADIANS(-90.0f), glm::vec3(1, 0, 0));

	// Use directions OnInputsUpdate
	model = glm::translate(model, glm::vec3(translateX, translateY, translateZ));
	model = glm::rotate(model, RADIANS(angularStepOX), glm::vec3(1, 0, 0));
	model = glm::rotate(model, RADIANS(angularStepOY), glm::vec3(0, 1, 0));
	model = glm::rotate(model, RADIANS(angularStepOZ), glm::vec3(0, 0, 1));

	glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(model));

	// Bind view matrix
	glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix
	glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	auto cameraPosition = camera->transform->GetWorldPosition();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
	int loc_texture = shader->GetUniformLocation("texture_cubemap");
	glUniform1i(loc_texture, 0);

	int loc_camera = shader->GetUniformLocation("camera_position");
	glUniform3f(loc_camera, cameraPosition.x, cameraPosition.y, cameraPosition.z);

	mesh->Render();
}

void Tema1::RenderCube(std::shared_ptr<Mesh>& mesh, std::shared_ptr<Shader>& shader, int cubeMapTextureID)
{
	if (!mesh || !shader || !shader->program)
		return;

	shader->Use();
	glm::mat4 model(1);
	model = glm::translate(model, glm::vec3(0, 14.99f, 0));
	model = glm::scale(model, glm::vec3(30));

	glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(model));

	// Bind view matrix
	glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix
	glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
	int loc_texture = shader->GetUniformLocation("texture_cubemap");
	glUniform1i(loc_texture, 0);

	mesh->Render();
}

void Tema1::RenderMeshObject(std::shared_ptr<Mesh>& mesh, std::shared_ptr<Shader>& shader, Texture2D* texture,
	const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotate)
{
	if (!mesh || !shader || !shader->program)
		return;

	// render an object using the specified shader 
	shader->Use();

	// Build Model
	glm::mat4 model(1);
	model = glm::translate(model, position);
	model = glm::scale(model, scale);

	// Bind model matrix
	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(model));

	// Bind view matrix
	glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix
	glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));


	// Apply textures if needed
	if (texture)
	{
		mesh->UseMaterials(false);
		texture->BindToTextureUnit(GL_TEXTURE0);
		glUniform1i(shader->loc_textures[0], 0);
	}
	else
	{
		mesh->UseMaterials(true);
	}

	// Render mesh with textures
	mesh->Render();

	if (texture)
		texture->UnBind();
}

// render mountain
void Tema1::RenderMountain(std::shared_ptr<Mesh>& mesh, std::shared_ptr<Shader>& shader, Texture2D* texture)
{
	if (!mesh || !shader || !shader->GetProgramID() || !texture)
		return;

	shader->Use();
	// Build Model
	glm::mat4 model(1);

	// Bind model matrix
	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(model));

	// Bind view matrix
	glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix
	glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	// Texture
	mesh->UseMaterials(false);
	texture->BindToTextureUnit(GL_TEXTURE0);
	glUniform1i(shader->loc_textures[0], 0);

	// Render mesh with textures
	mesh->Render();

	// Check for OpenGL errors
	CheckOpenGLError();

	texture->UnBind();
}


// render river
void Tema1::RenderRiver(Texture2D* texture)
{
	auto mesh = meshes["river"];
	auto shader = shaders["BezierCurve"];

	if (!mesh || !shader || !shader->GetProgramID() || !texture)
		return;

	shader->Use();

	// Build Model
	glm::mat4 model(1);

	// Bind model matrix
	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(model));

	// Bind view matrix
	glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix
	glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	// Send control points
	for (int i = 0; i < controlPointsCount; i++)
	{
		std::string name = "control_points[" + std::to_string(i) + "]";
		int loc = glGetUniformLocation(shader->program, name.c_str());
		glUniform3fv(loc, 1, glm::value_ptr(controlPoints[i]));
	}

	// Send other parameters
	int loc = glGetUniformLocation(shader->program, "generated_points_count");
	glUniform1i(loc, generatedPoints);
	loc = glGetUniformLocation(shader->program, "surface_width");
	glUniform1f(loc, riverWidth);
	loc = glGetUniformLocation(shader->program, "no_of_instances");
	glUniform1i(loc, instanceCount);

	// River flow
	loc = glGetUniformLocation(shader->program, "time");
	glUniform1f(loc, Engine::GetElapsedTime());
	loc = glGetUniformLocation(shader->program, "speed");
	glUniform1f(loc, animationSpeed);
	loc = glGetUniformLocation(shader->program, "tilingFactor");
	glUniform1f(loc, tilingFactor);

	// Texture
	texture->BindToTextureUnit(GL_TEXTURE0);
	glUniform1i(shader->loc_textures[0], 0);

	// Draw the object instanced
	glBindVertexArray(mesh->GetBuffers()->VAO);
	glDrawElementsInstanced(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, (void*)0, instanceCount);
	glBindVertexArray(0);

	texture->UnBind();
}

// render river first
void Tema1::RenderRiverFirst(Texture2D* texture)
{
	auto mesh = meshes["river_first"];
	auto shader = shaders["BezierCurve"];

	if (!mesh || !shader || !shader->GetProgramID() || !texture)
		return;

	shader->Use();

	// Build Model
	glm::mat4 model(1);

	// Bind model matrix
	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(model));

	// Bind view matrix
	glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix
	glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	// Send control points
	for (int i = 0; i < controlPointsCountOne; i++)
	{
		std::string name = "control_points[" + std::to_string(i) + "]";
		int loc = glGetUniformLocation(shader->program, name.c_str());
		glUniform3fv(loc, 1, glm::value_ptr(controlPointsOne[i]));
	}

	// Send other parameters
	int loc = glGetUniformLocation(shader->program, "generated_points_count");
	glUniform1i(loc, generatedPoints);
	loc = glGetUniformLocation(shader->program, "surface_width");
	glUniform1f(loc, riverWidth);
	loc = glGetUniformLocation(shader->program, "no_of_instances");
	glUniform1i(loc, instanceCount);

	// River flow
	loc = glGetUniformLocation(shader->program, "time");
	glUniform1f(loc, Engine::GetElapsedTime());
	loc = glGetUniformLocation(shader->program, "speed");
	glUniform1f(loc, animationSpeed);
	loc = glGetUniformLocation(shader->program, "tilingFactor");
	glUniform1f(loc, tilingFactor);

	// Texture
	texture->BindToTextureUnit(GL_TEXTURE0);
	glUniform1i(shader->loc_textures[0], 0);

	// Draw the object instanced
	glBindVertexArray(mesh->GetBuffers()->VAO);
	glDrawElementsInstanced(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, (void*)0, instanceCount);
	glBindVertexArray(0);

	texture->UnBind();
}

// render river second
void Tema1::RenderRiverSecond(Texture2D* texture)
{
	auto mesh = meshes["river_second"];
	auto shader = shaders["BezierCurve"];

	if (!mesh || !shader || !shader->GetProgramID() || !texture)
		return;

	shader->Use();

	// Build Model
	glm::mat4 model(1);

	// Bind model matrix
	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(model));

	// Bind view matrix
	glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix
	glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	// Send control points
	for (int i = 0; i < controlPointsCountOne; i++)
	{
		std::string name = "control_points[" + std::to_string(i) + "]";
		int loc = glGetUniformLocation(shader->program, name.c_str());
		glUniform3fv(loc, 1, glm::value_ptr(controlPointsTwo[i]));
	}

	// Send other parameters
	int loc = glGetUniformLocation(shader->program, "generated_points_count");
	glUniform1i(loc, generatedPoints);
	loc = glGetUniformLocation(shader->program, "surface_width");
	glUniform1f(loc, riverWidth);
	loc = glGetUniformLocation(shader->program, "no_of_instances");
	glUniform1i(loc, instanceCount);

	// River flow
	loc = glGetUniformLocation(shader->program, "time");
	glUniform1f(loc, Engine::GetElapsedTime());
	loc = glGetUniformLocation(shader->program, "speed");
	glUniform1f(loc, animationSpeed);
	loc = glGetUniformLocation(shader->program, "tilingFactor");
	glUniform1f(loc, tilingFactor);

	// Texture
	texture->BindToTextureUnit(GL_TEXTURE0);
	glUniform1i(shader->loc_textures[0], 0);

	// Draw the object instanced
	glBindVertexArray(mesh->GetBuffers()->VAO);
	glDrawElementsInstanced(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, (void*)0, instanceCount);
	glBindVertexArray(0);

	texture->UnBind();
}

// render river third
void Tema1::RenderRiverThird(Texture2D* texture)
{
	auto mesh = meshes["river_third"];
	auto shader = shaders["BezierCurve"];

	if (!mesh || !shader || !shader->GetProgramID() || !texture)
		return;

	shader->Use();

	// Build Model
	glm::mat4 model(1);

	// Bind model matrix
	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(model));

	// Bind view matrix
	glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix
	glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	// Send control points
	for (int i = 0; i < controlPointsCountOne; i++)
	{
		std::string name = "control_points[" + std::to_string(i) + "]";
		int loc = glGetUniformLocation(shader->program, name.c_str());
		glUniform3fv(loc, 1, glm::value_ptr(controlPointsThree[i]));
	}

	// Send other parameters
	int loc = glGetUniformLocation(shader->program, "generated_points_count");
	glUniform1i(loc, generatedPoints);
	loc = glGetUniformLocation(shader->program, "surface_width");
	glUniform1f(loc, riverWidth);
	loc = glGetUniformLocation(shader->program, "no_of_instances");
	glUniform1i(loc, instanceCount);

	// River flow
	loc = glGetUniformLocation(shader->program, "time");
	glUniform1f(loc, Engine::GetElapsedTime());
	loc = glGetUniformLocation(shader->program, "speed");
	glUniform1f(loc, animationSpeed);
	loc = glGetUniformLocation(shader->program, "tilingFactor");
	glUniform1f(loc, tilingFactor);

	// Texture
	texture->BindToTextureUnit(GL_TEXTURE0);
	glUniform1i(shader->loc_textures[0], 0);

	// Draw the object instanced
	glBindVertexArray(mesh->GetBuffers()->VAO);
	glDrawElementsInstanced(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, (void*)0, instanceCount);
	glBindVertexArray(0);

	texture->UnBind();
}

void Tema1::FrameEnd()
{
	// DrawCoordinatSystem();
}

unsigned int Tema1::UploadCubeMapTexture(const std::string& posx, const std::string& posy, const std::string& posz, const std::string& negx, const std::string& negy, const std::string& negz)
{
	int width, height, chn;

	unsigned char* data_posx = stbi_load(posx.c_str(), &width, &height, &chn, 0);
	unsigned char* data_posy = stbi_load(posy.c_str(), &width, &height, &chn, 0);
	unsigned char* data_posz = stbi_load(posz.c_str(), &width, &height, &chn, 0);
	unsigned char* data_negx = stbi_load(negx.c_str(), &width, &height, &chn, 0);
	unsigned char* data_negy = stbi_load(negy.c_str(), &width, &height, &chn, 0);
	unsigned char* data_negz = stbi_load(negz.c_str(), &width, &height, &chn, 0);

	// Create OpenGL texture
	unsigned int textureID = 0;
	glGenTextures(1, &textureID);

	// Bind the texture
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	float maxAnisotropy;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Load texture information for each face
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_posx);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_posy);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_posz);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_negx);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_negy);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_negz);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// free memory
	SAFE_FREE(data_posx);
	SAFE_FREE(data_posy);
	SAFE_FREE(data_posz);
	SAFE_FREE(data_negx);
	SAFE_FREE(data_negy);
	SAFE_FREE(data_negz);

	return textureID;
}

// Read the documentation of the following functions in: "Source/Core/Window/InputController.h" or
// https://github.com/UPB-Graphics/SPG-Framework/blob/master/Source/Core/Window/InputController.h

void Tema1::OnInputUpdate(float deltaTime, int mods)
{
	// TODO
	if (window->KeyHold(GLFW_KEY_W))
		translateZ -= 10 * deltaTime;
	if (window->KeyHold(GLFW_KEY_D))
		translateX += 10 * deltaTime;
	if (window->KeyHold(GLFW_KEY_A))
		translateX -= 10 * deltaTime;
	if (window->KeyHold(GLFW_KEY_S))
		translateZ += 10 * deltaTime;
	if (window->KeyHold(GLFW_KEY_R))
		translateY += 10 * deltaTime;
	if (window->KeyHold(GLFW_KEY_F))
		translateY -= 10 * deltaTime;

	if (window->KeyHold(GLFW_KEY_3)) {
		angularStepOX += 10 * deltaTime;
	}
	if (window->KeyHold(GLFW_KEY_4)) {
		angularStepOX -= 10 * deltaTime;
	}
	if (window->KeyHold(GLFW_KEY_5)) {
		angularStepOY += 10 * deltaTime;
	}
	if (window->KeyHold(GLFW_KEY_6)) {
		angularStepOY -= 10 * deltaTime;
	}
	if (window->KeyHold(GLFW_KEY_7)) {
		angularStepOZ += 10 * deltaTime;
	}
	if (window->KeyHold(GLFW_KEY_8)) {
		angularStepOZ -= 10 * deltaTime;
	}
}

void Tema1::OnKeyPress(int key, int mods)
{
	
};

void Tema1::OnKeyRelease(int key, int mods)
{
	// add key release event
};

void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// add mouse move event
};

void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	
};

void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button release event
}

void Tema1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
	// treat mouse scroll event
}

void Tema1::OnWindowResize(int width, int height)
{
	// treat window resize event
}
