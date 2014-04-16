#pragma once
#ifndef DS_UI_MESH_FILE_H
#define DS_UI_MESH_FILE_H

#include <fstream>
#include <string>
#include <vector>
#include <cinder/Vector.h>

namespace ds {
namespace ui {
namespace util {

//Original: Jeremy Robert Anderson
//License: Free as is.
struct MeshFileLoader
{
    MeshFileLoader();

    ~MeshFileLoader();

    void setVerts( const std::vector<ci::Vec3f> &verts );

    void setInd( const std::vector<unsigned> &inds );

    void setTexs( const std::vector<ci::Vec2f> &texs );

    void setNorms( const std::vector<ci::Vec3f> &norms );

    void ChunkRead( std::fstream &filestr, char *dest, unsigned size, unsigned chunksize );

    void ChunkWrite( std::fstream &filestr, char *src, unsigned size, unsigned chunksize );

    bool Load( const std::string &filename, unsigned chunk = 1024 );

    void Write( const std::string &filename, unsigned chunk = 1024 );

    void clear();

    ci::Vec3f *mVert;
    ci::Vec3f *mNorm;
    ci::Vec2f *mTex;
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
