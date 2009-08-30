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
:mDummyCnt(0)
,mNodeCnt(0)
{
}

Test3DSViewerApp::~Test3DSViewerApp(void)
{
}
//------------------------------------------------------------------------------
void Test3DSViewerApp::createScene()
{
    m3dsBuildLog = LogManager::getSingleton().createLog("3dsbuild.log");

    mSceneMgr->setNormaliseNormalsOnScale(true);
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
    mLightNode->setPosition(-300, 100, 200);
    
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
    //mFile = fopen("../media/3ds/cube.3ds", "rb");
    //mFile = fopen("../media/3ds/Modern-home-interior1.3DS", "rb");
   // mFile = fopen("../media/3ds/indochine.3DS", "rb");
    //mFile = fopen("../media/3ds/monaco.3DS", "rb");

    //m3dsFile = lib3ds_file_new();

    //memset(&m3dsIo, 0, sizeof(m3dsIo));
    //m3dsIo.self = mFile;
    //m3dsIo.seek_func = fileio_seek_func;
    //m3dsIo.tell_func = fileio_tell_func;
    //m3dsIo.read_func = fileio_read_func;
    //m3dsIo.write_func = fileio_write_func;
    //m3dsIo.log_func = fileio_log_func;

    //result =  lib3ds_file_read(m3dsFile, &m3dsIo);


   // mObjectBuilder = mSceneMgr->createManualObject("3ds cube");


   // 
   // for(int j=0 ; j<m3dsFile->nmeshes ; ++j)
   // {
   //     Lib3dsMesh *mesh = m3dsFile->meshes[j];
   // 
   //     float (*normals)[3] = (float(*)[3])malloc(sizeof(float) * 9 * mesh->nfaces);
   //     lib3ds_mesh_calculate_vertex_normals(mesh, normals);


   //     mObjectBuilder->begin("Gray", RenderOperation::OT_TRIANGLE_LIST);
   //     float p[3], t[2];
   //     for (int i = 0; i < mesh->nvertices; ++i) {
   //         lib3ds_vector_copy(p, mesh->vertices[i]);
   //         mObjectBuilder->position(p[1], p[2], p[0]);
   //         if (mesh->texcos) {
   //             mObjectBuilder->textureCoord(mesh->texcos[i][0], mesh->texcos[i][1]);
   //             
   //         }
   //         Vector3 n(normals[i][0], normals[i][2], normals[i][1]);
   //         n.normalise();
   //         mObjectBuilder->normal(n);
   //     }
   //     for(int i=0 ; i<mesh->nfaces ; i++)
   //     {
   //         mObjectBuilder->index(mesh->faces[i].index[0]);
   //         mObjectBuilder->index(mesh->faces[i].index[1]);
   //         mObjectBuilder->index(mesh->faces[i].index[2]);
   //     }

   //     mObjectBuilder->end();
   // }
   ///* MeshPtr ogreMesh = mObjectBuilder->convertToMesh("indochine.3ds");
   // ogreMesh->buildEdgeList();
   // 
   // Entity *ent = mSceneMgr->createEntity("indo", "indochine.3ds");*/

   // modelNode->attachObject(mObjectBuilder);



    /*Log *log = LogManager::getSingleton().createLog("3dsdump.log");

    Lib3dsNode *first = m3dsFile->nodes;
    _dumpNode(log, first, 0, "");*/

    //m3dsFile =  lib3ds_file_open("../media/3ds/monaco.3DS");
    m3dsFile =  lib3ds_file_open("../media/3ds/Modern-home-interior1.3DS");

    if (!m3dsFile->nodes)
        lib3ds_file_create_nodes_for_meshes(m3dsFile);

    lib3ds_file_eval(m3dsFile, 0);

    _buildSubtree( m3dsFile->nodes, "/", modelNode);

    modelNode->scale(0.1, 0.1, 0.1);
    modelNode->pitch(Degree(-90));

    lib3ds_file_free(m3dsFile);
 
}
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
void Test3DSViewerApp::_buildSubtree(Lib3dsNode *_node
                                    ,const std::string &_basename
                                    ,SceneNode *_parentNode)
{
    boost::format fullNameFmt("%s/%06d%s");

    Lib3dsNode *p;
    for(p = _node ; p ; p=p->next)
    {
        
        if (p->type == LIB3DS_NODE_MESH_INSTANCE) 
        {
            mNodeCnt++;
            fullNameFmt % _basename % p->node_id % p->name;

            std::string fullName = fullNameFmt.str();

            Lib3dsMeshInstanceNode *n = (Lib3dsMeshInstanceNode*) p;

            m3dsBuildLog->logMessage(boost::str(boost::format("building new node (%d) : %s") % mNodeCnt % fullName));

            SceneNode *newNode = _parentNode->createChildSceneNode(fullName + " Node");
            //newNode->setPosition(n->pivot[0], n->pivot[1], n->pivot[2]);

            Lib3dsMesh *mesh = lib3ds_file_mesh_for_node(m3dsFile, (Lib3dsNode*)n);
            
            if(mesh && mesh->nvertices)
            {

                MeshPtr newMesh = _convert3dsMeshToOgreMesh(mesh, n, fullName);
                mMeshVect.push_back(newMesh);
                mMeshes[newMesh->getName()] = newMesh;
                    
                m3dsBuildLog->logMessage(boost::str(boost::format("attaching %s to node %s")% newMesh->getName() % fullName));
                Entity *ent = mSceneMgr->createEntity(fullName+" Ent", newMesh->getName());
                
                newNode->attachObject(ent);
            }
            
            _buildSubtree(p->childs, fullName, newNode);
        }
    }
}
//------------------------------------------------------------------------------
MeshPtr Test3DSViewerApp::_convert3dsMeshToOgreMesh(Lib3dsMesh *_mesh
                                                    , Lib3dsMeshInstanceNode *_node
                                                    , const std::string &_basename)
{
    boost::format fmt("%s -- %s");
    fmt % _basename % _mesh->name;
    std::string fullMeshName = fmt.str();

    m3dsBuildLog->logMessage(std::string("building new mesh : ") + fullMeshName+".mesh");
    ManualObject *newObject = mSceneMgr->createManualObject(fullMeshName);
    Lib3dsMesh *mesh = _mesh;
    Lib3dsMeshInstanceNode *node = _node;


    float (*orig_vertices)[3];

    //save mesh
    orig_vertices = (float(*)[3])malloc(sizeof(float) * 3 * mesh->nvertices);
    memcpy(orig_vertices, mesh->vertices, sizeof(float) * 3 * mesh->nvertices);
    
    // create translated ogre manualobject
    {
        float inv_matrix[4][4], M[4][4];
        float tmp[3];
        int i;

        lib3ds_matrix_copy(M, node->base.matrix);
        lib3ds_matrix_translate(M, -node->pivot[0], -node->pivot[1], -node->pivot[2]);
        lib3ds_matrix_copy(inv_matrix, mesh->matrix);
        lib3ds_matrix_inv(inv_matrix);
        lib3ds_matrix_mult(M, M, inv_matrix);

        for (i = 0; i < mesh->nvertices; ++i) {
            lib3ds_vector_transform(tmp, M, mesh->vertices[i]);
            lib3ds_vector_copy(mesh->vertices[i], tmp);
        }
   
        float (*normals)[3] = (float(*)[3])malloc(sizeof(float) * 9 * mesh->nfaces);
        lib3ds_mesh_calculate_vertex_normals(mesh, normals);
        
        // copy everything to vertexbuffers
        newObject->begin("Gray", RenderOperation::OT_TRIANGLE_LIST);

        
        for (int i = 0; i < mesh->nvertices; ++i) 
        {
            
            newObject->position( mesh->vertices[i][0]
                               , mesh->vertices[i][1]
                               , mesh->vertices[i][2]);
            if (mesh->texcos) 
            {
                newObject->textureCoord(mesh->texcos[i][0], mesh->texcos[i][1]);
            }
            Vector3 n(normals[i][0], normals[i][1], normals[i][2]);
            
            newObject->normal(n);
        }

        for(int i=0 ; i<mesh->nfaces ; i++)
        {
            newObject->index(mesh->faces[i].index[0]);
            newObject->index(mesh->faces[i].index[1]);
            newObject->index(mesh->faces[i].index[2]);
        }

        newObject->end();
        free(normals); 
    }
    //restore mesh for future use
    memcpy(mesh->vertices, orig_vertices, sizeof(float) * 3 * mesh->nvertices);
    free(orig_vertices);

    // create ogre mesh from manualobject
    MeshPtr newMesh = newObject->convertToMesh(fullMeshName + ".mesh");
    //newMesh->buildTangentVectors();
    mSceneMgr->destroyManualObject(newObject);
    return newMesh;
}