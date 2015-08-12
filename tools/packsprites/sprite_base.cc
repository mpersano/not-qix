#include <cassert>
#include <cstring>

#include "sprite_base.h"

sprite_base::sprite_base(pixmap *pm)
: pm_(pm)
{ }

sprite_base::~sprite_base()
{
	delete pm_;
}
