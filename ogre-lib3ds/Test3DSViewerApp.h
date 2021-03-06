#pragma once

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sstream>

#include <boost/shared_ptr.hpp>
#include <lib3ds.h>
#include <Ogre.h>


#include "MovableText.h"
#include "OgreApplication.h"


class Test3DSViewerApp : public OgreApplication
{
public:
    Test3DSViewerApp(void);
    ~Test3DSViewerApp(void);
    
    void createScene();
    
protected:
    //void _createGrid(int);

    void _buildRadiator();

    void _build3dsModel();
    void _buildSubtree(Lib3dsNode*, const std::string&
                      ,SceneNode*);
    MeshPtr _convert3dsMeshToOgreMesh(Lib3dsMesh*
                                     ,Lib3dsMeshInstanceNode*
                                     ,const std::string&);

    void _dumpNode(Log*, Lib3dsNode*, int, std::string);

    void _createMeshesFrom3dsFile(Lib3dsFile*);
    void _buildSceneFromNode(Lib3dsNode*, SceneNode*
                            ,const std::string&
                            ,int
                            ,bool);

    void _logXformMatrix(const Matrix4&
                        ,const std::string&
                        ,const std::string &
                        ,bool _transpose=false);

    static std::string _makeIndentSpaces(int level);
   

protected:
    FILE *mFile;    
    Lib3dsFile *m3dsFile;
    Lib3dsIo m3dsIo;

    Ogre::ManualObject *mObjectBuilder;

    std::map<std::string, Ogre::MeshPtr> mMeshes;
    std::list<Ogre::MeshPtr> mMeshVect;

    Ogre::Log *m3dsBuildLog;
    int mDummyCnt, mNodeCnt;

    typedef boost::shared_ptr<MovableText> MovableTextPtr;
    std::map<std::string, Ogre::MeshPtr> mCenteredMeshes;
    std::map<std::string, MovableTextPtr> mNodeLabels;

    Ogre::AxisAlignedBox mAABB;
};