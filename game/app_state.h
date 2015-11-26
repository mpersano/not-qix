#pragma once

class game_app;

class app_state
{
public:
	app_state(game_app& app);

	virtual ~app_state() = default;

	virtual void draw() const = 0;
	virtual void update() = 0;

protected:
	game_app& app_;
};
