local SIN_TABLE = {
	 0.0000,  0.1951,  0.3827,  0.5556,
	 0.7071,  0.8315,  0.9239,  0.9808,
	 1.0000,  0.9808,  0.9239,  0.8315,
	 0.7071,  0.5556,  0.3827,  0.1951,
	 0.0000, -0.1951, -0.3827, -0.5556,
	-0.7071, -0.8315, -0.9239, -0.9808,
	-1.0000, -0.9808, -0.9239, -0.8315,
	-0.7071, -0.5556, -0.3827, -0.1951 }

local STATE_MOVING		= 0
local STATE_THINKING		= 1

local COOL_DOWN_TICS		= 40

v.state = STATE_MOVING

v.speed = 0
v.tics = 0
v.state_tics = 0
v.move_tics = 0

local function start_moving(self)
	v.state = STATE_MOVING
	v.state_tics = 0
	v.move_tics = rand(300, 1000)
	v.speed = rand(.6, 1.2)
end

local function start_thinking(self)
	v.state = STATE_THINKING
	v.state_tics = 0
end

local function update_moving(self)
	foe_rotate(self, .1*SIN_TABLE[1 + (v.tics % 32)])

	if v.state_tics < COOL_DOWN_TICS then
		local t = v.state_tics/COOL_DOWN_TICS
		foe_set_speed(self, t*v.speed)
	elseif v.state_tics > v.move_tics then
		start_thinking(self)
	end
end

local function update_thinking(self)
	if v.state_tics < COOL_DOWN_TICS then
		local t = 1 - v.state_tics/COOL_DOWN_TICS
		foe_set_speed(self, t*v.speed)
	elseif v.state_tics == 90 then
		local x, y = foe_get_direction(self)
		foe_set_direction(self, -x, .1*-y)
		start_moving(self)
	end
end

function init(self)
	local dx
	if rand(0, 1) > .5 then
		dx = 1
	else
		dx = -1
	end
	foe_set_direction(self, dx, 0)

	foe_set_speed(self, 0)

	start_moving(self)
end

function update(self)
	local x, y = foe_get_direction(self)

	foe_update_position(self)

	v.state_tics = v.state_tics + 1
	v.tics = v.tics + 1

	if v.state == STATE_MOVING then
		update_moving(self)
	elseif v.state == STATE_THINKING then
		update_thinking(self)
	end
end
