#pragma once

#include "script_interface.h"
#include "foe.h"

namespace ggl {
class sprite;
}

class boss : public foe
{
public:
	boss(game& g, const vec2f& pos);

	void draw() const override;
	bool update() override;

	void on_miniboss_killed();

	void set_pod_angle(float a);
	void set_pod_position(int pod, float da, float r);

	void rotate_pods_to_player();
	void rotate_pods(float a);

	void fire_bullet(int pod);

	static const int RADIUS = 30;

private:
	void draw_core() const;
	void draw_pods() const;

	float pod_angle_;
	int cur_pod_formation_;

	static const int NUM_PODS = 3;

	class pod {
	public:
		pod(game& g);

		void draw() const;
		void update();

		void fire_bullet(const vec2f& center, float angle);

		float ang_offset;
		float rotation;

	private:
		game& game_;
		const ggl::sprite *sprite_;
		int fire_tics_;
	} pods_[NUM_PODS];

	int miniboss_spawned_;

	const ggl::sprite *core_sprite_;
	std::unique_ptr<script_thread> script_thread_;
};
