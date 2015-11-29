local SPEED = 1

local SIN_TABLE = { 0.0000, 0.1951, 0.3827, 0.5556, 0.7071, 0.8315, 0.9239, 0.9808, 1.0000, 0.9808, 0.9239, 0.8315, 0.7071, 0.5556, 0.3827, 0.1951, 0.0000, -0.1951, -0.3827, -0.5556, -0.7071, -0.8315, -0.9239, -0.9808, -1.0000, -0.9808, -0.9239, -0.8315, -0.7071, -0.5556, -0.3827, -0.1951 }

v.tics = 0

function init(self)
	foe_set_direction(self, 1, 0)
	foe_set_speed(self, SPEED)
end

function update(self)
	foe_update_position(self)

	v.tics = v.tics + 1

	foe_rotate(self, .1*SIN_TABLE[1 + (v.tics % 32)])
end
