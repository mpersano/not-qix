#include <cassert>
#include <map>
#include <algorithm>

#include <tinyxml.h>

#include <ggl/core.h>
#include <ggl/asset.h>
#include <ggl/tween.h>
#include <ggl/action.h>

namespace ggl {

action::~action() = default;

//
// timed_action
//

timed_action::timed_action(int tics)
: tics_(tics), cur_tic_(0)
{ }

void
timed_action::update()
{
	if (cur_tic_ < tics_)
		++cur_tic_;
	set_properties();
}

bool
timed_action::done() const
{
	return cur_tic_ == tics_;
}

//
// delay_action
//

delay_action::delay_action(int tics)
: timed_action(tics)
{ }

void
delay_action::set_properties() const
{ }

void
delay_action::bind(const std::string&, float *)
{ }

action_ptr
delay_action::clone() const
{
	return action_ptr { new delay_action(*this) };
}

//
// property_change_action
//

property_change_action::property_change_action(const std::string& property, float from, float to, int tics, tween_fn tween)
: timed_action(tics)
, property_name_(property)
, property_(nullptr)
, from_(from)
, to_(to)
, tween_(tween)
{ }

property_change_action::property_change_action(float *property, float from, float to, int tics, tween_fn tween)
: timed_action(tics)
, property_(property)
, from_(from)
, to_(to)
, tween_(tween)
{ }

void
property_change_action::set_properties() const
{
	if (property_) {
		const float t = tween_(static_cast<float>(cur_tic_)/tics_);
		*property_ = from_ + t*(to_ - from_);
	}
}

void
property_change_action::bind(const std::string& name, float *value)
{
	if (name == property_name_)
		property_ = value;
}

action_ptr
property_change_action::clone() const
{
	return action_ptr { new property_change_action(*this) };
}

//
// action_group
//

action_group::action_group() = default;

void
action_group::add(action_ptr action)
{
	children_.push_back(std::move(action));
}

void
action_group::bind(const std::string& name, float *value)
{
	for (auto& child : children_)
		child->bind(name, value);
}

bool
action_group::done() const
{
	return children_.empty();
}

template <typename BaseType>
action_ptr
action_group::do_clone() const
{
	static_assert(std::is_base_of<action_group, BaseType>::value, "eh?");

	action_ptr rv { new BaseType };

	std::transform(
		std::begin(children_),
		std::end(children_),
		std::back_inserter(static_cast<action_group *>(rv.get())->children_),
		[](const action_ptr& p) { return p->clone(); });

	return rv;
}

// parallel_action_group

void
parallel_action_group::update()
{
	auto it = std::begin(children_);

	while (it != std::end(children_)) {
		auto& p = *it;

		assert(!p->done());
		p->update();

		if (p->done())
			it = children_.erase(it);
		else
			++it;
	}
}

void
parallel_action_group::set_properties() const
{
	for (auto& p : children_) {
		assert(!p->done());
		p->set_properties();
	}
}

action_ptr
parallel_action_group::clone() const
{
	return do_clone<parallel_action_group>();
}

// sequential_action_group

void
sequential_action_group::update()
{
	if (!children_.empty()) {
		auto& p = children_.front();

		assert(!p->done());
		p->update();

		if (p->done())
			children_.pop_front();
	}
}

void
sequential_action_group::set_properties() const
{
	if (!children_.empty()) {
		auto& p = children_.front();

		assert(!p->done());
		p->set_properties();
	}
}

action_ptr
sequential_action_group::clone() const
{
	return do_clone<sequential_action_group>();
}

namespace {

action_ptr parse_action(TiXmlNode *node);

void
parse_group_children(action_group& g, TiXmlElement *el)
{
	for (auto node = el->FirstChild(); node; node = node->NextSibling())
		g.add(parse_action(node));
}

const std::map<std::string, property_change_action::tween_fn> tweens
	{
		{ "linear", tween::linear },
		{ "in-quadratic", tween::in_quadratic },
		{ "out-quadratic", tween::out_quadratic },
		{ "in-out-quadratic", tween::in_out_quadratic },
	};

const std::map<std::string, std::function<action_ptr(TiXmlElement *el)>> action_parsers
	{
		{
			"property",
			[](TiXmlElement *el)
			{
				const std::string name = el->Attribute("name");
				const float from = atof(el->Attribute("from"));
				const float to = atof(el->Attribute("to"));
				const int tics = atoi(el->Attribute("tics"));

				property_change_action::tween_fn tween = tween::linear;

				if (const char *tween_name = el->Attribute("tween")) {
					auto it = tweens.find(tween_name);
					if (it != std::end(tweens))
						tween = it->second;
				}

				return action_ptr { new property_change_action { name, from, to, tics, tween } };
			}
		},
		{
			"delay",
			[](TiXmlElement *el)
			{
				return action_ptr { new delay_action { atoi(el->Attribute("tics")) } };
			}
		},
		{
			"sequence",
			[](TiXmlElement *el)
			{
				action_ptr rv { new sequential_action_group };
				parse_group_children(*static_cast<action_group *>(rv.get()), el);
				return rv;
			}
		},
		{
			"parallel",
			[](TiXmlElement *el)
			{
				action_ptr rv { new parallel_action_group };
				parse_group_children(*static_cast<action_group *>(rv.get()), el);
				return rv;
			}
		},
	};

action_ptr
parse_action(TiXmlNode *node)
{
	action_ptr rv;

	if (TiXmlElement *el = node->ToElement()) {
		const char *value = el->Value();

		auto it = action_parsers.find(value);

		if (it != std::end(action_parsers))
			rv = it->second(el);
	}

	return rv;
}

}

action_ptr
load_action(const std::string& path)
{
	auto asset = g_core->get_asset(path);

	std::vector<char> xml(asset->size());
	asset->read(&xml[0], asset->size());

	TiXmlDocument doc;
	doc.Parse(&xml[0]);

	return parse_action(doc.RootElement()->FirstChild());
}

}
