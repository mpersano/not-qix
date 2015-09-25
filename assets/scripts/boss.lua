local STATE_CHASING	= 0

local STATE_PRE_FIRING	= 1
local STATE_FIRING	= 2
local STATE_POST_FIRING	= 3
local STATE_LASER	= 4

local BOSS_SPEED = 2

local NUM_PODS = 3

local PI = 3.14159265

v.state = STATE_CHASING
v.state_tics = 0

local formation_chasing = { { da = -2*PI/3, r = PI/2 }, { da = 0, r = PI/2 }, { da = 2*PI/3, r = PI/2 } }
local formation_firing = { { da = -.25, r = 0 }, { da = 0, r = 0 }, { da = .25, r = 0 } }
local formation_laser = { { da = -2*PI/3, r = 0 }, { da = 0, r = 0 }, { da = 2*PI/3, r = 0 } }

local function set_pod_formation(self, formation)
	for i = 1, NUM_PODS do
		boss_set_pod_position(self, i - 1, formation[i].da, formation[i].r)
	end
end

local function set_pod_formation_lerp(self, from, to, t)
	for i = 1, NUM_PODS do
		local da = from[i].da + t*(to[i].da - from[i].da)
		local r = from[i].r + t*(to[i].r - from[i].r)

		boss_set_pod_position(self, i - 1, da, r)
	end
end

local function fire_all_lasers(self, power)
	for i = 0, NUM_PODS - 1 do
		boss_fire_laser(self, i, power)
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
	-- set_pod_formation_lerp(self, formation_chasing, formation_firing, 1 - t)
	set_pod_formation_lerp(self, formation_chasing, formation_laser, 1 - t)
	boss_rotate_pods_to_player(self)

	if v.state_tics == STOP_TICS then
		-- set_state(self, STATE_FIRING)
		set_state(self, STATE_LASER)
	end
end

local function update_firing(self)
	local FIRE_TICS = 90

	if v.state_tics % 30 == 0 then
		for i = 0, NUM_PODS - 1 do
			boss_fire_bullet(self, i)
		end
	end

	boss_rotate_pods_to_player(self)

	if v.state_tics == FIRE_TICS then
		set_state(self, STATE_POST_FIRING)
	end
end

local function update_laser(self)
	local LASER_TICS = 90

	if v.state_tics < 20 then
		local t = v.state_tics/20
		fire_all_lasers(self, t)
	end

	if v.state_tics == LASER_TICS then
		fire_all_lasers(self, 0)
		set_state(self, STATE_POST_FIRING)
	end
end

local function update_post_firing(self)
	local RECOVER_TICS = 90

	foe_rotate_to_player(self)

	local t = v.state_tics/RECOVER_TICS

	foe_set_speed(self, t*BOSS_SPEED)
	-- set_pod_formation_lerp(self, formation_firing, formation_chasing, t)
	set_pod_formation_lerp(self, formation_laser, formation_chasing, t)
	boss_rotate_pods_to_player(self)

	if v.state_tics == RECOVER_TICS then
		set_state(self, STATE_CHASING)
	end
end

function init(self)
	set_state(self, STATE_CHASING)

	foe_set_direction(self, 1, 0)
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
	elseif v.state == STATE_LASER then
		update_laser(self)
	elseif v.state == STATE_POST_FIRING then
		update_post_firing(self)
	end
end
