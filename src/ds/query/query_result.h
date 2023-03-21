#pragma once
#ifndef DS_THREAD_QUERYRESULT_H_
#define DS_THREAD_QUERYRESULT_H_

#include <Poco/DateTime.h>
#include <Poco/Timestamp.h>
#include <functional>
#include <memory>
#include <stdint.h>
#include <string>
#include <vector>

namespace ds { namespace query {

	extern const int QUERY_NO_TYPE;
	extern const int QUERY_NUMERIC;
	extern const int QUERY_STRING;
	// A special type that is used as a placeholder until I know the real type.
	extern const int QUERY_NULL;

	/**
	 * \class Result
	 * \brief A datastore for query results.
	 */
	class Result {
	  private:
		class Row;

	  public:
		class RowIterator {
		  public:
			RowIterator(const RowIterator&);
			RowIterator(const Result&);
			/// This variant seeks to the first row with the supplied name.
			RowIterator(const Result&, const std::string&);
			/// Find by row index
			RowIterator(const Result&, const size_t index);

			void operator++();
			void operator+=(const int count);
			bool hasValue() const;

			const std::string& getName() const;
			/// There's no error checking here -- if the reply doesn't have an
			/// error, and you've checked matches(), and you ask for the right
			/// column, you should be fine.
			int					 getInt(const int columnIndex) const;
			int64_t				 getInt64(const int columnIndex) const;
			float				 getFloat(const int columnIndex) const;
			const std::string&	 getString(const int columnIndex) const;
			const std::wstring&	 getWString(const int columnIndex) const;
			const Poco::DateTime getDateTime(const int columnIndex) const;
			const Poco::DateTime getDateTime24hr(const int columnIndex) const;

		  private:
			friend class ds::query::Result;
			RowIterator();
			RowIterator(const Result&, const std::unique_ptr<Row>&);
			void		 operator++(int);
			RowIterator& operator=(const RowIterator&);

			const Result&									  mResult;
			std::vector<std::unique_ptr<Row>>::const_iterator mRowIt;
			/// A utility -- directly override my current item. Can't increment
			Row* mOverride;
		};

	  public:
		Result();
		explicit Result(const Result&);

		Result& operator=(const Result&);
		/// Replace my contents with a single row from the row iterator
		Result& operator=(const RowIterator&);

		void clear();

		/// In certain situations clients can do an assert test to make
		/// sure the structure matches what they're expecting, i.e.:
		/// _ASSERT(queryResult.matches(&QUERY_NUMERIC, &QUERY_STRING, NULL));
		/// Note that this doesn't work for results from SQLite, as columns
		/// that are sometimes ints can be promoted to strings on NULL cases.
		/// In that case, the best you can do is match the number of columns.
		bool matches(const int* type0, ...) const;

		Poco::Timestamp getRequestTime() const;
		int				getClientId() const;

		int				   getColumnSize() const;
		const int		   getColumnType(const int idx) const;
		const std::string& getColumnName(const int idx) const;

		bool		rowsAreEmpty() const;
		int			getRowSize() const;
		RowIterator getRows() const;
		RowIterator rowAt(const size_t index) const;
		/// Answer a RowIterator for the first row that has the given int field
		/// with the given value.
		/// Add all of source rows into me
		bool addRows(const Result& src);
		/// Remove my first row, optionally placing it
		void popRowFront();

		/// Turn this off for now, not sure if anyone's using it
		//	void					moveRow(Result&, const int from, const int to);
		/// Swap all data
		void swap(Result&);
		/// Sort by specific columns
		void sortByString(const int columnIndex,
						  const std::function<bool(const std::string& a, const std::string& b)>&);
		void sort_if(const std::function<bool(const RowIterator& a, const RowIterator& b)>&);

	  private:
		/// Convenience to add a new row at the end, throwing if I fail
		Row* pushBackRow();

		friend class ResultBuilder;
		friend class ResultEditor;
		friend class ResultRandomizer;
		class Row {
		  public:
			/// Rows have an optional name.  This isn't used when returning results from
			/// a query, but it is used when we are using the QueryResult as a general data
			/// storage mechanism locally in apps.
			std::string				  mName;
			std::vector<double>		  mNumeric;
			std::vector<std::string>  mString;
			std::vector<std::wstring> mWString;

			Row();
			void clear();
			void initialize(const int columns);
			Row& operator=(const Row&);
		};

		/// column types
		std::vector<int>				  mCol;
		std::vector<std::string>		  mColNames;
		std::vector<std::unique_ptr<Row>> mRow;

		/// The time this query was requested.
		Poco::Timestamp mRequestTime;
		int				mClientId;

#ifdef _DEBUG
	  public:
		void print() const;
#endif
	};

}} // namespace ds::query

#endif // DS_THREAD_QUERYRESULT_H_
