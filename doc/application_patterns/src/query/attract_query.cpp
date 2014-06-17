#include "na/query/attract_query.h"

#include <ofxQuery/QueryClient.h>

namespace na {

namespace {
const std::string		ID_FIELD("row_id");
const std::string		QUOTE_FIELD("quote");
const std::string		SOURCE_FIELD("source");

na::StoryType       virtue_type_from_db(const std::string& s)
{
  if (s == "s") return na::SolutionsType;
  if (s == "p") return na::PeopleType;
  return na::UnknownStoryType;
}

}

/**
 * \class na::AttractQuery
 */
AttractQuery::AttractQuery()
{
  std::stringstream   buf;
  buf << "SELECT * FROM attract";
  mInput = buf.str();

  mRnd.seed();
}

static std::string make_str(std::stringstream& buf, const std::string& txt, const int idx)
{
  buf.str("");
  buf << txt << idx;
  return buf.str();
}

bool AttractQuery::generate(std::vector<Attract>& out)
{
  out.clear();
  try {
    query(out);
    return true;
  } catch (std::exception const&) {
  }
  return false;
}

bool AttractQuery::query(std::vector<Attract>& out)
{
  ds::resource_id     cms(ds::resource_id::CMS_TYPE, 0);
  if (!QueryClient::query(cms.getDatabasePath(), mInput, mResult, QueryClient::INCLUDE_COLUMN_NAMES_F)) return false;

  // The query is always the same, the fields should always be the same
  if (mFields.empty()) buildFields(mResult);

  QueryResult::RowIterator		it(mResult);
	const int						        fieldCount = mFields.size();
  if (fieldCount < 1) return false;

	while (it.hasValue()) {
		out.push_back(Attract());
		Attract&						      p(out.back());

    // Get the fields
    int							          k = 0;
  	for (auto fit = mFields.begin(), fend=mFields.end(); fit != fend; ++fit) (*fit)(p, it, k++);

    ++it;
  }

  return true;
}

void AttractQuery::buildFields(const QueryResult& r)
{
	mFields.clear();
	for (int k=0; k<r.getColumnSize(); ++k) {
		const std::string&	n = r.getColumnName(k);

		if (n == ID_FIELD) mFields.push_back([](Attract& p, const QueryResult::RowIterator& it, const int k) { p.mDbId = it.getInt(k); });
		else if (n == QUOTE_FIELD) mFields.push_back([](Attract& p, const QueryResult::RowIterator& it, const int k) { p.mQuote = it.getString(k); });
		else if (n == SOURCE_FIELD) mFields.push_back([](Attract& p, const QueryResult::RowIterator& it, const int k) { p.mSource = it.getString(k); });
		else {
			mFields.push_back([](Attract& p, const QueryResult::RowIterator& it, const int k) { ; });
#ifdef _DEBUG
			// If it's not any of the unsupported fields then throw a warning
//			if (n != CREATED_AT_FIELD && n != UPDATED_AT_FIELD) {
				cout << "ERROR AttractQuery::buildFields() on unhandled field (" << n << ")" << endl;
				throw std::exception("AttractQuery::buildFields() on unhandled field");
//			}
#endif
		}
	}
}

} // namespace na
