#pragma once

class ScoreKeeper
{
private:
	unsigned int score = 0;

public:
	// Add points to the score and return the new total score
	unsigned int addPoints(unsigned int points)
	{
		score += points;
		return score;
	}

	// Get the current score
	unsigned int getScore() const
	{
		return score;
	}

	void reset()
	{
		score = 0;
	}
};
