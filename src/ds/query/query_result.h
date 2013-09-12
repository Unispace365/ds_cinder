#pragma once
#ifndef DS_THREAD_QUERYRESULT_H_
#define DS_THREAD_QUERYRESULT_H_

#include <string>
#include <vector>
#include "Poco/Timestamp.h"
// I'd really like to weed these classes out, but it's not worth it for now.
#include "ManagedList.h"
#include "RecycleArray.h"

namespace ds {

namespace query {

extern const int		QUERY_NO_TYPE;
extern const int		QUERY_NUMERIC;
extern const int		QUERY_STRING;
// A special type that is used as a placeholder until I know the real type.
extern const int		QUERY_NULL;

/**
 * \class ds::query::Result
 * \brief A datastore for query results.
 */
class Result
{
private:
	class Row;

public:
	class RowIterator {
	public:
		RowIterator(const RowIterator&);
		RowIterator(const Result&);
		// This variant seeks to the first row with the supplied name.
		RowIterator(const Result&, const std::string&);

		RowIterator&					operator=(const RowIterator&);
		void							operator++();
		void							operator+=(const int count);
		bool							hasValue() const;

		const std::string&				getName() const;
		// There's no error checking here -- if the reply doesn't have an
		// error, and you've checked matches(), and you ask for the right
		// column, you should be fine.
		int								getInt(const int columnIndex) const;
		float							getFloat(const int columnIndex) const;
		const std::string&				getString(const int columnIndex) const;
		const std::wstring&				getWString(const int columnIndex) const;

	private:
		friend class ds::query::Result;
		RowIterator();
		void							operator++(int);

		const Result*					mResult;
		ManagedListIterator<Row*>		mRowIt;
	};

public:
	Result();
	explicit Result(const Result&);

	Result&					operator=(const Result&);
	// Replace my contents with a single row from the row iterator
	Result&					operator=(const RowIterator&);

	void					clear();

	// In certain situations clients can do an assert test to make
	// sure the structure matches what they're expecting, i.e.:
	// _ASSERT(queryResult.matches(&QUERY_NUMERIC, &QUERY_STRING, NULL));
	// Note that this doesn't work for results from SQLite, as columns
	// that are sometimes ints can be promoted to strings on NULL cases.
	// In that case, the best you can do is match the number of columns.
	bool					matches(const int* type0, ...) const;

	Poco::Timestamp			getRequestTime() const;
	int						getClientId() const;

	int						getColumnSize() const;
	const std::string&		getColumnName(const int idx) const;

	bool					rowsAreEmpty() const;
	int						getRowSize() const;
	RowIterator				getRows() const;
	// Add all of source rows into me
	bool					addRows(const Result& src);
	// Remove my first row, optionally placing it 
	void					popRowFront();
	// Access to ManagedList::move(), see it for args
	void					moveRow(Result&, const int from, const int to);
	// Swap all data
	void					swap(Result&);

	// Collections for memory management.  Strings can't be constructed
	// with realloc(), hence the need for the list.
private:
	typedef RecycleArray<double>				NumericArray;
	typedef ManagedList<std::string*>			StringList;
	typedef ManagedListIterator<std::string*>	StringIterator;
	typedef ManagedList<Row*>					RowList;

private:
	friend class ResultBuilder;
	friend class ResultEditor;
	friend class ResultRandomizer;
	class Row {
	public:
		// Rows have an optional name.  This isn't used when returning results from
		// a query, but it is used when we are using the QueryResult as a general data
		// storage mechanism locally in apps.
		std::string					mName;
		NumericArray				mNumeric;
		StringList					mString;
		std::vector<std::wstring>	mWString;

		Row();
		void						clear();
		void						initialize(const int columns);
		Row&						operator=(const Row&);
	};

	// column types
	RecycleArray<int>				mCol;
	std::vector<std::string>		mColNames;
	RowList							mRow;
	// The time this query was requested.
	Poco::Timestamp					mRequestTime;
	int								mClientId;

public:
	class StringFactory;
	class RowFactory;

#ifdef _DEBUG
public:
	void							print() const;
#endif
};

} // namespace query

} // namespace ds

#endif // DS_THREAD_QUERYRESULT_H_