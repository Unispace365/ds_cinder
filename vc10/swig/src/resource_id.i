
%extend ds::Resource {
    static const std::string& getCmsDatabasePath() {
        return ds::Resource::Id().getDatabasePath();
    }
    static const std::string& getCmsResourcePath() {
        return ds::Resource::Id().getResourcePath();
    }

    bool queryCms( const int id ) {
        return $self->query( ds::Resource::Id( id ) );
    }

    static void setupPaths( 
            const std::string& resource, 
            const std::string& db,
            const std::string& projectPath )
    {
        ds::Resource::Id::setupPaths( resource, db, projectPath );
    }
    
    static ds::Resource::Id cmsId( const int id ) {
        return ds::Resource::Id( id );
    }

    static ds::Resource cmsResource( const int id ) {
        ds::Resource ret;
        ret.query( ds::Resource::Id( id ) );
        return  ret;
    }

    static ds::Resource::Id cmsId( const int id ) {
        return ds::Resource::Id( id );
    }
}

/*
struct Id {
      static const char   CMS_TYPE = 0;
      static const char   APP_TYPE = 1;
      static const char   CUSTOM_TYPE = -32;

      char                mType;
      int                 mValue;

      Id();
      Id(const int value);
      Id(const char type, const int value);

      bool                operator==(const Id&) const;
      bool                operator!=(const Id&) const;
      bool                operator>(const Id&) const;
      bool                operator<(const Id&) const;
      bool                operator>(const int value) const;
      bool                operator>=(const int value) const;
      bool                operator<(const int value) const;
      bool                operator<=(const int value) const;

      bool                empty() const;
      void                clear();

      bool                tryParse(const std::string&);
      const std::string&  getResourcePath() const;
      const std::string&  getDatabasePath() const;
      bool                verifyPaths() const;

      void                writeTo(DataBuffer&) const;
      bool                readFrom(DataBuffer&);

      static void setupPaths( const std::string& resource, const std::string& db, const std::string& projectPath);
      static void setupCustomPaths( const std::function<const std::string&(const Resource::Id&)>& resourcePath, const std::function<const std::string&(const Resource::Id&)>& dbPath);
};
*/

