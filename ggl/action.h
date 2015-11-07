#pragma once

#include <algorithm>
#include <string>
#include <list>
#include <memory>
#include <type_traits>

namespace ggl {

class action;
using action_ptr = std::unique_ptr<action>;

class action
{
public:
	virtual ~action();

	virtual void update() = 0;
	virtual bool done() const = 0;
	virtual void set_properties() const = 0;
	virtual void bind(const std::string& name, float *value) = 0;
	virtual action_ptr clone() const = 0;
};

class timed_action : public action
{
public:
	timed_action(int tics);

	void update() override;
	bool done() const override;

protected:
	int tics_, cur_tic_;
};

class delay_action : public timed_action
{
public:
	delay_action(int tics);

	void set_properties() const override;
	void bind(const std::string& name, float *value) override;
	action_ptr clone() const override;
};

class property_change_action : public timed_action
{
public:
	using tween_fn = std::function<float(float)>;

	property_change_action(const std::string& property, float from, float to, int tics, tween_fn tween);
	property_change_action(float *property, float from, float to, int tics, tween_fn tween);

	void set_properties() const override;
	void bind(const std::string& name, float *value) override;
	action_ptr clone() const override;

private:
	std::string property_name_;
	float *property_;
	float from_, to_;
	tween_fn tween_;
};

class action_group : public action
{
public:
	action_group();

	void add(action_ptr action);
	void bind(const std::string& name, float *value) override;
	bool done() const override;

protected:
	template <typename BaseType>
	action_ptr do_clone() const
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

	std::list<action_ptr> children_;
};

class parallel_action_group : public action_group
{
public:
	void update() override;
	void set_properties() const override;
	action_ptr clone() const override;
};

class sequential_action_group : public action_group
{
public:
	void update() override;
	void set_properties() const override;
	action_ptr clone() const override;
};

action_ptr
load_action(const std::string& path);

}
