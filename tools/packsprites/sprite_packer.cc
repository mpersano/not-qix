#include <cassert>
#include <cstring>

#include <algorithm>

#include "panic.h"
#include "sprite.h"
#include "sprite_packer.h"

struct rect
{
	rect(int left, int top, int width, int height)
	: left_(left), top_(top), width_(width), height_(height)
	{ }

	int left_, top_, width_, height_;

	std::pair<rect, rect> split_vert(int c) const
	{
		assert(c < width_);
		return std::pair<rect, rect>(rect(left_, top_, c, height_), rect(left_ + c, top_, width_ - c, height_));
	}

	std::pair<rect, rect> split_horiz(int r)
	{
		assert(r < height_);
		return std::pair<rect, rect>(rect(left_, top_, width_, r), rect(left_, top_ + r, width_, height_ - r));
	}
};

struct node {
	node(const rect& rc)
	: rc_(rc), border_(0), left_(0), right_(0), sprite_(0)
	{ }

	bool insert_sprite(sprite_base *sp, int border);

	template <typename F>
	void for_each_sprite(F f)
	{
		if (left_) {
			left_->for_each_sprite(f);
			assert(right_);
			right_->for_each_sprite(f);
		} else if (sprite_) {
			f(rc_, border_, sprite_);
		}
	}

	rect rc_;
	int border_;
	node *left_, *right_;
	sprite_base *sprite_;
};

bool
node::insert_sprite(sprite_base *sp, int border)
{
	if (left_ != NULL) {
		// not a leaf
		return left_->insert_sprite(sp, border) || right_->insert_sprite(sp, border);
	} else {
		const int wanted_width = sp->width() + 2*border;
		const int wanted_height = sp->height() + 2*border;

		// doesn't fit or already occupied
		if (sprite_ || rc_.width_ < wanted_width || rc_.height_ < wanted_height) {
			return false;
		}

		if (rc_.width_ == wanted_width && rc_.height_ == wanted_height) {
			sprite_ = sp;
			border_ = border;
			return true;
		}

		if (rc_.width_ - wanted_width > rc_.height_ - wanted_height) {
			std::pair<rect, rect> child_rect = rc_.split_vert(wanted_width);
			left_ = new node(child_rect.first);
			right_ = new node(child_rect.second);
		} else {
			std::pair<rect, rect> child_rect = rc_.split_horiz(wanted_height);
			left_ = new node(child_rect.first);
			right_ = new node(child_rect.second);
		}

		bool rv = left_->insert_sprite(sp, border);
		assert(rv);
		return rv;
	}
}

static bool
sprite_cmp(sprite_base *a, sprite_base *b)
{
	return b->width()*b->height() < a->width()*a->height();
}

void
pack_sprites(std::vector<sprite_base *>& sprites, pixmap::type color_type, int border, const char *sheet_name, size_t width, size_t height)
{
}

class sprite_packer_impl : public sprite_packer
{
public:
	void write_sprite_sheet(pixmap& pm, const node *root);
	void write_sprite_sheet(const char *name, pixmap::type color_type, const node *root);
};

class single_sheet_packer : public sprite_packer_impl
{
public:
	void pack(std::vector<sprite_base *>& sprites, const char *sheet_name, pixmap::type color_type);
};

class multi_sheet_packer : public sprite_packer_impl
{
public:
	void pack(std::vector<sprite_base *>& sprites, const char *sheet_name, pixmap::type color_type);
};

void
sprite_packer_impl::write_sprite_sheet(pixmap& pm, const node *root)
{
	if (root->left_) {
		write_sprite_sheet(pm, root->left_);
		write_sprite_sheet(pm, root->right_);
	} else if (root->sprite_) {
		const pixmap *child_pm = root->sprite_->pm_;

		assert(child_pm->get_pixel_size() == pm.get_pixel_size());

		const size_t pixel_size = pm.get_pixel_size();

		const size_t dest_stride = pm.get_width()*pixel_size;
		const size_t src_stride = child_pm->get_width()*pixel_size;

		uint8_t *dest_bits =
			pm.get_bits() +
			((root->rc_.top_ + root->border_)*pm.get_width() + root->rc_.left_ + root->border_)*pixel_size;
		const uint8_t *src_bits = child_pm->get_bits();

		for (size_t i = 0; i < child_pm->get_height(); i++) {
			::memcpy(dest_bits, src_bits, src_stride);
			dest_bits += dest_stride;
			src_bits += src_stride;
		}
	}
}

void
sprite_packer_impl::write_sprite_sheet(const char *name, pixmap::type color_type, const node *root)
{
	assert(root->rc_.top_ == 0 && root->rc_.left_ == 0);
	
	pixmap pm(root->rc_.width_, root->rc_.height_, color_type);
	write_sprite_sheet(pm, root);
	pm.save(name);
}

void
single_sheet_packer::pack(std::vector<sprite_base *>& sprites, const char *sheet_name, pixmap::type color_type)
{
	const size_t num_sprites = sprites.size();

	std::sort(sprites.begin(), sprites.end(), sprite_cmp);

	node *root = new node(rect(0, 0, sheet_width_, sheet_height_));
	
	for (auto it = sprites.begin(); it != sprites.end(); it++) {
		sprite_base *sp = *it;

		assert(sp->width() <= sheet_width_ && sp->height() <= sheet_height_);

		if (!root->insert_sprite(sp, border_))
			panic("sprite sheet too small!\n");
	}

	// write sprite sheets

	{
	char name[80];
	sprintf(name, "%s.spr", sheet_name);

	file_writer out(name);

	out.write_uint16(num_sprites);

	root->for_each_sprite(
		[&] (const rect& rc, int border, const sprite_base *sp) {
			sp->serialize(out);
	
			out.write_uint16(rc.left_ + border);
			out.write_uint16(rc.top_ + border);
			out.write_uint16(sp->width());
			out.write_uint16(sp->height());
		});
	}

	{
	char name[80];
	sprintf(name, "%s.png", sheet_name);

	printf("writing %s\n", name);
	write_sprite_sheet(name, color_type, root);
	}
}

void
multi_sheet_packer::pack(std::vector<sprite_base *>& sprites, const char *sheet_name, pixmap::type color_type)
{
	const size_t num_sprites = sprites.size();

	std::vector<node *> trees;

	while (sprites.size()) {
		std::sort(sprites.begin(), sprites.end(), sprite_cmp);

		node *root = new node(rect(0, 0, sheet_width_, sheet_height_));

		std::vector<sprite_base *>::iterator it = sprites.begin();
		
		while (it != sprites.end()) {
			sprite_base *sp = *it;

			assert(sp->width() <= sheet_width_ && sp->height() <= sheet_height_);

			if (root->insert_sprite(sp, border_))
				it = sprites.erase(it);
			else
				++it;
		}

		trees.push_back(root);
	}

	// write sprite sheets

	char name[80];
	sprintf(name, "%s.spr", sheet_name);

	file_writer out(name);

	out.write_uint16(num_sprites);
	out.write_uint8(trees.size());

	for (size_t i = 0; i < trees.size(); i++) {
		char name[80];

		sprintf(name, "%s.%03lu.png", sheet_name, i);
		printf("writing %s\n", name);
		write_sprite_sheet(name, color_type, trees[i]);

		trees[i]->for_each_sprite(
			[&] (const rect& rc, int border, const sprite_base *sp) {
				sp->serialize(out);
		
				out.write_uint8(i);
				out.write_uint16(rc.left_ + border);
				out.write_uint16(rc.top_ + border);
				out.write_uint16(sp->width());
				out.write_uint16(sp->height());
			});
	}
}

sprite_packer::sprite_packer()
: sheet_width_(256), sheet_height_(256)
, border_(0)
{ }

sprite_packer::~sprite_packer()
{ }

void
sprite_packer::set_border(size_t border)
{
	border_ = border;
}

void
sprite_packer::set_sheet_size(size_t width, size_t height)
{
	sheet_width_ = width;
	sheet_height_ = height;
}

sprite_packer *
sprite_packer::make(bool multi)
{
	if (multi)
		return new multi_sheet_packer;
	else
		return new single_sheet_packer;
}