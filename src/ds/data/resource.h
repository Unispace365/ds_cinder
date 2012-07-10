#pragma once
#ifndef DS_DATA_RESOURCE_H_
#define DS_DATA_RESOURCE_H_

#include <string>

namespace ds {

/**
 * \class ds::Resource
 * \brief Encapsulate a row in the Resources table.
 */
class Resource
{
  public:
    /**
     * \struct ds::Resource::id
     * \brief A single resource ID, composed of a type and value.
     * Resources can exist in one of several databases, hence the type.
     */
    struct Id {
      // The traditional type for remotely-generated user content,
      // but also the only type supported until this structure was added.
      //  All legacy projects will have this type and only this type.
      static const char   CMS_TYPE = 0;
      // Local application resources like UI components.
      static const char   APP_TYPE = 1;

      // All framework-defined types will be positive.  Negative values
      // are reserved for applications to create fake resource_ids.
      char                mType;
      int                 mValue;

      Id();
      Id(const int value);
      Id(const char type, const int value);

      bool                operator==(const Id&) const;
      bool                operator!=(const Id&) const;
      bool                operator>(const Id&) const;
      bool                operator<(const Id&) const;
      // Comparison with a raw value
      bool                operator>(const int value) const;
      bool                operator>=(const int value) const;
      bool                operator<(const int value) const;
      bool                operator<=(const int value) const;

      bool                empty() const;
      void                clear();

      // Assumes the string is in my string output format (type:value)
      bool                tryParse(const std::string&);

      // Get the path to the resources for this type, or directly to the
      // database (which will always be in the resource path).
      const std::string&  getResourcePath() const;
      const std::string&  getDatabasePath() const;

      // Utility to report and log an error if I'm missing path information
      bool                verifyPaths() const;

//		void						writeTo(UDPPacketSender&) const;
//		void						readFrom(UDPPacketReceiver&);

      // The engines are required to set paths to the various resource database before
      // anyone does anything.  This assumes the traditional CMS path -- a resource
      // location and a database file inside it.  From this info, the legacy and CMS
      // (which are currently the same thing) paths are set, and the files are probed
      // to set any additional paths.
      // root and have the newer db structure figured automatically.
      // OK!  resource and db are now considered legacy values.  Project path will
      // have a segment of the path to the resources -- it won't have the start,
      // which is standardized to My Documents\downstream\resources, and it won't have
      // the end, which is now a common structure.  Currently this only applies to
      // app-local resources, the traditional CMS resources have the traditional CMS setup.
      static void					setupPaths(	const std::string& resource, const std::string& db,
                                      const std::string& projectPath);
    };

  public:
    static const int      ERROR_TYPE          = 0;
    static const int      FONT_TYPE           = 1;
    static const int      IMAGE_TYPE          = 2;
    static const int      IMAGE_SEQUENCE_TYPE = 3;
    static const int      PDF_TYPE            = 4;
    static const int      VIDEO_TYPE          = 5;
    static const int      WEB_TYPE            = 6;

  public:
    Resource();
    Resource(const Resource::Id& dbId, const int type);

    const Resource::Id&   getDbId() const			{ return mDbId; }
    int                   getType() const			{ return mType; }
    const std::wstring&   getTypeName() const;

    void                  setDbId(const Resource::Id&);
    void                  setType(const int);

    // Warning: Expensive operation (database lookup).  Use with care.
    bool                  existsInDb() const;

  private:
    friend class MediaItem;
    friend class MediaList;
    friend class MediaQuery;

    Resource::Id          mDbId;
    int                   mType;

    void                  setTypeFromString(const std::string& typeChar);
};

} // namespace ds

std::ostream&             operator<<(std::ostream&, const ds::Resource::Id&);
std::wostream&            operator<<(std::wostream&, const ds::Resource::Id&);

#endif // DS_DATA_RESOURCE_H_