#pragma once
#ifndef DS_ARC_ARCIO_H_
#define DS_ARC_ARCIO_H_

#include <memory>
#include "ds/arc/arc.h"

namespace ds {
namespace arc {

/**
 * \brief Load an arc from the file name.
 */
std::unique_ptr<Arc>		load(const std::string& filename);

/**
 * \brief Create a new arc from the classname
 */
std::unique_ptr<Arc>		create(const std::string& classname);
std::unique_ptr<Arc>		create(const ci::XmlTree&);

} // namespace arc
} // namespace ds

#endif // DS_ARC_ARCIO_H_