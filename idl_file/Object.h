#pragma once

namespace g6
{
	interface Object
	{
		/* read& write attribute */
		bool			attr_bool;
		int8			attr_i8;
		int16		attr_i16;
		int32		attr_i32;
		int64		attr_i64;
		uint8		attr_ui8;
		uint16		attr_ui16;
		uint32		attr_ui32;
		uint64		attr_ui64;
		float		attr_f;
		double		attr_df;
		string		attr_str;
		span<int8>	attr_sp;
		vector<float>	attr_vec;
		/* read only attribute */
		[[get]] int32 attr_r;
		/* write only attribute */
		[[set]] int32 attr_w;


		/*******************************************
		 * function foo
		 * args:
		 *     iVal1 : Input parameters
		 *     iVal2 : Input parameters
		 *     oVal3 : Output parameters
		 *     ioVal4: Input and output parameters
		 *******************************************/
		bool foo(int8 iVal1, [[in]] int16 iVal2, [[out]] float oVal3, [[inout]] double ioVal4);
	};
}
