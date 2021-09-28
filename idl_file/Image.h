#pragma once

#include "PixPoint.h"

namespace g6
{
	interface Image
	{
		[[get]] vector<vector<PixPoint>> data;

		int32 width() const;
		int32 height() const;
		void size([[out]] int32 w, [[out]] int32 h) const;
		vector<PixPoint> row(int32 n);
		PixPoint pix(int32 row, int32 col);
	};
}