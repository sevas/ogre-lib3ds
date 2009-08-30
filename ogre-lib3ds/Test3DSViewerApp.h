#pragma once

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <Ogre.h>
#include "ExampleApplication.h"

extern "C"{
#include "lib3ds.h"
};


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

    void _dumpNode(Log*, Lib3dsNode*, int, std::string);

protected:
    FILE *mFile;
    Lib3dsFile *m3dsFile;
    Lib3dsIo m3dsIo;

    ManualObject *mObjectBuilder;

    BillboardSet *mBBset;
    Billboard *mLightFlare;
    Light *mLight;
    SceneNode *mLightNode;

    std::map<std::string, Entity*> mMeshes;
};