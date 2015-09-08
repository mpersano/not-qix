#include "action.h"

void
timed_action::step()
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

action_group *
action_group::add(abstract_action *action)
{
	children_.push_back(std::unique_ptr<abstract_action>(action));
	return this;
}

bool
action_group::done() const
{
	for (auto& child : children_) {
		if (!child->done())
			return false;
	}

	return true;
}

void
parallel_action_group::step()
{
	for (auto& p : children_) {
		if (!p->done())
			p->step();
	}
}

void
parallel_action_group::set_properties() const
{
	for (auto& p : children_) {
		if (!p->done())
			p->set_properties();
	}
}

void
sequential_action_group::step()
{
	for (auto& p : children_) {
		if (!p->done()) {
			p->step();
			break;
		}
	}
}

void
sequential_action_group::set_properties() const
{
	for (auto& p : children_) {
		if (!p->done()) {
			p->set_properties();
			break;
		}
	}
}
