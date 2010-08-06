#include "precompiled.h"
#include "Test3DSViewerApp.h"

#include <string>
#include <sstream>
#include <boost/format.hpp>


static int  log_level = LIB3DS_LOG_INFO;


Matrix4 createMatrix4FromArray(float _mat[4][4])
{
    Matrix4 out;
    for(int i=0 ; i<4 ; i++)
        for(int j=0 ; j<4 ; j++)
            out[i][j] = _mat[i][j];

    return out;
}




Test3DSViewerApp::Test3DSViewerApp(void)
    :OgreApplication("3DS loader")
    ,mDummyCnt(0)
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
    _createGrid(1000);
    _createLight();


    _build3dsModel();
    //_buildRadiator(); 
}
//------------------------------------------------------------------------------
void Test3DSViewerApp::_build3dsModel()
{
    SceneNode *modelNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("3ds model");
 
    //m3dsFile =  lib3ds_file_open("../media/3ds/test3.3DS");
    //m3dsFile =  lib3ds_file_open("../media/3ds/indochine.3DS");
    ///m3dsFile =  lib3ds_file_open("../media/3ds/monaco.3DS");
    //m3dsFile =  lib3ds_file_open("../media/3ds/amphimath_walls.3DS");
    //m3dsFile =  lib3ds_file_open("../media/3ds/lyon.3DS");
    //m3dsFile =  lib3ds_file_open("../media/3ds/Kengresshus-visuelle.3DS");
    //m3dsFile =  lib3ds_file_open("../media/3ds/casa_de_musica-visuelle.3DS");
    //m3dsFile =  lib3ds_file_open("../media/3ds/Modern-home-interior1.3DS");
    m3dsFile =  lib3ds_file_open("../media/3ds/test.3DS");
    //m3dsFile =  lib3ds_file_open("../media/3ds/chienvert.3DS");
    if (!m3dsFile->nodes)
        lib3ds_file_create_nodes_for_meshes(m3dsFile);

    lib3ds_file_eval(m3dsFile, 0);

    mAABB = Ogre::AxisAlignedBox::BOX_NULL;

    /*_createMeshesFrom3dsFile(m3dsFile);
    _buildSceneFromNode(m3dsFile->nodes, modelNode, "/", 0, false);*/

    _buildSubtree( m3dsFile->nodes, "/", modelNode);




    Real width = mAABB.getSize()[0];
    Real scale = 1000.0/width;
    modelNode->scale(scale, scale, scale);
    modelNode->pitch(Degree(-90));

    StaticGeometry *geom = mSceneMgr->createStaticGeometry("fucking shit");
    geom->addSceneNode(modelNode);
    geom->build();
    geom->setVisible(true);
    modelNode->setVisible(false);


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

                    m3dsBuildLog->logMessage(boost::str(boost::format("attaching %s to node %s")
                                                            % newMesh->getName() % fullName));
                    Entity *ent = mSceneMgr->createEntity(fullName+" Ent", newMesh->getName());

                    newNode->attachObject(ent);

                    {
                        Ogre::Vector3 pos = ent->getParentSceneNode()->_getDerivedPosition();
                        Ogre::Vector3 scale = ent->getParentSceneNode()->_getDerivedScale();
                        Ogre::Quaternion rotation = ent->getParentSceneNode()->_getDerivedOrientation();

                        const Ogre::Vector3 * corners = ent->getBoundingBox().getAllCorners();

                        for (int i=0 ; i<8 ; i++)
                        {
                            mAABB.merge(pos + rotation * corners[i] * scale);
                        }
                    }
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
        newObject->begin("3DS/Gray", RenderOperation::OT_TRIANGLE_LIST);

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

    //restore mesh for future use
    memcpy(mesh->vertices, orig_vertices, sizeof(float) * 3 * mesh->nvertices);
    free(orig_vertices);

    
    MeshPtr newMesh;
    if(idx)
    {
        // create ogre mesh from manualobject
        newMesh = newObject->convertToMesh(fullMeshName + ".mesh");
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
        // TODO: better default material
        newObject->begin("3DS/Gray", RenderOperation::OT_TRIANGLE_LIST);

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
void Test3DSViewerApp::_buildSceneFromNode(Lib3dsNode *_3dsNode
                                           ,SceneNode *_parentNode
                                           ,const std::string &_basename
                                           ,int _level
                                           ,bool _show)
{
    
    boost::format fullNameFmt("%s/%06d%s");
  
    
    for(Lib3dsNode *p = _3dsNode ; p ; p=p->next)
    {
        SceneNode *newNode;
        std::stringstream spaces;
        for(int i=0 ; i<_level*4 ; ++i)
        {
            if ((i%4) == 0)
                spaces << ".";
            else
                spaces << " ";
        }
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
        
        Matrix4 baseMatrix = createMatrix4FromArray(p->matrix);
        _logXformMatrix(baseMatrix, spaces, "node->base.matrix : ", true);
    

        if (p->type == LIB3DS_NODE_MESH_INSTANCE) 
        {
            mNodeCnt++;
            fullNameFmt % _basename % p->node_id % p->name;
            std::string fullName = fullNameFmt.str();

            Lib3dsMeshInstanceNode *n = (Lib3dsMeshInstanceNode*) p;


            Vector3 pos, scl;
            Quaternion rot;
            scl = Vector3(n->scl[0], n->scl[1], n->scl[2]);
            pos = Vector3(n->pos[0], n->pos[1], n->pos[2]);
            rot = Quaternion(n->rot[3], n->rot[0], n->rot[1], n->rot[2]);

            if(!_show)
            {
                _show = (scl.x < 0 || scl.y < 0 || scl.z < 0 );            
            }


            newNode = _parentNode->createChildSceneNode(fullName + " Node");
                 

            {
                Matrix4 nodeMatrix = newNode->_getFullTransform();
                _logXformMatrix(nodeMatrix, spaces, "SceneNode before xform");
            
                Matrix4 localXform(Matrix4::IDENTITY);
                localXform.makeTransform(pos, scl, rot);
                _logXformMatrix(localXform, spaces, "mesh instance xform (scl*rot*pos)");
                
                Matrix4 finalXform = nodeMatrix * localXform;
                
                //_logXformMatrix(finalXform, spaces, "expected result xform ("
                //                                    +StringConverter::toString(finalXform == baseMatrix)
                //                                    +")");
                
            }

            newNode->scale(scl);

            {
                m3dsBuildLog->logMessage(spaces.str() + "    3dsMeshNode->scl : "
                    + StringConverter::toString(scl));

                Matrix4 xform(Matrix4::IDENTITY);
                xform.setScale(scl);
                _logXformMatrix(xform, spaces, "mesh instance scale");

                Matrix4 nodeMatrix = newNode->_getFullTransform();
                _logXformMatrix(nodeMatrix, spaces, "SceneNode after scale : ");
                m3dsBuildLog->logMessage("");
            }

            newNode->rotate(rot);

            {
                m3dsBuildLog->logMessage(spaces.str() + "    3dsMeshNode->rot : "
                                                      + StringConverter::toString(rot));

                Matrix4 xform(rot);
                _logXformMatrix(xform, spaces, "mesh instance rotation");
                Matrix4 nodeMatrix = newNode->_getFullTransform();
                _logXformMatrix(nodeMatrix, spaces, "SceneNode after scale & rotate : ");
                m3dsBuildLog->logMessage("");
            }

            newNode->translate(pos);
            
            {
                m3dsBuildLog->logMessage(spaces.str() + "    3dsMeshNode->pos : "
                                                      + StringConverter::toString(pos));

                Matrix4 xform(Matrix4::IDENTITY);
                xform.makeTrans(pos);
                _logXformMatrix(xform, spaces, "mesh instance translation");

                Matrix4 nodeMatrix = newNode->_getFullTransform();
                _logXformMatrix(nodeMatrix, spaces, "SceneNode after translate");
                m3dsBuildLog->logMessage("");
            }

            newNode = newNode->createChildSceneNode(fullName+"pivot node"
                                                    , Vector3(-n->pivot[0]
                                                            , -n->pivot[1]
                                                            , -n->pivot[2]));
            newNode->setVisible(!(bool)n->hide);
            {
                // log node xforms
                m3dsBuildLog->logMessage(spaces.str() + "    3dsMeshNode->pivot : "
                    + StringConverter::toString(Vector3(-n->pivot[0]
                                                        ,-n->pivot[1]
                                                        ,-n->pivot[2])));
                Matrix4 nodeMatrix = newNode->_getFullTransform();
                _logXformMatrix(nodeMatrix, spaces, "SceneNode after pivot");
                m3dsBuildLog->logMessage("");
            }


            m3dsBuildLog->logMessage("");
            Lib3dsMesh *mesh = lib3ds_file_mesh_for_node(m3dsFile, (Lib3dsNode*)n);
            if (mesh && mesh->name)
            {
                std::string meshName = mesh->name;
                
                Matrix4 meshMatrix = createMatrix4FromArray(mesh->matrix);
                _logXformMatrix(meshMatrix, spaces, "mesh matrix : ", true);

                MeshPtr meshToAdd = mCenteredMeshes[meshName];

                if(! meshToAdd.isNull())
                {
                    if(1)//_show)
                    {

                        Entity *ent = mSceneMgr->createEntity(fullName+" Ent"
                                                            , mCenteredMeshes[meshName]->getName());
                        newNode->attachObject(ent);

                        {
                            Ogre::Vector3 pos = ent->getParentSceneNode()->_getDerivedPosition();
                            Ogre::Vector3 scale = ent->getParentSceneNode()->_getDerivedScale();
                            Ogre::Quaternion rotation = ent->getParentSceneNode()->_getDerivedOrientation();

                            const Ogre::Vector3 * corners = ent->getBoundingBox().getAllCorners();

                            for (int i=0 ; i<8 ; i++)
                            {
                                mAABB.merge(pos + rotation * corners[i] * scale);
                            }
                        }
                    }
                }
            }

            m3dsBuildLog->logMessage("\n\n\n");
            
           
            _buildSceneFromNode(p->childs, newNode, fullName, _level+1, _show);
        }
    }
}
//------------------------------------------------------------------------------
void Test3DSViewerApp::_logXformMatrix(const Matrix4 &_matrix
                                      ,const std::stringstream &_spaces
                                      ,const std::string &_title
                                      ,bool _transpose)
{
    m3dsBuildLog->logMessage("");
    m3dsBuildLog->logMessage(_spaces.str() + "    "+_title);

    for(int i=0 ; i<4 ; ++i)
    {
        boost::format matrixLineFmt("%s%s %+.2f   %+.2f   %+.2f   %+.2f");
        matrixLineFmt % _spaces.str() % "    ";
        if(_transpose)
            matrixLineFmt % _matrix[0][i] % _matrix[1][i] % _matrix[2][i] % _matrix[3][i];
        else
            matrixLineFmt % _matrix[i][0] % _matrix[i][1] % _matrix[i][2] % _matrix[i][3];
        m3dsBuildLog->logMessage(matrixLineFmt.str());
    }
    m3dsBuildLog->logMessage("");
}
//------------------------------------------------------------------------------
void Test3DSViewerApp::_buildRadiator()
{
    mSceneMgr->setFlipCullingOnNegativeScale(false);

    SceneNode *baseNode, *box210Node, *loft394Node;
    Entity *box210Ent, *loft394Ent;

    Vector3 baseScl, basePos, basePivot;
    Vector3 box210Scl, box210Pos, box210Pivot;
    Vector3 loft394Scl, loft394Pos, loft394Pivot;
    Quaternion baseRot, box210Rot, loft394Rot;


    baseNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("base node");
    box210Node = baseNode->createChildSceneNode("Box210 node");
    loft394Node = baseNode->createChildSceneNode("Loft394 node");

    box210Ent = mSceneMgr->createEntity("Box210", "Box210.mesh");
    loft394Ent = mSceneMgr->createEntity("Loft394", "Loft394.mesh");

    box210Node->attachObject(box210Ent);
    loft394Node->attachObject(loft394Ent);


    box210Scl = Vector3(1, 1, 1);
    box210Rot = Quaternion(0.5, 0.5, -0.5, -0.5);
    box210Pos = Vector3(107.256, 1.29345, 17.5695);
    
    box210Node->scale(box210Scl);
    box210Node->rotate(box210Rot);
    box210Node->translate(box210Pos);



    loft394Scl = Vector3(1, 1.44587, 0.891658);
    loft394Rot = Quaternion( 0.707107, 0.707107, -0, -0);
    loft394Pos = Vector3(-0.83252, 1.06597, 9.29529);

    loft394Node->scale(loft394Scl);
    loft394Node->rotate(loft394Rot);
    loft394Node->translate(loft394Pos);



    baseScl = Vector3(-1, -0.75888, 0.75888);
    baseRot = Quaternion( -0.139173, -0, -0, -0.990268);
    //basePos = Vector3(-1062.25, -1199.29, -775.482);
    basePos = Vector3(0, 0, 0);

    //baseNode->scale(baseScl);
    baseNode->rotate(baseRot);
    //baseNode->translate(basePos);


    Matrix4 boxMatrix, loftMatrix, baseMatrix;
    
    boxMatrix = box210Node->_getFullTransform();
    loftMatrix = loft394Node->_getFullTransform();
    baseMatrix = baseNode->_getFullTransform();

    Matrix4 baseMatrix2, loftMatrix2, boxMatrix2, M;

    baseMatrix2.makeTransform(basePos, baseScl, baseRot);
    M = Matrix4::IDENTITY;
    M.makeTransform(loft394Pos, loft394Scl, loft394Rot);
    loftMatrix2 = baseMatrix2 * M;
    M = Matrix4::IDENTITY;
    M.makeTransform(box210Pos, box210Scl, box210Rot);
    boxMatrix2 = baseMatrix2 * M;
}