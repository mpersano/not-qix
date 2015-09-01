local STATE_CHASING	= 0
local STATE_PRE_FIRING	= 1
local STATE_FIRING	= 2
local STATE_POST_FIRING	= 3

local BOSS_SPEED = 2

v.state = STATE_CHASING
v.state_tics = 0
v.fire_tics = 0

local function set_state(self, new_state)
	v.state = new_state
	v.state_tics = 0
end

local function update_chasing(self)
	foe_rotate_to_player(self)

	if v.state_tics == 60 then
		set_state(self, STATE_PRE_FIRING)
	end
end

local function update_pre_firing(self)
	local STOP_TICS = 90

	foe_rotate_to_player(self)

	local t = 1 - v.state_tics/STOP_TICS

	foe_set_speed(self, t*BOSS_SPEED)
	boss_set_spike_dispersion(self, 1 - t)
	boss_rotate_spike_to_player(self)

	if v.state_tics == STOP_TICS then
		set_state(self, STATE_FIRING)
	end
end

local function update_firing(self)
	local FIRE_TICS = 90

	if v.fire_tics == 0 then
		boss_fire_bullet(self)
		v.fire_tics = 30
	else
		v.fire_tics = v.fire_tics - 1
	end

	boss_rotate_spike_to_player(self)

	if v.state_tics == FIRE_TICS then
		set_state(self, STATE_POST_FIRING)
	end
end

local function update_post_firing(self)
	local RECOVER_TICS = 90

	foe_rotate_to_player(self)

	local t = v.state_tics/RECOVER_TICS

	foe_set_speed(self, t*BOSS_SPEED)
	boss_set_spike_dispersion(self, 1 - t)
	boss_rotate_spike_to_player(self)

	if v.state_tics == RECOVER_TICS then
		set_state(self, STATE_CHASING)
	end
end

function init(self)
	set_state(self, STATE_CHASING)

	foe_set_speed(self, BOSS_SPEED)

	boss_set_spike_angle(self, 0)
	boss_set_spike_dispersion(self, 0)
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
