#include <cstring>

#include "sprite.h"

sprite::sprite(const char *name, const margins& margins, pixmap *pm)
: sprite_base(pm)
, name_(name)
, margins_(margins)
{ }

void
sprite::serialize(file_writer& fw) const
{
	fw.write_string(name_.c_str());
	fw.write_uint16(margins_.left_);
	fw.write_uint16(margins_.right_);
	fw.write_uint16(margins_.top_);
	fw.write_uint16(margins_.bottom_);
}
