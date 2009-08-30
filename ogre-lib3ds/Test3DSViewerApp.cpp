#include "precompiled.h"
#include "Test3DSViewerApp.h"

#include <string>
#include <sstream>
#include <boost/format.hpp>


static int  log_level = LIB3DS_LOG_INFO;

static long
fileio_seek_func(void *self, long offset, Lib3dsIoSeek origin) {
    FILE *f = (FILE*)self;
    int o;
    switch (origin) {
        case LIB3DS_SEEK_SET:
            o = SEEK_SET;
            break;

        case LIB3DS_SEEK_CUR:
            o = SEEK_CUR;
            break;

        case LIB3DS_SEEK_END:
            o = SEEK_END;
            break;
    }
    return (fseek(f, offset, o));
}


static long
fileio_tell_func(void *self) {
    FILE *f = (FILE*)self;
    return(ftell(f));
}


static size_t
fileio_read_func(void *self, void *buffer, size_t size) {
    FILE *f = (FILE*)self;
    return (fread(buffer, 1, size, f));
}


static size_t
fileio_write_func(void *self, const void *buffer, size_t size) {
    FILE *f = (FILE*)self;
    return (fwrite(buffer, 1, size, f));
}


static void 
fileio_log_func(void *self, Lib3dsLogLevel level, int indent, const char *msg)
{
    static const char * level_str[] = {
        "ERROR", "WARN", "INFO", "DEBUG"
    };
    if (log_level >=  level) {
        int i;
        printf("%5s : ", level_str[level]);
        for (i = 1; i < indent; ++i) printf("\t");
        printf("%s\n", msg);
    }
}






Test3DSViewerApp::Test3DSViewerApp(void)
{
}

Test3DSViewerApp::~Test3DSViewerApp(void)
{
}
//------------------------------------------------------------------------------
void Test3DSViewerApp::createScene()
{
    _createGrid(500);
    mCamera->setPosition(100, 100, 100);
    mCamera->lookAt(Vector3::ZERO);

    _build3dsModel();


    mBBset = mSceneMgr->createBillboardSet("Light BB");
    mBBset->setMaterialName("Objects/Flare");
    mLightFlare = mBBset->createBillboard(Vector3::ZERO);

    mLight = mSceneMgr->createLight("main light");
    mLight->setType(Light::LT_POINT);
    mLight->setDiffuseColour(ColourValue::White);
    mLight->setSpecularColour(ColourValue::White);

    mLightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("light node");
    mLightNode->attachObject(mLight);
    mLightNode->attachObject(mBBset);
    mLightNode->setPosition(-400, 100, 200);
    
}
//------------------------------------------------------------------------------
void Test3DSViewerApp::_createGrid(int _units)
{
    SceneNode *gridNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("WorldGrid Node");
    ManualObject *axes = mSceneMgr->createManualObject("AXES");


    axes->begin("WorldGrid/Axes", RenderOperation::OT_LINE_LIST);
    // X axis
    axes->position(-_units, 0.0, 0.0);     
    axes->colour(0.1, 0.0, 0.0);

    axes->position( _units, 0.0, 0.0);     
    axes->colour(1.0, 0.0, 0.0);

    // Y Axis
    axes->position(0.0, -_units, 0.0);     
    axes->colour(0.0, 0.1, 0.0);

    axes->position(0.0,  _units, 0.0);     
    axes->colour(0.0, 1.0, 0.0);

    // Z axis
    axes->position( 0.0, 0.0, -_units);     
    axes->colour(0.0, 0.0, 0.1);

    axes->position( 0.0, 0.0,  _units);  
    axes->colour(0.0, 0.0, 1.0);

    axes->end();
    gridNode->attachObject(axes);
    axes->setQueryFlags(0x00);

    ManualObject *grid = mSceneMgr->createManualObject("Grid Lines");

    grid->begin("WorldGrid/Lines", RenderOperation::OT_LINE_LIST);
    float c;
    for (int i = 10; i<=_units ; i+=10)
    {
        c = (i%100) ? 0.3 : 0.5;

        grid->position(-_units, 0, i);
        grid->colour(c, c, c);
        grid->position( _units, 0, i);
        grid->colour(c, c, c);

        grid->position(-_units, 0, -i);
        grid->colour(c, c, c);
        grid->position( _units, 0, -i);
        grid->colour(c, c, c);


        grid->position(i, 0, -_units);
        grid->colour(c, c, c);
        grid->position(i, 0,  _units);
        grid->colour(c, c, c);

        grid->position(-i, 0, -_units);
        grid->colour(c, c, c);
        grid->position(-i, 0,  _units);
        grid->colour(c, c, c);
    }


    grid->end();
    grid->setQueryFlags(0x00);
    gridNode->attachObject(grid);
}
//------------------------------------------------------------------------------
void Test3DSViewerApp::_build3dsModel()
{
    SceneNode *modelNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("3ds model");
    int result;
    //mFile = fopen("cube.3ds", "rb");
    //mFile = fopen("../media/3ds/Modern-home-interior1.3DS", "rb");
    mFile = fopen("../media/3ds/indochine.3DS", "rb");

    m3dsFile = lib3ds_file_new();

    memset(&m3dsIo, 0, sizeof(m3dsIo));
    m3dsIo.self = mFile;
    m3dsIo.seek_func = fileio_seek_func;
    m3dsIo.tell_func = fileio_tell_func;
    m3dsIo.read_func = fileio_read_func;
    m3dsIo.write_func = fileio_write_func;
    m3dsIo.log_func = fileio_log_func;

    result =  lib3ds_file_read(m3dsFile, &m3dsIo);

    mObjectBuilder = mSceneMgr->createManualObject("3ds cube");


    
    for(int j=0 ; j<m3dsFile->nmeshes ; ++j)
    {
        Lib3dsMesh *mesh = m3dsFile->meshes[j];
    
        float (*normals)[3] = (float(*)[3])malloc(sizeof(float) * 9 * mesh->nfaces);
        lib3ds_mesh_calculate_vertex_normals(mesh, normals);


        mObjectBuilder->begin("Gray", RenderOperation::OT_TRIANGLE_LIST);
        float p[3], t[2];
        for (int i = 0; i < mesh->nvertices; ++i) {
            lib3ds_vector_copy(p, mesh->vertices[i]);
            mObjectBuilder->position(p[1], p[2], p[0]);
            if (mesh->texcos) {
                mObjectBuilder->textureCoord(mesh->texcos[i][0], mesh->texcos[i][1]);
                
            }
            Vector3 n(normals[i][0], normals[i][2], normals[i][1]);
            n.normalise();
            mObjectBuilder->normal(n);
        }
        for(int i=0 ; i<mesh->nfaces ; i++)
        {
            mObjectBuilder->index(mesh->faces[i].index[0]);
            mObjectBuilder->index(mesh->faces[i].index[1]);
            mObjectBuilder->index(mesh->faces[i].index[2]);
        }

        mObjectBuilder->end();
    }
   /* MeshPtr ogreMesh = mObjectBuilder->convertToMesh("indochine.3ds");
    ogreMesh->buildEdgeList();
    
    Entity *ent = mSceneMgr->createEntity("indo", "indochine.3ds");*/

    modelNode->attachObject(mObjectBuilder);



    /*Log *log = LogManager::getSingleton().createLog("3dsdump.log");

    Lib3dsNode *first = m3dsFile->nodes;
    _dumpNode(log, first, 0, "");*/


    fclose(mFile);
}

void Test3DSViewerApp::_dumpNode(Log *_log, Lib3dsNode *_node
                                , int _level, std::string _basename)
{
    boost::format fmt("%s [mesh : %s]");
    std::stringstream s;

    Lib3dsNode *p = _node;
    for(; p ; p=p->next)
    {
        if (p->type == LIB3DS_NODE_MESH_INSTANCE) 
        {
            std::string fullName = _basename + "/" + std::string(p->name);

            Lib3dsMesh *mesh = lib3ds_file_mesh_for_node(m3dsFile, p);
            if(mesh)
                fmt % fullName % mesh->name;
            
            else
                fmt % fullName % "N/A";

            _log->logMessage(fmt.str());
            _dumpNode(_log, p->childs, _level+1, fullName);
        }
    }
}