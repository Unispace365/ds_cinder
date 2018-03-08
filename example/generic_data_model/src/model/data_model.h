#pragma once
#ifndef DS_MODEL_DATA_MODEL
#define DS_MODEL_DATA_MODEL

#include <map>
#include <memory>
#include <vector>
#include <ds/data/resource.h>
#include <cinder/Color.h>
#include <cinder/Rect.h>
#include <cinder/Vector.h>


namespace ds {

namespace ui {
class SpriteEngine;
}

namespace model {

class DataProperty {
public:
	DataProperty();
	DataProperty(const std::string& name, const std::string& value);

	/// Get the name of this property
	const std::string&				getName() const;
	void							setName(const std::string& name);

	const std::string&				getValue() const;
	void							setValue(const std::string& value);
	void							setValue(const std::wstring& value);
	void							setValue(const int& value);
	void							setValue(const double& value);
	void							setValue(const float& value);
	void							setValue(const ci::Color& value);
	void							setValue(const ci::ColorA& value);
	void							setValue(const ci::vec2& value);
	void							setValue(const ci::vec3& value);
	void							setValue(const ci::Rectf& value);

	ds::Resource					getResource() const;
	void							setResource(const ds::Resource& resource);


	// ------- This value, type converted when called --------- //

	bool							getBool() const;
	int								getInt() const;
	float							getFloat() const;
	double							getDouble() const;

	/// The Engine is supplied to look up named colors
	const ci::Color					getColor(ds::ui::SpriteEngine&) const;
	const ci::ColorA				getColorA(ds::ui::SpriteEngine&) const;

	const std::string&				getString() const; // same as getValue(), but supplied here for convenience
	const std::wstring				getWString() const;

	const ci::vec2					getVec2() const;
	const ci::vec3					getVec3() const;
	const ci::Rectf					getRect() const;

protected:
	std::string						mName;
	std::string						mValue;
	ds::Resource					mResource;

};

/**
* \class ds::model::DataModelRef
*/
class DataModelRef {
public:

	// TODO: operators (equality and such)
	// TODO: get children / property by dot and array notation. For instance: getChild("something.items[5].title"); // DOT notation is done
	// TODO: duplicate
	// TODO: auto validation

	// TODO: Rework the children setup so it's a single vector of children instead of a map. We're effectively making each list of children it's own node and it's confusing

	DataModelRef();
	DataModelRef(const std::string& name, const int id = 0);

	/// Get the id for this item
	const int&						getId() const;
	void							setId(const int& id);

	/// Get the name of this item
	const std::string&				getName() const;
	void							setName(const std::string& name);

	/// If this item has no data, value, name, id, properties or children
	const bool						empty() const;


	// -------  end of this value ---------------------------- //

	/// Use this for looking stuff up only. Use the other functions to manage the list
	const std::map<std::string, DataProperty>&				getProperties();

	/// This can return an empty property, which is why it's const.
	/// If you want to modify a property, use the setProperty() function
	const DataProperty										getProperty(const std::string& propertyName);
	const std::string										getPropertyValue(const std::string& propertyName);

	/// Set the property with a given name
	void													setProperty(const std::string& propertyName, DataProperty datamodel);
	void													setProperty(const std::string& propertyName, const std::string& propertyValue);

	/// Gets all of the children of all names
	/// Manage the children using the other functions
	const std::map<std::string, std::vector<DataModelRef>>&	getChildrenMap();

	/// Gets all of the children with a name. 
	/// For instance, you could have a series of chapters, where each child is a DataModelRef, and the name of the children is "chapters". And another series of children named "images"
	/// Don't modify the children here, use the other functions
	const std::vector<DataModelRef>&						getChildren(const std::string& childrenName) const;

	/// Gets the first child with a name
	/// If no children exist for that name, creates a new child
	DataModelRef											getChild(const std::string& childName);

	/// Replaces any children with this name
	void													setChild(const std::string& childName, DataModelRef datamodel);
	/// Adds this child to the end of this children list
	void													addChild(const std::string& childName, DataModelRef datamodel);
	/// Is there at least one child with this name?
	bool													hasChild(const std::string& childName);
	/// Replaces any children with this name
	void													setChildren(const std::string& childrenName, std::vector<ds::model::DataModelRef> children);
	
	/// Logs this, it's properties, and all it's children recursively
	void					printTree(const bool verbose, const std::string& indent);

private:
	void					createData();
	class Data;
	std::shared_ptr<Data>	mData;
};

} // namespace model
} // namespace ds

#endif

#pragma once
