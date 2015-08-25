#include <cstring>

#include "rect.h"
#include "sprite.h"

sprite::sprite(const char *name, pixmap *pm)
: sprite_base(pm)
, name_(name)
{ }

void
sprite::serialize(FILE *out, const rect& rc, int border) const
{
	fprintf(out, "    <sprite x=\"%d\" y=\"%d\" w=\"%d\" h=\"%d\" name=\"%s\" />\n",
		rc.left_ + border, rc.top_ + border,
		width(), height(),
		name_.c_str());
}
