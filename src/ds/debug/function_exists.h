#pragma once
#ifndef DS_DEBUG_FUNCTIONEXISTS_H_
#define DS_DEBUG_FUNCTIONEXISTS_H_

// This file contains a pattern that can be used to determine if a function
// exists at compile time. The check simply determines the function name,
// it can't distinguish between overloads. The convention is to name each
// check like "has_(function name)_fn".
// Clients make use of this functionality like so:
// ds::dbg::has_(function name)_fn<(class name)>::value
// i.e., if you want to check class Query for function run:
// ds::dbg::has_run_fn<Query>::value

namespace ds { namespace dbg {

	template <typename T>
	class has_finished_fn {
		typedef char one;
		typedef long two;
		template <typename C>
		static one test(decltype(&C::finished));
		template <typename C>
		static two test(...);

	  public:
		enum { value = std::is_same<decltype(test<T>(nullptr)), char>::value };
	};

}} // namespace ds::dbg

#endif // DS_DEBUG_FUNCTIONEXISTS_H_