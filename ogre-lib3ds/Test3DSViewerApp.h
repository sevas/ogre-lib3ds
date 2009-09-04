#pragma once

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <boost/shared_ptr.hpp>
#include <lib3ds.h>
#include <Ogre.h>


#include "MovableText.h"
#include "ExampleApplication.h"


    

using namespace Ogre;

class Test3DSViewerApp :
    public ExampleApplication
{
public:
    Test3DSViewerApp(void);
    ~Test3DSViewerApp(void);
    

    void createScene();
    void _createGrid(int);
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
                            ,int);

protected:
    FILE *mFile;
    Lib3dsFile *m3dsFile;
    Lib3dsIo m3dsIo;

    ManualObject *mObjectBuilder;

    BillboardSet *mBBset;
    Billboard *mLightFlare;
    Light *mLight;
    SceneNode *mLightNode;

    std::map<std::string, MeshPtr> mMeshes;
    std::list<MeshPtr> mMeshVect;

    Log *m3dsBuildLog;
    int mDummyCnt, mNodeCnt;

    typedef boost::shared_ptr<MovableText> MovableTextPtr;
    std::map<std::string, MeshPtr> mCenteredMeshes;
    std::map<std::string, MovableTextPtr> mNodeLabels;

};