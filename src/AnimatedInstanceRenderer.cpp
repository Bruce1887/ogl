#include "AnimatedInstanceRenderer.h"

void AnimatedInstanceRenderer::addAnimationFrame(std::unique_ptr<AnimatedInstanceFrame> frame)
{
#ifdef DEBUG
	assert(frame.get()->m_duration > 0.0f);
#endif
	m_animationFrames[frame->m_state].push_back(std::move(frame));
}

void AnimatedInstanceRenderer::updateFogUniforms(const glm::vec3 &fogColor, float fogStart, float fogEnd)
{
	for (auto &kv : m_animationFrames)
	{
		for (auto &frame : kv.second)
		{
			frame->m_InstancedRenderer.setFogUniforms(fogColor, fogStart, fogEnd);
		}
	}
}

void AnimatedInstanceRenderer::updateInstances(std::unordered_map<AnimationState, std::vector<glm::mat4>> &instanceTransformsByState, float dt)
{
	// Now process each animation state
	for (auto &[state, transforms] : instanceTransformsByState)
	{
		// Update animation timer for this state
		float &timer = m_animationTimers[state];
		timer += dt;

		auto &frames = m_animationFrames[state];
		if (frames.empty())
			continue;

		// CLEAR ALL FRAMES FOR THIS STATE FIRST
		for (auto &frame : frames)
		{
			frame->m_InstancedRenderer.clearInstances();
		}

		// Calculate total duration for this animation state
		float totalDuration = 0.0f;
		for (const auto &frame : frames)
		{
			totalDuration += frame->m_duration;
		}

		// Loop the timer if it exceeds total duration
		timer = std::fmod(timer, totalDuration);

		// Determine which frame to use based on the animation timer
		float timeAccumulator = 0.0f;
		for (auto &frame : frames)
		{
			timeAccumulator += frame->m_duration;
			if (timer < timeAccumulator)
			{
				// Found the correct frame - update its renderer with all transforms at once
				frame->m_InstancedRenderer.replaceInstances(transforms);
				break;
			}
		}
	}

	// Clear any animation states that have no instances this update
	for (auto &kv : m_animationFrames)
	{
		AnimationState state = kv.first;
		if (instanceTransformsByState.find(state) == instanceTransformsByState.end())
		{
			// No instances for this state - clear all its frames
			for (auto &frame : kv.second)
			{
				frame->m_InstancedRenderer.clearInstances();
			}
		}
	}
}

void AnimatedInstanceRenderer::render(const glm::mat4 view, const glm::mat4 projection, const PhongLightConfig *phongLight)
{
	// Render all frames' instance renderers
	for (auto &kv : m_animationFrames)
	{
		for (auto &frame : kv.second)
		{
			frame->m_InstancedRenderer.render(view, projection, phongLight);
		}
	}
}

std::unique_ptr<AnimatedInstanceFrame> AnimatedInstanceRenderer::createAnimatedInstanceFrame(const std::filesystem::path &modelPath, AnimationState state, float duration, std::shared_ptr<Shader> shader)
{
	auto frame = std::make_unique<AnimatedInstanceFrame>();
	frame->m_state = state;
	frame->m_duration = duration;
	std::unique_ptr<Model> model = std::make_unique<Model>(modelPath);

	frame->m_InstancedRenderer.init(std::move(model), shader);
	return frame;
}