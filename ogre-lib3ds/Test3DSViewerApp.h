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


    

using namespace Ogre;

class Test3DSViewerApp :
    public OgreApplication
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
                        ,const std::stringstream&
                        ,const std::string &
                        ,bool _transpose=false);

   

protected:
    FILE *mFile;    
    Lib3dsFile *m3dsFile;
    Lib3dsIo m3dsIo;

    ManualObject *mObjectBuilder;

    std::map<std::string, MeshPtr> mMeshes;
    std::list<MeshPtr> mMeshVect;

    Log *m3dsBuildLog;
    int mDummyCnt, mNodeCnt;

    typedef boost::shared_ptr<MovableText> MovableTextPtr;
    std::map<std::string, MeshPtr> mCenteredMeshes;
    std::map<std::string, MovableTextPtr> mNodeLabels;

    Ogre::AxisAlignedBox mAABB;
};