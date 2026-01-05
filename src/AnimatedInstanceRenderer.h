#pragma once

#include "InstancedRenderer.h"

/**
 * @brief A single frame of an animated model's animation sequence.
 *
 * Out animation system is basically just a series of these frames played in sequence.
 *
 */
struct AnimatedInstanceFrame
{
	AnimationState m_state;				   // The animation state this frame belongs to
	float m_duration;					   // How long to display this frame (in seconds)
	InstancedRenderer m_InstancedRenderer; // The instanced renderer for this frame
};


/**
 * @brief Renders many instances of an animated model using GPU instancing.
 *
 * This class manages several InstanceRenderers, one for each animation
 *
 */
class AnimatedInstanceRenderer : public Renderable
{
public:
	AnimatedInstanceRenderer() = default;
	~AnimatedInstanceRenderer() = default;

	/**
	 * @brief Create an animated instance frame from a model file.
	 *
	 * @param modelPath Path to the model file
	 * @param state Animation state this frame belongs to
	 * @param duration Duration to display this frame (in seconds)
	 * @return std::unique_ptr<AnimatedInstanceFrame> The created animated instance frame
	 */
	static std::unique_ptr<AnimatedInstanceFrame> createAnimatedInstanceFrame(const std::filesystem::path &modelPath, AnimationState state, float duration);
	
	/**
	 * @brief Add an animation frame to this renderer.
	 * Frames need to be added in the order they should be played.
	 * 
	 * @param frame The frame to add
	 */
	void addAnimationFrame(std::unique_ptr<AnimatedInstanceFrame> frame);

	void updateFogUniforms(const glm::vec3 &fogColor, float fogStart, float fogEnd);

	void updateInstances(std::unordered_map<AnimationState, std::vector<glm::mat4>> &instanceTransformsByState, float dt);
	
	void render(const glm::mat4 view, const glm::mat4 projection, const PhongLightConfig *phongLight) override;

	std::unordered_map<AnimationState, std::vector<std::unique_ptr<AnimatedInstanceFrame>>> m_animationFrames; // Map of animation states to their frames

private:
	std::unordered_map<AnimationState, float> m_animationTimers; // Map of animation states to their timers
};