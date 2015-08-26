#include <cstring>

#include <tinyxml.h>

#include "rect.h"
#include "sprite.h"

sprite::sprite(const char *name, pixmap *pm)
: sprite_base(pm)
, name_(name)
{ }

void
sprite::serialize(TiXmlElement *el) const
{
	el->SetAttribute("name", name_);
}
