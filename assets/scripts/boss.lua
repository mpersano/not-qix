local STATE_CHASING		= 0

local STATE_BEFORE_ATTACK	= 1
local STATE_AFTER_ATTACK	= 2

local STATE_FIRING		= 3
local STATE_LASER		= 4

local SPEED = 2
local POD_ANG_SPEED = .1

local NUM_PODS = 5
local NUM_ATTACKS = 2

local PI = 3.14159265

local FORMATION_CHASING =
	{ { da = -4*PI/5, r = PI/2 },
	  { da = -2*PI/5, r = PI/2 },
	  { da =       0, r = PI/2 },
	  { da =  2*PI/5, r = PI/2 },
	  { da =  4*PI/5, r = PI/2 } }

local FORMATION_FIRING =
	{ { da =  -.3, r = 0 },
	  { da = -.15, r = 0 },
	  { da =    0, r = 0 },
	  { da =  .15, r = 0 },
	  { da =   .3, r = 0 } }

local FORMATION_LASER =
	{ { da = -4*PI/5, r = 0 },
	  { da = -2*PI/5, r = 0 },
	  { da =       0, r = 0 },
	  { da =  2*PI/5, r = 0 },
	  { da =  4*PI/5, r = 0 } }

local ATTACKS =
	{ { formation = FORMATION_FIRING, state = STATE_FIRING },
	  { formation = FORMATION_LASER, state = STATE_LASER } }

v.state = STATE_CHASING

v.state_tics = 0
v.attack = 1

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

	boss_rotate_pods(self, POD_ANG_SPEED)

	if v.state_tics == 60 then
		set_state(self, STATE_BEFORE_ATTACK)
	end
end

local function update_before_attack(self)
	local STOP_TICS = 90

	local t = 1 - v.state_tics/STOP_TICS

	foe_set_speed(self, t*SPEED)
	boss_rotate_pods(self, t*POD_ANG_SPEED)

	set_pod_formation_lerp(self, FORMATION_CHASING, ATTACKS[v.attack].formation, 1 - t)

	if v.state_tics == STOP_TICS then
		set_state(self, ATTACKS[v.attack].state)
	end
end

local function update_after_attack(self)
	local RECOVER_TICS = 90

	local t = v.state_tics/RECOVER_TICS

	foe_set_speed(self, t*SPEED)
	boss_rotate_pods(self, t*POD_ANG_SPEED)

	set_pod_formation_lerp(self, ATTACKS[v.attack].formation, FORMATION_CHASING, t)

	if v.state_tics == RECOVER_TICS then
		v.attack = (v.attack % NUM_ATTACKS) + 1
		set_state(self, STATE_CHASING)
	end
end

local function update_firing(self)
	local FIRE_TICS = 120 
	local AIM_TICKS = 20

	if v.state_tics > AIM_TICKS and (v.state_tics - AIM_TICKS) % 30 == 0 then
		for i = 0, NUM_PODS - 1 do
			boss_fire_bullet(self, i)
		end
	end

	boss_rotate_pods_to_player(self)

	if v.state_tics == FIRE_TICS then
		set_state(self, STATE_AFTER_ATTACK)
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
		set_state(self, STATE_AFTER_ATTACK)
	end
end

function init(self)
	set_state(self, STATE_CHASING)

	foe_set_direction(self, 1, 0)
	foe_set_speed(self, SPEED)

	boss_set_pod_angle(self, 0)
	set_pod_formation(self, FORMATION_CHASING)
end

function update(self)
	foe_update_position(self)

	v.state_tics = v.state_tics + 1

	if v.state == STATE_CHASING then
		update_chasing(self)
	elseif v.state == STATE_BEFORE_ATTACK then
		update_before_attack(self)
	elseif v.state == STATE_AFTER_ATTACK then
		update_after_attack(self)
	elseif v.state == STATE_FIRING then
		update_firing(self)
	elseif v.state == STATE_LASER then
		update_laser(self)
	end
end
