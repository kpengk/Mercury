#pragma once

namespace g6
{
	interface Widget
	{
		[[get]] string	id;		// get_id
		Widget*			parent;	// set_parent  get_parent
		vector<Widget*>	childs;	// set_childs  get_childs
		[[set]] int8		flag;	// set_flag
		int32 x;					// set_x       get_x
		int32 y;					// set_y       get_y
		int32 width;				// set_width   get_width
		int32 height;				// set_height  get_height


		bool isVisible() const;
		string windowTitle() const;
		int32 childCount() const;
		bool move([[inout]]int x, [[inout]]int y);// input the offset distance and output the position after moving
	};
}
