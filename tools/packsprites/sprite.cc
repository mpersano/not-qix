#include <cstring>

#include "sprite.h"

sprite::sprite(const char *name, pixmap *pm)
: sprite_base(pm)
, name_(name)
{ }

void
sprite::serialize(file_writer& fw) const
{
	fw.write_string(name_.c_str());
}
