#pragma once
#ifndef DS_UI_MESH_FILE_H
#define DS_UI_MESH_FILE_H

#include <fstream>
#include <string>
#include <vector>

namespace ds {
namespace ui {
namespace util {

//Original: Jeremy Robert Anderson
//License: Free as is.
struct MeshFile
{
    struct Vec3
    {
        Vec3(float x = 0.0f, float y = 0.0f, float z = 0.0f);
        bool operator !=( const Vec3 &rhs ) const;
        float x, y, z;
    };

    struct Vec2
    {
        Vec2(float x = 0.0f, float y = 0.0f);
        bool operator !=( const Vec2 &rhs ) const;
        float x, y;
    };

    MeshFile();

    ~MeshFile();

    void setVerts( const std::vector<Vec3> &verts );

    void setInd( const std::vector<unsigned> &inds );

    void setTexs( const std::vector<Vec2> &texs );

    void setNorms( const std::vector<Vec3> &norms );

    void ChunkRead( std::fstream &filestr, char *dest, unsigned size, unsigned chunksize );

    void ChunkWrite( std::fstream &filestr, char *src, unsigned size, unsigned chunksize );

    bool Load( const std::string &filename, unsigned chunk = 1024 );

    void Write( const std::string &filename, unsigned chunk = 1024 );

    void clear();

    Vec3 *mVert;
    Vec3 *mNorm;
    Vec2 *mTex;
    unsigned *mIndices;
    unsigned mNumVert;
    unsigned mNumNorm;
    unsigned mNumIndices;
    unsigned mNumTex;
};

} //namespace util
} //namespace ui
} //namespace ds

#endif//DS_UI_MESH_FILE_H
