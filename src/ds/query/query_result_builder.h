#pragma once
#ifndef DS_QUERY_QUERYRESULTBUILDER_H_
#define DS_QUERY_QUERYRESULTBUILDER_H_

#include "ds/query/query_result.h"
#include <string>

namespace ds { namespace query {

	/**
	 * \class ResultBuilder
	 * \brief Internal class to handle translating abstract results to
	 * local results.  Kind of weirdly designed -- it automatically
	 * builds from the constructor.
	 */
	class ResultBuilder {
	  public:
		/*
		 * Get access to setting the QueryResult request time
		 * \param result The QueryResult to set the time
		 * \param timestamp The poco timestamp
		 */
		static void setRequestTime(Result& result, const Poco::Timestamp& timestamp);
		static void setClientId(Result& result, const int id);

	  public:
		ResultBuilder(Result&);
		virtual ~ResultBuilder();

		bool isValid() const;

		/// Kinda weird, but all you do with this class is construct and build.
		/// Set columnNames to true if you want them in the result.
		void build(const bool columnNames = false);

		virtual int			getColumnCount() const				 = 0;
		virtual int			getColumnType(const int index) const = 0;
		virtual std::string getColumnName(const int index)		 = 0;
		/// Advance the row
		virtual bool hasNext() const						   = 0;
		virtual bool next()									   = 0;
		virtual bool getDouble(const int column, double&)	   = 0;
		virtual bool getString(const int column, std::string&) = 0;

	  protected:
		ResultBuilder& startRow();

		ResultBuilder& addNumeric(const double);
		ResultBuilder& addString(const std::string&);

	  private:
		Result&		 mResult;
		Result::Row* mRow;
		int			 mColIdx;

	  protected:
		bool mError;
	};

}} // namespace ds::query

#endif // DS_QUERY_QUERYRESULTBUILDER_H_
