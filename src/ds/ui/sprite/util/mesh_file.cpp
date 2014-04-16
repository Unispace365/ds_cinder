#include "ds/ui/sprite/util/mesh_file.h"

namespace ds {
namespace ui {
namespace util {

MeshFile::MeshFile()
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

MeshFile::~MeshFile()
{
    clear();
}

void MeshFile::ChunkRead( std::fstream &filestr, char *dest, unsigned size, unsigned chunksize )
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

void MeshFile::ChunkWrite( std::fstream &filestr, char *src, unsigned size, unsigned chunksize )
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

bool MeshFile::Load( const std::string &filename, unsigned chunk /*= 256*/ )
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
            mVert = new Vec3[mNumVert];
        if ( mNumNorm > 0 )
            mNorm = new Vec3[mNumNorm];
        if ( mNumIndices > 0 )
            mIndices = new unsigned[mNumIndices];
        if ( mNumTex > 0 )
            mTex = new Vec2[mNumTex];
        if ( mNumVert > 0 )
            ChunkRead( filestr, (char*)mVert, mNumVert*sizeof(Vec3), chunk );
        if ( mNumNorm > 0 )
            ChunkRead( filestr, (char*)mNorm, mNumNorm*sizeof(Vec3), chunk );
        if ( mNumIndices > 0 )
            ChunkRead( filestr, (char*)mIndices, mNumIndices*sizeof(unsigned), chunk );
        if ( mNumTex > 0 )
            ChunkRead( filestr, (char*)mTex, mNumTex*sizeof(Vec2), chunk );
        filestr.close();

        return true;
    }
    return false;
}

void MeshFile::Write( const std::string &filename, unsigned chunk /*= 256*/ )
{
    std::fstream filestr;
    filestr.open( filename.c_str(), std::fstream::out | std::fstream::binary );
    if ( filestr.is_open() )
    {
        int count = 3*sizeof(int)+mNumVert*sizeof(Vec3)+mNumNorm*sizeof(Vec3)+mNumIndices*sizeof(int)+mNumTex*sizeof(Vec2);
        //Header
        int header = 0x1ee7ed;
        filestr.write( (char*)&header, sizeof(unsigned) );

        filestr.write( (char*)&mNumVert, sizeof(unsigned) );
        filestr.write( (char*)&mNumNorm, sizeof(unsigned) );
        filestr.write( (char*)&mNumIndices, sizeof(unsigned) );
        filestr.write( (char*)&mNumTex, sizeof(unsigned) );
        if ( mNumVert > 0 )
            ChunkWrite( filestr, (char*)mVert, mNumVert*sizeof(Vec3), chunk );
        if ( mNumNorm > 0 )
            ChunkWrite( filestr, (char*)mNorm, mNumNorm*sizeof(Vec3), chunk );
        if ( mNumIndices > 0 )
            ChunkWrite( filestr, (char*)mIndices, mNumIndices*sizeof(unsigned), chunk );
        if ( mNumTex > 0 )
            ChunkWrite( filestr, (char*)mTex, mNumTex*sizeof(Vec2), chunk );
        filestr.close();
    }
}

void MeshFile::setVerts( const std::vector<Vec3> &verts )
{
    if ( mVert )
    {
        delete [] mVert;
        mVert = nullptr;
    }

    mNumVert = verts.size();
    mVert = new Vec3[mNumVert];
    for ( unsigned i = 0; i < mNumVert; ++i )
    {
        mVert[i] = verts[i];
    }
}

void MeshFile::setInd( const std::vector<unsigned> &inds )
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

void MeshFile::setTexs( const std::vector<Vec2> &texs )
{
    if ( mTex )
    {
        delete [] mTex;
        mTex = nullptr;
    }

    mNumTex = texs.size();
    mTex = new Vec2[mNumTex];
    for ( unsigned i = 0; i < mNumTex; ++i )
    {
        mTex[i] = texs[i];
    }
}

void MeshFile::setNorms( const std::vector<Vec3> &norms )
{
    if ( mNorm )
    {
        delete [] mNorm;
        mNorm = nullptr;
    }

    mNumNorm = norms.size();
    mNorm = new Vec3[mNumNorm];
    for ( unsigned i = 0; i < mNumNorm; ++i )
    {
        mNorm[i] = norms[i];
    }
}

void MeshFile::clear()
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

MeshFile::Vec3::Vec3(float x, float y, float z)
    : x(x)
    , y(y)
    , z(z)
{

}

bool MeshFile::Vec3::operator !=( const MeshFile::Vec3 &rhs ) const
{
    if ( x != rhs.x || y != rhs.y || z != rhs.z )
        return true;
    return false;
}

MeshFile::Vec2::Vec2(float x, float y)
    : x(x)
    , y(y)
{

}

bool MeshFile::Vec2::operator !=( const MeshFile::Vec2 &rhs ) const
{
    if ( x != rhs.x || y != rhs.y )
        return true;
    return false;
}


} //namespace util
} //namespace ui
} //namespace ds