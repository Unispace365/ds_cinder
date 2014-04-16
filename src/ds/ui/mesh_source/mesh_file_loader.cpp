#include "mesh_file_loader.h"

namespace ds {
namespace ui {
namespace util {

MeshFileLoader::MeshFileLoader()
{
    mVert = nullptr;
    mNorm = nullptr;
    mIndices = nullptr;
    mTex = nullptr;
    mNumVert = 0;
    mNumNorm = 0;
    mNumIndices = 0;
    mNumTex = 0;
}

MeshFileLoader::~MeshFileLoader()
{
    clear();
}

void MeshFileLoader::ChunkRead( std::fstream &filestr, char *dest, unsigned size, unsigned chunksize )
{
    if (!size)
        return;

    int chunks = size / chunksize;
    for ( int i = 0; i < chunks; ++i )
    {
        filestr.read( dest + i * chunksize, chunksize );
    }
    filestr.read( dest + chunks * chunksize, size % chunksize );
}

void MeshFileLoader::ChunkWrite( std::fstream &filestr, char *src, unsigned size, unsigned chunksize )
{
    if (!size)
        return;

    int filesize = filestr.tellg();
    int chunks = size / chunksize;
    for ( int i = 0; i < chunks; ++i )
    {
        filestr.write( src + i * chunksize, chunksize );
    }
    filestr.write( src + chunks * chunksize, size % chunksize );
}

bool MeshFileLoader::Load( const std::string &filename, unsigned chunk /*= 256*/ )
{
    clear();

    std::fstream filestr;
    filestr.open( filename.c_str(), std::fstream::in | std::fstream::binary );
    if ( filestr.is_open() )
    {
        int header = 0;
        filestr.seekg(0, std::fstream::end);
        int filesize = filestr.tellg();
        filestr.seekg(0, std::fstream::beg);

        if ( filesize < 5 * sizeof(unsigned) )
            return false;

        filestr.read( (char*)&header, sizeof(unsigned) );
        if ( header != 0x1ee7ed )
            return false;

        filestr.read( (char*)&mNumVert, sizeof(unsigned) );
        filestr.read( (char*)&mNumNorm, sizeof(unsigned) );
        filestr.read( (char*)&mNumIndices, sizeof(unsigned) );
        filestr.read( (char*)&mNumTex, sizeof(unsigned) );
        if ( mNumVert > 0 )
            mVert = new ci::Vec3f[mNumVert];
        if ( mNumNorm > 0 )
            mNorm = new ci::Vec3f[mNumNorm];
        if ( mNumIndices > 0 )
            mIndices = new unsigned[mNumIndices];
        if ( mNumTex > 0 )
            mTex = new ci::Vec2f[mNumTex];
        if ( mNumVert > 0 )
            ChunkRead( filestr, (char*)mVert, mNumVert*sizeof(ci::Vec3f), chunk );
        if ( mNumNorm > 0 )
            ChunkRead( filestr, (char*)mNorm, mNumNorm*sizeof(ci::Vec3f), chunk );
        if ( mNumIndices > 0 )
            ChunkRead( filestr, (char*)mIndices, mNumIndices*sizeof(unsigned), chunk );
        if ( mNumTex > 0 )
            ChunkRead( filestr, (char*)mTex, mNumTex*sizeof(ci::Vec2f), chunk );
        filestr.close();

        return true;
    }
    return false;
}

void MeshFileLoader::Write( const std::string &filename, unsigned chunk /*= 256*/ )
{
    std::fstream filestr;
    filestr.open( filename.c_str(), std::fstream::out | std::fstream::binary );
    if ( filestr.is_open() )
    {
        int count = 3*sizeof(int)+mNumVert*sizeof(ci::Vec3f)+mNumNorm*sizeof(ci::Vec3f)+mNumIndices*sizeof(int)+mNumTex*sizeof(ci::Vec2f);
        //Header
        int header = 0x1ee7ed;
        filestr.write( (char*)&header, sizeof(unsigned) );

        filestr.write( (char*)&mNumVert, sizeof(unsigned) );
        filestr.write( (char*)&mNumNorm, sizeof(unsigned) );
        filestr.write( (char*)&mNumIndices, sizeof(unsigned) );
        filestr.write( (char*)&mNumTex, sizeof(unsigned) );
        if ( mNumVert > 0 )
            ChunkWrite( filestr, (char*)mVert, mNumVert*sizeof(ci::Vec3f), chunk );
        if ( mNumNorm > 0 )
            ChunkWrite( filestr, (char*)mNorm, mNumNorm*sizeof(ci::Vec3f), chunk );
        if ( mNumIndices > 0 )
            ChunkWrite( filestr, (char*)mIndices, mNumIndices*sizeof(unsigned), chunk );
        if ( mNumTex > 0 )
            ChunkWrite( filestr, (char*)mTex, mNumTex*sizeof(ci::Vec2f), chunk );
        filestr.close();
    }
}

void MeshFileLoader::setVerts( const std::vector<ci::Vec3f> &verts )
{
    if ( mVert )
    {
        delete [] mVert;
        mVert = nullptr;
    }

    mNumVert = verts.size();
    mVert = new ci::Vec3f[mNumVert];
    for ( unsigned i = 0; i < mNumVert; ++i )
    {
        mVert[i] = verts[i];
    }
}

void MeshFileLoader::setInd( const std::vector<unsigned> &inds )
{
    if ( mIndices )
    {
        delete [] mIndices;
        mIndices = nullptr;
    }

    mNumIndices = inds.size();
    mIndices = new unsigned[mNumIndices];
    for ( unsigned i = 0; i < mNumIndices; ++i )
    {
        mIndices[i] = inds[i];
    }
}

void MeshFileLoader::setTexs( const std::vector<ci::Vec2f> &texs )
{
    if ( mTex )
    {
        delete [] mTex;
        mTex = nullptr;
    }

    mNumTex = texs.size();
    mTex = new ci::Vec2f[mNumTex];
    for ( unsigned i = 0; i < mNumTex; ++i )
    {
        mTex[i] = texs[i];
    }
}

void MeshFileLoader::setNorms( const std::vector<ci::Vec3f> &norms )
{
    if ( mNorm )
    {
        delete [] mNorm;
        mNorm = nullptr;
    }

    mNumNorm = norms.size();
    mNorm = new ci::Vec3f[mNumNorm];
    for ( unsigned i = 0; i < mNumNorm; ++i )
    {
        mNorm[i] = norms[i];
    }
}

void MeshFileLoader::clear()
{
    if ( mVert )
    {
        delete [] mVert;
        mVert = nullptr;
    }

    if ( mNorm )
    {
        delete [] mNorm;
        mNorm = nullptr;
    }

    if ( mIndices )
    {
        delete [] mIndices;
        mIndices = nullptr;
    }

    if ( mTex )
    {
        delete [] mTex;
        mTex = nullptr;
    }

    mNumVert = 0;
    mNumNorm = 0;
    mNumTex = 0;
    mNumIndices = 0;
}

} //namespace util
} //namespace ui
} //namespace ds