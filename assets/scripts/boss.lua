local STATE_CHASING	= 0
local STATE_PRE_FIRING	= 1
local STATE_FIRING	= 2
local STATE_POST_FIRING	= 3

local BOSS_SPEED = 2

local NUM_PODS = 3

local PI = 3.14159265

v.state = STATE_CHASING

v.state_tics = 0

local formation_chasing = { { da = -2*PI/3, r = PI/2 }, { da = 0, r = PI/2 }, { da = 2*PI/3, r = PI/2 } }
local formation_firing = { { da = -.25, r = 0 }, { da = 0, r = 0 }, { da = .25, r = 0 } }

local function set_pod_formation(self, formation)
	for i = 1, NUM_PODS do
		boss_set_pod_position(self, i - 1, formation[i].da, formation[i].r)
	end
end

local function set_pod_formation_lerp(self, formation_from, formation_to, t)
	for i = 1, NUM_PODS do
		local da = formation_from[i].da + t*(formation_to[i].da - formation_from[i].da)
		local r = formation_from[i].r + t*(formation_to[i].r - formation_from[i].r)

		boss_set_pod_position(self, i - 1, da, r)
	end
end

local function set_state(self, new_state)
	v.state = new_state
	v.state_tics = 0
end

local function update_chasing(self)
	foe_rotate_to_player(self)

	boss_rotate_pods(self, .1)

	if v.state_tics == 60 then
		set_state(self, STATE_PRE_FIRING)
	end
end

local function update_pre_firing(self)
	local STOP_TICS = 90

	foe_rotate_to_player(self)

	local t = 1 - v.state_tics/STOP_TICS

	foe_set_speed(self, t*BOSS_SPEED)
	set_pod_formation_lerp(self, formation_chasing, formation_firing, 1 - t)
	boss_rotate_pods_to_player(self)

	if v.state_tics == STOP_TICS then
		set_state(self, STATE_FIRING)
	end
end

local function update_firing(self)
	local FIRE_TICS = 90

	if v.state_tics % 30 == 0 then
		boss_fire_bullet(self, 0)
		boss_fire_bullet(self, 1)
		boss_fire_bullet(self, 2)
	end

	boss_rotate_pods_to_player(self)

	if v.state_tics == FIRE_TICS then
		set_state(self, STATE_POST_FIRING)
	end
end

local function update_post_firing(self)
	local RECOVER_TICS = 90

	foe_rotate_to_player(self)

	local t = v.state_tics/RECOVER_TICS

	foe_set_speed(self, t*BOSS_SPEED)
	set_pod_formation_lerp(self, formation_firing, formation_chasing, t)
	boss_rotate_pods_to_player(self)

	if v.state_tics == RECOVER_TICS then
		set_state(self, STATE_CHASING)
	end
end

function init(self)
	set_state(self, STATE_CHASING)

	foe_set_speed(self, BOSS_SPEED)

	boss_set_pod_angle(self, 0)
	set_pod_formation(self, formation_chasing)
end

function update(self)
	foe_update_position(self)

	v.state_tics = v.state_tics + 1

	if v.state == STATE_CHASING then
		update_chasing(self)
	elseif v.state == STATE_PRE_FIRING then
		update_pre_firing(self)
	elseif v.state == STATE_FIRING then
		update_firing(self)
	elseif v.state == STATE_POST_FIRING then
		update_post_firing(self)
	end
end
