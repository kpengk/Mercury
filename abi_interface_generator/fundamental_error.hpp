#pragma once

#include <stdexcept>

namespace glasssix
{
	struct fundamental_error : std::logic_error
	{
		using logic_error::logic_error;
	};
}
