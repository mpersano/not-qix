#include <ggl/asset.h>

namespace ggl {

std::vector<char>
asset::read_all()
{
	auto s = size();

	std::vector<char> data(s);
	read(&data[0], s);

	return data;
}

}
