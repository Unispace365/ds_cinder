#pragma once
#ifndef MODEL_NODE_H_
#define MODEL_NODE_H_

#include <memory>

namespace na {

/**
 * \class na::NodeRef
 */
class NodeRef {
public:
	NodeRef();
	NodeRef(const std::wstring& name);

	bool					empty() const;
	void					clear();

	const std::wstring&		getName() const;

private:
	NodeRef&				setName(const std::string&);

	class Data;
	std::shared_ptr<Data>	mData;
};

} // namespace na

#endif
