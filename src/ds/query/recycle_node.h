#pragma once
#ifndef _COLLECTIONS_RECYCLENODE_H
#define _COLLECTIONS_RECYCLENODE_H

#pragma once

namespace ds { namespace prvlist {

	/* NODE
	 **************************************************************/
	struct _node_base {
		_node_base* mNext;

		_node_base* tail() {
			_node_base* n = this;
			while (n->mNext != NULL)
				n = n->mNext;
			return n;
		}
	};

	template <class T>
	struct _node : public _node_base {
		T mData;
	};

}} // namespace ds::prvlist
#endif
