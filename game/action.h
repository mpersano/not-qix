#pragma once

#include <list>
#include <memory>

class abstract_action
{
public:
	virtual ~abstract_action() = default;

	virtual void step() = 0;
	virtual bool done() const = 0;

	virtual void set_properties() const = 0;
};

class timed_action : public abstract_action
{
public:
	timed_action(int tics)
	: tics_(tics), cur_tic_(0)
	{ }

	void step() override;
	bool done() const override;

protected:
	int tics_, cur_tic_;
};

class delay_action : public timed_action
{
public:
	delay_action(int tics)
	: timed_action(tics)
	{ }

	void set_properties() const override
	{ }
};

template <class Tween>
class property_change_action : public timed_action
{
public:
	using type = typename Tween::type;

	property_change_action(type& property, const type& from, const type& to, int tics)
	: timed_action(tics)
	, from_(from)
	, to_(to)
	, property_(property)
	{ }

	void set_properties() const
	{
		property_ = tweener_(from_, to_, static_cast<float>(cur_tic_)/tics_);
	}

private:
	type from_, to_;
	type& property_;
	Tween tweener_;
};

class action_group : public abstract_action
{
public:
	action_group()
	{ }

	action_group *add(abstract_action *action);

	bool done() const;
	void reset();

protected:
	std::list<std::unique_ptr<abstract_action>> children_;
};

class parallel_action_group : public action_group
{
public:
	void step();
	void set_properties() const override;
};

class sequential_action_group : public action_group
{
public:
	void step();
	void set_properties() const override;
};
