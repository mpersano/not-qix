#pragma once

#include "script_interface.h"
#include "foe.h"

namespace ggl {
class sprite;
class texture;
}

class boss : public foe
{
public:
	boss(game& g, const vec2f& pos);

	void draw(ggl::sprite_batch& sb) const override;
	bool update() override;

	void on_miniboss_killed();

	void set_pod_angle(float a);
	void set_pod_position(int pod, float da, float r);

	void rotate_pods_to_player();
	void rotate_pods(float a);

	void fire_bullet(int pod);
	void fire_laser(int pod, float power);

	static const int RADIUS = 30;

private:
	bool intersects_children(const vec2i& from, const vec2i& to) const override;
	bool intersects_children(const vec2i& center, float radius) const override;

	void draw_core(ggl::sprite_batch& sb) const;
	void draw_pods(ggl::sprite_batch& sb) const;

	float pod_angle_;
	int cur_pod_formation_;

	static const int NUM_PODS = 5;

	class pod {
	public:
		pod(game& g);

		void draw(ggl::sprite_batch& sb) const;
		void update();

		void fire_bullet(const vec2f& center, float angle);
		void fire_laser(float power);

		bool intersects(const vec2f& center, float angle, const vec2i& from, const vec2i& to) const;

		float ang_offset;
		float rotation;

	private:
		float get_laser_radius() const;

		game& game_;
		const ggl::sprite *sprite_;
		const ggl::sprite *muzzle_flash_sprite_;
		const ggl::sprite *laser_flash_sprite_;
		const ggl::texture *laser_segment_texture_;
		int fire_tics_;
		float laser_power_;
	};

	std::vector<std::unique_ptr<pod>> pods_;

	int miniboss_spawned_;

	const ggl::sprite *core_sprite_;
	const ggl::sprite *danger_up_sprite_, *danger_down_sprite_;

	std::unique_ptr<script_thread> script_thread_;
};
