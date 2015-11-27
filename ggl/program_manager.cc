#include <tinyxml.h>

#include <ggl/panic.h>
#include <ggl/core.h>
#include <ggl/asset.h>
#include <ggl/gl_program.h>
#include <ggl/program_manager.h>

namespace ggl { namespace res {

const gl_program *
program_manager::get(const std::string& name) const
{
	auto it = program_map_.find(name);

	if (it == program_map_.end())
		panic("failed to locate program `%s'", name.c_str());

	return it->second.get();
}

namespace {

std::pair<std::string, std::unique_ptr<gl_program>>
parse_program(TiXmlNode *node)
{
	std::string name;
	std::unique_ptr<gl_program> prog;

	if (TiXmlElement *program_el = node->ToElement()) {
		std::string vertex_shader_path;
		std::string frag_shader_path;

		name = program_el->Attribute("name");

		// TODO error checking

#if 0
		auto read_shader = [&](const std::string& path)
			{
				auto data = g_core->get_asset(path)->read_all();

				std::string source;
				source.assign(std::begin(data), std::end(data));

				return source;
			};
#endif

		std::string vp_path, fp_path;

		for (auto node = program_el->FirstChild(); node; node = node->NextSibling()) {
			if (auto el = node->ToElement()) {
				auto value = el->Value();

				if (!strcmp(value, "vert")) {
					vp_path = el->Attribute("source");
				} else if (!strcmp(value, "frag")) {
					fp_path = el->Attribute("source");
				}
			}
		}

		prog.reset(new gl_program { vp_path, fp_path });
	}

	return std::make_pair(name, std::move(prog));
}

} // (anonymous namespace)

void
program_manager::load_programs(const std::string& path)
{
	auto xml = g_core->get_asset(path)->read_all();

	TiXmlDocument doc;
	doc.Parse(&xml[0]);

	auto root_el = doc.RootElement();

	for (auto node = root_el->FirstChild(); node; node = node->NextSibling())
		program_map_.insert(parse_program(node));
}

} }