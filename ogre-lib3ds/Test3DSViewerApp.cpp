#include "precompiled.h"
#include "Test3DSViewerApp.h"

#include <string>
#include <sstream>
#include <boost/format.hpp>


static int  log_level = LIB3DS_LOG_INFO;


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
    mLightNode->setPosition(-300, 100, -200);
    //mLightNode->setPosition(0, 100, 0);
    
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
 
    //m3dsFile =  lib3ds_file_open("../media/3ds/test3.3DS");
    //m3dsFile =  lib3ds_file_open("../media/3ds/indochine.3DS");
    //m3dsFile =  lib3ds_file_open("../media/3ds/monaco.3DS");
    //m3dsFile =  lib3ds_file_open("../media/3ds/amphimath_walls.3DS");
    //m3dsFile =  lib3ds_file_open("../media/3ds/amphimath2.3DS");
    m3dsFile =  lib3ds_file_open("../media/3ds/amphimath_pillars.3DS");
    //m3dsFile =  lib3ds_file_open("../media/3ds/Modern-home-interior1.3DS");
    //m3dsFile =  lib3ds_file_open("../media/3ds/test_pivot2.3DS");
    if (!m3dsFile->nodes)
        lib3ds_file_create_nodes_for_meshes(m3dsFile);

    lib3ds_file_eval(m3dsFile, 0);

    _createMeshesFrom3dsFile(m3dsFile);
    _buildSceneFromNode(m3dsFile->nodes, modelNode, "/", 0);

    //_buildSubtree( m3dsFile->nodes, "/", modelNode);

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


            //// node params
            m3dsBuildLog->logMessage(boost::str(boost::format("building new node (%d) : %s") % mNodeCnt % fullName));

            SceneNode *newNode = _parentNode->createChildSceneNode(fullName + " Node");
            Lib3dsMesh *mesh = lib3ds_file_mesh_for_node(m3dsFile, (Lib3dsNode*)n);
            
            if(mesh && mesh->nvertices)
            {

                MeshPtr newMesh = _convert3dsMeshToOgreMesh(mesh, n, fullName);
                if(!newMesh.isNull())
                {
                    mMeshVect.push_back(newMesh);
                    mMeshes[newMesh->getName()] = newMesh;

                    m3dsBuildLog->logMessage(boost::str(boost::format("attaching %s to node %s")% newMesh->getName() % fullName));
                    Entity *ent = mSceneMgr->createEntity(fullName+" Ent", newMesh->getName());

                    newNode->attachObject(ent);
                }
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
    //{
        float inv_matrix[4][4], M[4][4];
        float tmp[3];
        //int i;

        lib3ds_matrix_copy(M, node->base.matrix);
        lib3ds_matrix_translate(M, -node->pivot[0], -node->pivot[1], -node->pivot[2]);
        lib3ds_matrix_copy(inv_matrix, mesh->matrix);
        lib3ds_matrix_inv(inv_matrix);
        lib3ds_matrix_mult(M, M, inv_matrix);

        for (int i = 0; i < mesh->nvertices; ++i) {
            lib3ds_vector_transform(tmp, M, mesh->vertices[i]);
            lib3ds_vector_copy(mesh->vertices[i], tmp);
        }
        float (*normals)[3] = (float(*)[3])malloc(sizeof(float) * 9 * mesh->nfaces);
        lib3ds_mesh_calculate_vertex_normals(mesh, normals);
        
        // copy everything to vertex buffers
        newObject->begin("Gray", RenderOperation::OT_TRIANGLE_LIST);

        int idx = 0;

        // foreach tri
        for(int tri_idx = 0 ; tri_idx < mesh->nfaces ; ++tri_idx)
        {
           
            // foreach vertex in tri
            for(int j=0 ; j<3 ; ++j)
            {
                Vector3 pos, norm;
                Vector2 tc;
                   
                pos = Vector3(mesh->vertices[mesh->faces[tri_idx].index[j]]);
                newObject->position(pos);
                                
                if(mesh->texcos)
                    tc = Vector2(mesh->texcos[mesh->faces[tri_idx].index[j]]);
                norm = Vector3(normals[idx]);


                newObject->normal(norm);

                newObject->index(idx++);
            }
        }

        newObject->end();
        free(normals); 
    //}
    //restore mesh for future use
    memcpy(mesh->vertices, orig_vertices, sizeof(float) * 3 * mesh->nvertices);
    free(orig_vertices);

    
    MeshPtr newMesh;
    if(idx)
    {
        // create ogre mesh from manualobject
        newMesh = newObject->convertToMesh(fullMeshName + ".mesh");
        //newMesh->buildTangentVectors();
        mSceneMgr->destroyManualObject(newObject);
    }
    else
        newMesh.setNull();

    return newMesh;
}
//------------------------------------------------------------------------------
void Test3DSViewerApp::_createMeshesFrom3dsFile(Lib3dsFile *_3dsfile)
{
    m3dsBuildLog->logMessage("----------------  building meshes");
    for(int i=0 ; i<_3dsfile->nmeshes ; ++i)
    {
        Lib3dsMesh *mesh = _3dsfile->meshes[i];
        ManualObject *newObject = mSceneMgr->createManualObject(boost::str(boost::format("%d_%s")% i % mesh->name));

        m3dsBuildLog->logMessage(std::string("building new mesh : ") + newObject->getName());

        float (*orig_vertices)[3];
        orig_vertices = (float(*)[3])malloc(sizeof(float) * 3 * mesh->nvertices);
        memcpy(orig_vertices, mesh->vertices, sizeof(float) * 3 * mesh->nvertices);

        // transform mesh back to origin
        float inv_matrix[4][4], M[4][4];
        float tmp[3];

        lib3ds_matrix_copy(inv_matrix, mesh->matrix);
        lib3ds_matrix_inv(inv_matrix);
        lib3ds_matrix_copy(M, inv_matrix);

        for (int i = 0; i < mesh->nvertices; ++i) {
            lib3ds_vector_transform(tmp, M, mesh->vertices[i]);
            lib3ds_vector_copy(mesh->vertices[i], tmp);
        }

        // normals
        float (*normals)[3] = (float(*)[3])malloc(sizeof(float) * 9 * mesh->nfaces);
        lib3ds_mesh_calculate_vertex_normals(mesh, normals);

        // create an ogre object for easy OgreMesh conversion
        // Gray = default material
        // TODO: better default material
        newObject->begin("Gray", RenderOperation::OT_TRIANGLE_LIST);

        int idx = 0;
        // foreach tri
        for(int tri_idx = 0 ; tri_idx < mesh->nfaces ; ++tri_idx)
        {
            // foreach vertex in tri
            for(int j=0 ; j<3 ; ++j)
            {
                Vector3 pos, norm;
                Vector2 tc;

                pos = Vector3(mesh->vertices[mesh->faces[tri_idx].index[j]]);
                newObject->position(pos);

                if(mesh->texcos)
                {
                    tc = Vector2(mesh->texcos[mesh->faces[tri_idx].index[j]]);
                    newObject->textureCoord(tc);
                }

                norm = Vector3(normals[idx]);

                newObject->normal(norm);
                newObject->index(idx++);
            }
        }

        newObject->end();
        free(normals); 
        //restore mesh for future use
        memcpy(mesh->vertices, orig_vertices, sizeof(float) * 3 * mesh->nvertices);
        free(orig_vertices);


        MeshPtr newMesh;
        if(idx)
        {
            boost::format fmt("creating new Ogre::Mesh %s [%d vertices]");
            fmt % mesh->name % idx;
            m3dsBuildLog->logMessage(fmt.str());

            newMesh = newObject->convertToMesh(newObject->getName());
            newMesh->buildEdgeList();
        }
        else
        {
            boost::format fmt("mesh %s had %d vertices");
            fmt % mesh->name % idx;
            m3dsBuildLog->logMessage(fmt.str());
            newMesh.setNull();
        }

        mSceneMgr->destroyManualObject(newObject);
        mCenteredMeshes[mesh->name] = newMesh;
    }
    m3dsBuildLog->logMessage("----------------  building meshes ended");
}
//------------------------------------------------------------------------------
void Test3DSViewerApp::_buildSceneFromNode(Lib3dsNode *_3dsNode, SceneNode *_parentNode
                                           ,const std::string &_basename
                                           ,int _level)
{
    boost::format fullNameFmt("%s/%06d%s");
    Lib3dsNode *p;
    SceneNode *newNode;
    for(p = _3dsNode ; p ; p=p->next)
    {
        std::stringstream spaces;
        for(int i=0 ; i<_level ; ++i)
            spaces << " ";
        boost::format fmt("%s (%d) \t %s \t [%s]");
        fmt % spaces.str() % p->node_id % p->name;
        switch(p->type)
        {
        case LIB3DS_NODE_AMBIENT_COLOR:    fmt % "Ambient Color"; break;
        case LIB3DS_NODE_MESH_INSTANCE:    fmt % "Mesh Instance"; break;
        case LIB3DS_NODE_CAMERA:           fmt % "Camera"; break;
        case LIB3DS_NODE_CAMERA_TARGET:    fmt % "Camera Target"; break;
        case LIB3DS_NODE_OMNILIGHT:        fmt % "Omnilight"; break;
        case LIB3DS_NODE_SPOTLIGHT:        fmt % "Spotlight"; break;
        case LIB3DS_NODE_SPOTLIGHT_TARGET: fmt % "Spotlight Target"; break;
        default: break;
        }

        m3dsBuildLog->logMessage(fmt.str());
        
        m3dsBuildLog->logMessage(spaces.str() + "    matrix[4][4] : ");
        for(int i=0 ; i<4 ; ++i)
        {
            std::stringstream matrixLine;
            boost::format matrixLineFmt("%s%s %+.2f   %+.2f   %+.2f   %+.2f");
            matrixLineFmt % spaces.str() % "    ";
            matrixLineFmt   % p->matrix[0][i] % p->matrix[1][i]
                            % p->matrix[2][i] % p->matrix[3][i];
            m3dsBuildLog->logMessage(matrixLineFmt.str());
        }

        

        if (p->type == LIB3DS_NODE_MESH_INSTANCE) 
        {
            mNodeCnt++;
            fullNameFmt % _basename % p->node_id % p->name;
            std::string fullName = fullNameFmt.str();

            Lib3dsMeshInstanceNode *n = (Lib3dsMeshInstanceNode*) p;

            //m3dsBuildLog->logMessage(boost::str(boost::format("building new node (%d) : %s") % mNodeCnt % fullName));

            newNode = _parentNode->createChildSceneNode(fullName + " Node");
                 
            SceneNode *nodeCenter = newNode->createChildSceneNode(fullName + " NodeCenter");
            Entity *nodeSphere = mSceneMgr->createEntity(fullName + "sphere", "axes.mesh");
            //nodeSphere->setMaterialName("Gray");
            nodeCenter->attachObject(nodeSphere);
            float scale = 20.0 / nodeSphere->getBoundingBox().getSize().x;
            nodeCenter->setScale(scale, scale, scale);

            MovableTextPtr nodeLabel(new MovableText(fullName, std::string(p->name)));
            nodeLabel->setTextAlignment(MovableText::H_CENTER, MovableText::V_BELOW);
            nodeLabel->setCharacterHeight(7);
            nodeLabel->setVisible(true);
            nodeLabel->showOnTop(true);
            
            //nodeLabel = newObject.ObjectNode->createChildSceneNode(name+"labelnode", Vector3(0, 120, 0));
            nodeCenter->attachObject(&(*nodeLabel));
            mNodeLabels[fullName] = nodeLabel;





            //newNode->translate(n->pos[0], n->pos[1], n->pos[2]);
            newNode->scale(n->scl[0], n->scl[1], n->scl[2]);
            newNode->rotate(Quaternion(n->rot[3], n->rot[0], n->rot[1], n->rot[2]));
            
           
            //newNode->translate(-n->pivot[0], -n->pivot[1], -n->pivot[2]);

            //newNode->setVisible((bool)n->hide);


            // log node xforms
            m3dsBuildLog->logMessage(spaces.str() + "    pivot : "
                                                  + StringConverter::toString(Vector3(-n->pivot[0]
                                                                                     ,-n->pivot[1]
                                                                                     , -n->pivot[2])));

            m3dsBuildLog->logMessage(spaces.str() + "    mesh instance transform : ");
            Vector3 pos, scl;
            Quaternion rot;
            scl = Vector3(n->scl[0], n->scl[1], n->scl[2]);
            pos = Vector3(n->pos[0], n->pos[1], n->pos[2]);
            rot = Quaternion(n->rot[3], n->rot[0], n->rot[1], n->rot[2]);

            Matrix4 M;
            M.makeTransform(pos, scl, rot);
            for(int i=0 ; i<4 ; ++i)
            {
                std::stringstream matrixLine;
                boost::format matrixLineFmt("%s%s %+.2f   %+.2f   %+.2f   %+.2f");
                matrixLineFmt % spaces.str() % "    ";
                matrixLineFmt   % M[i][0] % M[i][1] % M[i][2] % M[i][3];
                m3dsBuildLog->logMessage(matrixLineFmt.str());
            }


            
            Lib3dsMesh *mesh = lib3ds_file_mesh_for_node(m3dsFile, (Lib3dsNode*)n);
            if (mesh && mesh->name)
            {
                std::string meshName = mesh->name;

                // log mesh xforms
                m3dsBuildLog->logMessage(spaces.str() + "    mesh matrix : ");
                for(int i=0 ; i<4 ; ++i)
                {
                    std::stringstream matrixLine;
                    boost::format matrixLineFmt("%s%s %+.2f   %+.2f   %+.2f   %+.2f");
                    matrixLineFmt % spaces.str() % "    ";
                    matrixLineFmt   % mesh->matrix[0][i] % mesh->matrix[1][i]
                                    % mesh->matrix[2][i] % mesh->matrix[3][i];
                    m3dsBuildLog->logMessage(matrixLineFmt.str());
                }


                MeshPtr meshToAdd = mCenteredMeshes[meshName];
                if(! meshToAdd.isNull())
                {
                    //m3dsBuildLog->logMessage(boost::str(boost::format("attaching %s to node %s")% mCenteredMeshes[meshName]->getName() % fullName));
                    Entity *ent = mSceneMgr->createEntity(fullName+" Ent", mCenteredMeshes[meshName]->getName());
                    newNode->attachObject(ent);
                }
            }

            m3dsBuildLog->logMessage("\n\n");

            _buildSceneFromNode(p->childs, newNode, fullName, _level+1);
        }
    }
}
//------------------------------------------------------------------------------