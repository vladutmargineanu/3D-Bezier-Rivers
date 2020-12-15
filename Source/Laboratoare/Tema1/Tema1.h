#pragma once

#include <Component/SimpleScene.h>
#include <unordered_map>

#include "Particle.h"


class Tema1 : public SimpleScene
{
public:
	Tema1();
	~Tema1();

	void Init() override;

private:
	void FrameStart() override;
	void Update(float deltaTimeSeconds) override;
	void FrameEnd() override;
	unsigned int UploadCubeMapTexture(const std::string& posx, const std::string& posy, const std::string& posz, const std::string& negx, const std::string& negy, const std::string& negz);
	// Input controls
	void OnInputUpdate(float deltaTime, int mods) override;
	void OnKeyPress(int key, int mods) override;
	void OnKeyRelease(int key, int mods) override;
	void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
	void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
	void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
	void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
	void OnWindowResize(int width, int height) override;

	// Sets the scene parameters to their default values
	void DefaultParameters();

	// Render boat
	void RenderBoat(std::shared_ptr<Mesh>& mesh, std::shared_ptr<Shader>& shader);

	// Render Cube
	void RenderCube(std::shared_ptr<Mesh>& mesh, std::shared_ptr<Shader>& shader, int cubeMapTextureID);

	// Basic rendering of objects
	void RenderMeshObject(std::shared_ptr<Mesh>& mesh, std::shared_ptr<Shader>& shader, Texture2D* texture,
		const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotate);

	// Specific rendering of the curve
	void RenderRiver(Texture2D* texture);

	// Specific rendering of the curve
	void RenderRiverFirst(Texture2D* texture);

	// Specific rendering of the curve
	void RenderRiverSecond(Texture2D* texture);

	// Specific rendering of the curve
	void RenderRiverThird(Texture2D* texture);

	// Render Mountain
	void RenderMountain(std::shared_ptr<Mesh>& mesh, std::shared_ptr<Shader>& shader, Texture2D* texture);

private:
	// Resource managers
	std::unordered_map< std::string, std::shared_ptr<Mesh> > meshes;
	std::unordered_map< std::string, std::shared_ptr<Shader> > shaders;
	std::unordered_map< std::string, std::shared_ptr<Texture2D> > textures;

	// Control points
	unsigned int controlPointsCountOne;
	unsigned int controlPointsCount;

	std::vector< glm::vec3 > controlPoints;
	std::vector< glm::vec3 > controlPointsOne;
	std::vector< glm::vec3 > controlPointsTwo;
	std::vector< glm::vec3 > controlPointsThree;

	// Generate random points
	int firstPoint;

	int min = 2;
	int max = 4;
	int min_left = 0;
	int max_left = 4;
	int min_middle = 8;
	int max_middle = 10;
	int min_right = 4;
	int max_right = 8;

	// Calculate points for the mountains
	glm::vec3 CommonPoint;
	glm::vec3 leftFirst;
	glm::vec3 leftSecond;
	glm::vec3 leftThird;

	glm::vec3 rightFirst;
	glm::vec3 rightSecond;
	glm::vec3 rightThird;
	
	// Particle Effect
	std::unique_ptr< ParticleEffect<Particle> > splashEffect;
	glm::vec3 particleFallSpeed;

	// River animation
	float animationSpeed;
	float tilingFactor;

	// Editing
	float smoothness;
	float maxAnimationSpeed;

	// Curve generation parameters
	unsigned int instanceCount;
	unsigned int generatedPoints;
	float riverWidth;
	float maxTranslate;
	float maxRotate;
	int cubeMapTextureID;

	float translateX, translateY, translateZ;
	float angularStepOX, angularStepOY, angularStepOZ;
	unsigned int nrParticles;
	ParticleEffect<Particle>* particleEffect;
};
