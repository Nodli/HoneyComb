////////////////////////////////////////////////////////////////////////////////
//                   This file is part of VistaStars                          //
//             Copyright: (c) German Aerospace Center (DLR)                   //
////////////////////////////////////////////////////////////////////////////////

#include "VistaStars.hpp"

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

#include <VistaInterProcComm/Connections/VistaByteBufferDeSerializer.h>
#include <VistaInterProcComm/Connections/VistaByteBufferSerializer.h>
#include <VistaKernel/DisplayManager/VistaDisplayManager.h>
#include <VistaKernel/DisplayManager/VistaProjection.h>
#include <VistaKernel/DisplayManager/VistaViewport.h>
#include <VistaKernel/GraphicsManager/VistaGeometryFactory.h>
#include <VistaKernel/GraphicsManager/VistaOpenGLNode.h>
#include <VistaKernel/GraphicsManager/VistaSceneGraph.h>
#include <VistaKernel/VistaSystem.h>
#include <VistaMath/VistaBoundingBox.h>
#include <VistaOGLExt/VistaBufferObject.h>
#include <VistaOGLExt/VistaGLSLShader.h>
#include <VistaOGLExt/VistaOGLUtils.h>
#include <VistaOGLExt/VistaShaderRegistry.h>
#include <VistaOGLExt/VistaTexture.h>
#include <VistaOGLExt/VistaVertexArrayObject.h>
#include <VistaTools/tinyXML/tinyxml.h>

#include <GL/gl.h>

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

#include <fstream>
#include <limits>
#include <cstdint>
#include <utility>
#include <cmath>
#include <algorithm>

/*============================================================================*/
/*  IMPORT UTILITY FUNCTIONS                                                  */
/*============================================================================*/

namespace
{
    template <typename T> bool FromString(std::string const& v, T& out)
    {
        std::istringstream iss(v);
        iss >> out;
        return (iss.rdstate() & std::stringstream::failbit) == 0;
    }

    template <typename T> T getBytes(std::ifstream& stream)
    {
        T     val       = 0;
        char* val_bytes = (char*)(&val);

        stream.read(val_bytes, sizeof(T));

        return val;
    }
}

/*============================================================================*/
/*  CONSTRUCTORS / DESTRUCTOR                                                 */
/*============================================================================*/

VistaStars::VistaStars(const std::string& cacheFolderPath, const uint32_t maxGPUStarsInMbytes)
{
    std::cout << "[VistaStars] Opening the cache from path " << cacheFolderPath << std::endl;

    m_maxGPUAllocation = maxGPUStarsInMbytes * 1000000u; // in bytes

    GLint maxSSBOBytes;
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxSSBOBytes); // in bytes
        std::cout << "GL_MAX_SHADER_STORAGE_BLOCK_SIZE is " << maxSSBOBytes << " bytes." << std::endl;

    if(m_maxGPUAllocation > maxSSBOBytes){
        std::cout << "[VistaStars] User asked for a buffer size of " << m_maxGPUAllocation << " bytes but GL_MAX_SHADER_STORAGE_BLOCK_SIZE is " << maxSSBOBytes << " bytes" << std::endl;
        std::cout << "[VistaStars] Limiting the allocation to " << maxSSBOBytes<< " bytes" << std::endl;

        m_maxGPUAllocation = maxSSBOBytes;
    }

    Init(cacheFolderPath);
}

VistaStars::~VistaStars()
{
    delete m_pPixelQueryShader;
    delete m_pStarOutlierShader;
    delete m_pStarShader;
    delete m_pBackgroundShader;
    delete m_pBoxShader;

    delete m_pBackgroundTexture1;
    delete m_pBackgroundTexture2;
    delete m_pBackgroundVAO;
    delete m_pBackgroundVBO;
    delete m_pBoxVAO;
    delete m_pBoxIBO;
    delete m_pPixelQueryVAO;
    delete m_pPixelQueryIBO;

    delete m_pStarOutlierVBO;
    delete m_pStarOutlierVAO;
    delete m_pAccumulatedSSO;
    delete m_pStarSSO;
    delete m_pEmptyVAO;

    PurgeTree();
    PurgeQueries();
}

/*============================================================================*/
/*  IMPLEMENTATION                                                            */
/*============================================================================*/

void VistaStars::SetDrawMode(VistaStars::DrawMode eValue)
{
    if (eValue != m_eDrawMode)
    {
        m_eDrawMode    = eValue;
        m_bShaderDirty = true;
    }
}

VistaStars::DrawMode VistaStars::GetDrawMode() const
{
    return m_eDrawMode;
}

void VistaStars::SetSolidAngle(float fValue)
{
    m_fSolidAngle = fValue;
}

float VistaStars::GetSolidAngle() const
{
    return m_fSolidAngle;
}

void VistaStars::SetLuminanceMultiplicator(float fValue)
{
    m_fLuminanceMultiplicator = fValue;
}

float VistaStars::GetLuminanceMultiplicator() const
{
    return m_fLuminanceMultiplicator;
}

void VistaStars::SetEnableBoundingBoxes(bool bValue)
{
    m_bEnableBoundingBoxes = bValue;
}

bool VistaStars::GetEnableBoundingBoxes() const
{
    return m_bEnableBoundingBoxes;
}

void VistaStars::SetEnableStarsDisplay(bool bValue)
{
    m_bEnableStarsDisplay = bValue;
}

bool VistaStars::GetEnableStarsDisplay() const
{
    return m_bEnableStarsDisplay;
}

void VistaStars::SetPercentBudgetCut(float fValue){
    m_fPercentBudgetCut = fValue;

    unsigned int newTotalGPUBudget = m_fPercentBudgetCut * m_gpuBlocks;

    // always keeping at least one node for the root node
    if(newTotalGPUBudget < 1){
        newTotalGPUBudget = 1;
    }

    m_Cut.currentGPUBudget = m_Cut.currentGPUBudget - (m_Cut.totalGPUBudget - newTotalGPUBudget);
    m_Cut.totalGPUBudget = newTotalGPUBudget;
}

float VistaStars::GetPercentBudgetCut() const{
    return m_fPercentBudgetCut;
}

void VistaStars::SetEnableFreezeCut(bool bValue){
    m_bEnableFreezeCut = bValue;
}

bool VistaStars::GetEnableFreezeCut() const{
    return m_bEnableFreezeCut;
}

void VistaStars::SetBackgroundColor1(const VistaColor& cValue)
{
    m_cBackgroundColor1 = cValue;
}

const VistaColor& VistaStars::GetBackgroundColor1() const
{
    return m_cBackgroundColor1;
}

void VistaStars::SetBackgroundColor2(const VistaColor& cValue)
{
    m_cBackgroundColor2 = cValue;
}

const VistaColor& VistaStars::GetBackgroundColor2() const
{
    return m_cBackgroundColor2;
}

void VistaStars::SetBackgroundTexture1(const std::string& sFilename)
{
    if (m_pBackgroundTexture1)
    {
        delete m_pBackgroundTexture1;
        m_pBackgroundTexture1 = NULL;
    }

    if (sFilename != "")
    {
        m_pBackgroundTexture1 = VistaOGLUtils::LoadTextureFromTga(sFilename, false);
    }
}

void VistaStars::SetBackgroundTexture2(const std::string& sFilename)
{
    if (m_pBackgroundTexture2)
    {
        delete m_pBackgroundTexture2;
        m_pBackgroundTexture2 = NULL;
    }

    if (sFilename != "")
    {
        m_pBackgroundTexture2 = VistaOGLUtils::LoadTextureFromTga(sFilename, false);
    }
}

bool VistaStars::Do()
{

    if (m_bShaderDirty)
    {
        UpdateShader();
        m_bShaderDirty = false;
    }

    // save current state of the OpenGL state machine
    glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT);
    glDepthMask(GL_FALSE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    // get matrices ------------------------------------------------------------
    GLfloat glMat[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, &glMat[0]);
    VistaTransformMatrix matModelView(glMat, true);

    glGetFloatv(GL_PROJECTION_MATRIX, &glMat[0]);
    VistaTransformMatrix matProjection(glMat, true);

    VistaTransformMatrix matModelViewProjection = matProjection * matModelView;

    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    VistaTransformMatrix matInverseMV(matModelView.GetInverted());
    VistaTransformMatrix matInverseP(matProjection.GetInverted());

    // draw background ---------------------------------------------------------
    if ((m_pBackgroundTexture1 && m_cBackgroundColor1[3] != 0.f)
        || (m_pBackgroundTexture2 && m_cBackgroundColor2[3] != 0.f))
    {
        m_pBackgroundVAO->Bind();
        m_pBackgroundShader->Bind();
        m_pBackgroundShader->SetUniform(m_pBackgroundShader->GetUniformLocation("iTexture"), 0);

        VistaTransformMatrix matMVNoTranslation = matModelView;

        // reduce jitter
        matMVNoTranslation[0][3] = 0.f;
        matMVNoTranslation[1][3] = 0.f;
        matMVNoTranslation[2][3] = 0.f;

        VistaTransformMatrix matMVP(matProjection * matMVNoTranslation);
        VistaTransformMatrix matInverseMVP(matMVP.GetInverted());
        VistaTransformMatrix matInverseMV(matMVNoTranslation.GetInverted());

        GLint loc = m_pBackgroundShader->GetUniformLocation("uInvMVP");
        glUniformMatrix4fv(loc, 1, GL_FALSE, matInverseMVP.GetData());

        loc = m_pBackgroundShader->GetUniformLocation("uInvMV");
        glUniformMatrix4fv(loc, 1, GL_FALSE, matInverseMV.GetData());

        if (m_pBackgroundTexture1 && m_cBackgroundColor1[3] != 0.f)
        {
            m_pBackgroundShader->SetUniform(m_pBackgroundShader->GetUniformLocation("cColor"),
                                            m_cBackgroundColor1[0], m_cBackgroundColor1[1],
                                            m_cBackgroundColor1[2], m_cBackgroundColor1[3]);
            m_pBackgroundTexture1->Bind(GL_TEXTURE0);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            m_pBackgroundTexture1->Unbind(GL_TEXTURE0);
        }

        if (m_pBackgroundTexture2 && m_cBackgroundColor2[3] != 0.f)
        {
            m_pBackgroundShader->SetUniform(m_pBackgroundShader->GetUniformLocation("cColor"),
                                            m_cBackgroundColor2[0], m_cBackgroundColor2[1],
                                            m_cBackgroundColor2[2], m_cBackgroundColor2[3]);
            m_pBackgroundTexture2->Bind(GL_TEXTURE0);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            m_pBackgroundTexture2->Unbind(GL_TEXTURE0);
        }

        m_pBackgroundShader->Release();
        m_pBackgroundVAO->Release();
    }

    // computing draw parameters -----------------------------------------------
    // camera / frustum values
    VistaVector3D posCamera = {0, 0, 0};
    constexpr float parsecToMeter = 3.08567758e16;
    posCamera = matInverseMV * posCamera / parsecToMeter;

    constexpr bool displayRenderData = false;

    // draw octree boundaries --------------------------------------------------
    // the tree has to be traversed one more time ie performance hit -----------
    if(m_bEnableBoundingBoxes){
        unsigned int boxRenderCount = 0;

        m_pBoxVAO->Bind();
        m_pBoxIBO->Bind(GL_ELEMENT_ARRAY_BUFFER);
        m_pBoxShader->Bind();

        GLint boundaryMinLoc = m_pBoxShader->GetUniformLocation("boundaryMin");
        GLint boundaryMaxLoc = m_pBoxShader->GetUniformLocation("boundaryMax");
        GLint uMatMVLoc = m_pBoxShader->GetUniformLocation("uMatMV");
        GLint uMatPLoc = m_pBoxShader->GetUniformLocation("uMatP");

        GLint iLevelLoc = m_pBoxShader->GetUniformLocation("nodeID");
        GLint nLevelsLoc = m_pBoxShader->GetUniformLocation("nNodes");

        glUniformMatrix4fv(uMatMVLoc, 1, GL_FALSE, matModelView.GetData());
        glUniformMatrix4fv(uMatPLoc, 1, GL_FALSE, matProjection.GetData());

        glUniform1i(nLevelsLoc, m_Octree.maxDepth);

        std::deque<std::pair<unsigned int, Node*>> renderQueue; // level of the node, pointer to the node
        renderQueue.push_back({0, m_Octree.root});

        // when using offBoxes, it is normal that the span of one box appear
        // smaller than the span of all its children this is because we use an
        // offsetFactor which will be higher for bigger boxes ie the parent and
        // lower for smaller boxes ie the children
        auto drawBox = [this, boundaryMinLoc, boundaryMaxLoc, iLevelLoc](const int nodeLevel, const PrecisionBox& boundaries){

            constexpr bool offsetBoxes = true;
            constexpr float offsetFactor = 0.1;

            if(offsetBoxes){
                PreciseVec3 diagonal = {boundaries.max[0] - boundaries.min[0],
                    boundaries.max[1] - boundaries.min[1],
                    boundaries.max[2] - boundaries.min[2]};

                glUniform3f(boundaryMinLoc,
                        (float)(boundaries.min[0] + offsetFactor * diagonal.mX),
                        (float)(boundaries.min[1] + offsetFactor * diagonal.mY),
                        (float)(boundaries.min[2] + offsetFactor * diagonal.mZ));
                glUniform3f(boundaryMaxLoc,
                        (float)(boundaries.max[0] - offsetFactor * diagonal.mX),
                        (float)(boundaries.max[1] - offsetFactor * diagonal.mY),
                        (float)(boundaries.max[2] - offsetFactor * diagonal.mZ));
            }else{
                glUniform3f(boundaryMinLoc,
                        (float)boundaries.min[0],
                        (float)boundaries.min[1],
                        (float)boundaries.min[2]);
                glUniform3f(boundaryMaxLoc,
                        (float)boundaries.max[0],
                        (float)boundaries.max[1],
                        (float)boundaries.max[2]);

            }

            glUniform1i(iLevelLoc, nodeLevel);

            constexpr unsigned int nIndicesBox = 24;
            glDrawElements(GL_LINES, nIndicesBox, GL_UNSIGNED_INT, nullptr);
        };

        auto enqueueChildren = [&renderQueue](){
            for(unsigned int iChild = 0; iChild != renderQueue.front().second->nChildren; ++iChild){
                renderQueue.push_back({renderQueue.front().first + 1, renderQueue.front().second->children[iChild]});
            }
        };

        // 0 : draw nodes inside the cut only
        // 1 : draw nodes that are rendered
        // 2 : draw nodes up to those that are rendered
        // 3 : draw all nodes
        constexpr int drawMode = 1;

        if(drawMode == 0){
            for(auto& node : m_Cut.nodes){
                drawBox(node.first, node.second->boundaries);
                ++boxRenderCount;
            }

        }else if(drawMode == 1){

            while(!renderQueue.empty()){
                // leaf node
                if(renderQueue.front().second->isLeaf()){
                    drawBox(renderQueue.front().first, renderQueue.front().second->boundaries);
                    ++boxRenderCount;

                    // children are also above cut
                }else if(renderQueue.front().second->children[0]->aboveCut){
                    enqueueChildren();

                    // children are not above cut
                }else{
                    drawBox(renderQueue.front().first, renderQueue.front().second->boundaries);
                    ++boxRenderCount;

                }

                renderQueue.pop_front();
            }

        }else if(drawMode == 2){
            while(!renderQueue.empty()){
                if(renderQueue.front().second->aboveCut){
                    drawBox(renderQueue.front().first, renderQueue.front().second->boundaries);
                    ++boxRenderCount;

                    enqueueChildren();
                }

                renderQueue.pop_front();
            }

        }else if(drawMode == 3){
            while(!renderQueue.empty()){
                if(isBoxVisible(renderQueue.front().second->boundaries, matModelViewProjection)){
                    // always draw a visible box
                    // (this is different from the impostors)
                    drawBox(renderQueue.front().first, renderQueue.front().second->boundaries);
                    ++boxRenderCount;

                    // traversal
                    if(isPositionInsideBox(posCamera, renderQueue.front().second->boundaries)){
                        enqueueChildren();

                    }else{ // need to check for solid angle culling
                        VistaVector3D boxCenter = {(float)(renderQueue.front().second->boundaries.min[0] + renderQueue.front().second->boundaries.max[0]) / 2,
                            (float)(renderQueue.front().second->boundaries.min[1] + renderQueue.front().second->boundaries.max[1]) / 2,
                            (float)(renderQueue.front().second->boundaries.min[2] + renderQueue.front().second->boundaries.max[2]) / 2};
                        VistaVector3D cameraToBox = {boxCenter[0] - posCamera[0],
                            boxCenter[1] - posCamera[1],
                            boxCenter[2] - posCamera[2]};
                        float boxCameraDistance = cameraToBox.GetLength();

                        enqueueChildren();
                    }

                }
                renderQueue.pop_front();
            }
        }

        m_pBoxShader->Release();
        m_pBoxIBO->Release();
        m_pBoxVAO->Release();

        std::cout << "DRAWING " << boxRenderCount << " bounding boxes" << std::endl;
    }


    // draw star nodes / impostors ---------------------------------------------
    if(m_bEnableStarsDisplay){
        // draw outlier stars --------------------------------------------------
        if(m_Octree.nOutliers > 0){
            m_pStarOutlierShader->Bind();

            if (m_eDrawMode == DRAWMODE_POINT || m_eDrawMode == DRAWMODE_SMOOTH_POINT)
            {
                glPointSize(0.5f);
            }

            if (m_eDrawMode == DRAWMODE_SMOOTH_POINT)
            {
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_POINT_SMOOTH);
                glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
            }
            else
            {
                glDisable(GL_POINT_SMOOTH);
            }

            m_pStarOutlierShader->SetUniform(m_pStarOutlierShader->GetUniformLocation("uResolution"), viewport[2],
                    viewport[3]);

            const GLint solidAngleLocation = m_pStarOutlierShader->GetUniformLocation("uSolidAngle");
            m_pStarOutlierShader->SetUniform(solidAngleLocation, m_fSolidAngle);

            m_pStarOutlierShader->SetUniform(m_pStarOutlierShader->GetUniformLocation("uLuminanceMultiplicator"),
                    m_fLuminanceMultiplicator);

            GLint loc = m_pStarOutlierShader->GetUniformLocation("uMatMV");
            glUniformMatrix4fv(loc, 1, GL_FALSE, matModelView.GetData());

            loc = m_pStarOutlierShader->GetUniformLocation("uMatP");
            glUniformMatrix4fv(loc, 1, GL_FALSE, matProjection.GetData());

            loc = m_pStarOutlierShader->GetUniformLocation("uInvMV");
            glUniformMatrix4fv(loc, 1, GL_FALSE, matInverseMV.GetData());

            loc = m_pStarOutlierShader->GetUniformLocation("uInvP");
            glUniformMatrix4fv(loc, 1, GL_FALSE, matInverseP.GetData());

            m_pStarOutlierVAO->Bind();

            glDrawArrays(GL_POINTS, 0, m_Octree.nOutliers);

            GLenum errorCode = glGetError();
            if(errorCode){
                std::cout << "error code after drawing outliers : " << errorCode << std::endl;
            }

            m_pStarOutlierVAO->Release();
            m_pStarOutlierShader->Release();

            std::cout << "DRAWING " << m_Octree.nOutliers << " outlier stars" << std::endl;
        }

        // update octree cut and data ------------------------------------------
        {
            if(!m_bEnableFreezeCut){
                constexpr bool useQueries = true;
                if(useQueries){
                    RetrieveGPUQueries();
                    UpdateCutWithQueries(posCamera);
                    IssueGPUQueries(posCamera, matModelViewProjection, viewport[2] * viewport[3]);

                }else{
                    #if 0
                    // ALERT ! This is legacy code and does not work properly when reducing the cut budget at runtime
                    //UpdateCut(posCamera, matModelViewProjection);
                    #endif

                }

                UpdateGPUBlocks();
            }
        }

        // draw everything
        {
            m_pStarShader->Bind();

            if (m_eDrawMode == DRAWMODE_POINT || m_eDrawMode == DRAWMODE_SMOOTH_POINT)
            {
                glPointSize(0.5f);
            }

            if (m_eDrawMode == DRAWMODE_SMOOTH_POINT)
            {
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_POINT_SMOOTH);
                glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
            }
            else
            {
                glDisable(GL_POINT_SMOOTH);
            }

            m_pStarShader->SetUniform(m_pStarShader->GetUniformLocation("uResolution"), viewport[2],
                    viewport[3]);

            const GLint solidAngleLocation = m_pStarShader->GetUniformLocation("uSolidAngle");
            m_pStarShader->SetUniform(solidAngleLocation, m_fSolidAngle);

            m_pStarShader->SetUniform(m_pStarShader->GetUniformLocation("uLuminanceMultiplicator"),
                    m_fLuminanceMultiplicator);

            GLint loc = m_pStarShader->GetUniformLocation("uMatMV");
            glUniformMatrix4fv(loc, 1, GL_FALSE, matModelView.GetData());

            loc = m_pStarShader->GetUniformLocation("uMatP");
            glUniformMatrix4fv(loc, 1, GL_FALSE, matProjection.GetData());

            loc = m_pStarShader->GetUniformLocation("uInvMV");
            glUniformMatrix4fv(loc, 1, GL_FALSE, matInverseMV.GetData());

            loc = m_pStarShader->GetUniformLocation("uInvP");
            glUniformMatrix4fv(loc, 1, GL_FALSE, matInverseP.GetData());

            loc = m_pStarShader->GetUniformLocation("uStarsPerBlock");
            glUniform1ui(loc, m_Octree.nStarsPerNode);

            m_pEmptyVAO->Bind();

            constexpr unsigned int accumulatedBinding = 0;
            constexpr unsigned int starDataBinding = 1;

            m_pAccumulatedSSO->Bind(GL_SHADER_STORAGE_BUFFER);
            m_pAccumulatedSSO->BindBufferBase(GL_SHADER_STORAGE_BUFFER, accumulatedBinding);

            m_pStarSSO->Bind(GL_SHADER_STORAGE_BUFFER);
            m_pStarSSO->BindBufferBase(GL_SHADER_STORAGE_BUFFER, starDataBinding);

            glDrawArrays(GL_POINTS, 0, m_gpuAccumulated[m_gpuAccumulated.size() - 1]);

            GLenum errorCode = glGetError();
            if(errorCode){
                std::cout << "error code after drawing node stars : " << errorCode << std::endl;
            }

            m_pEmptyVAO->Release();
            m_pStarShader->Release();

            std::cout << "CUT CURRENT BUDGET " << m_Cut.currentGPUBudget << " out of " << m_Cut.totalGPUBudget << std::endl;
            std::cout << "DRAWING " << m_gpuAccumulated[m_gpuAccumulated.size() - 1] << " stars from " << m_Cut.nodes.size() << " nodes" << std::endl;

            unsigned int inUseBlocks = 0;
            for(auto& block : m_gpuBlockBuffer){
                if(block.node != nullptr && block.node->aboveCut){
                    ++inUseBlocks;
                }
            }
            std::cout << "IN USE " << inUseBlocks << " blocks" << std::endl;
        }

    }

    glDepthMask(GL_TRUE);
    glPopAttrib();

    return true;
}

bool VistaStars::GetBoundingBox(VistaBoundingBox& oBoundingBox)
{
    float min(std::numeric_limits<float>::min());
    float max(std::numeric_limits<float>::max());
    float fMin[3] = { min, min, min };
    float fMax[3] = { max, max, max };

    oBoundingBox.SetBounds(fMin, fMax);

    return true;
}

bool VistaStars::Node::isLeaf(){
    return (nChildren == 0);
}

VistaStars::LoadingThread::LoadingThread(){
    // https://en.cppreference.com/w/cpp/thread/condition_variable
    // Any thread that intends to wait on std::condition_variable has to
    //    acquire a std::unique_lock<std::mutex>, on the same mutex as used to
    //      protect the shared variable
    //    execute wait, wait_for, or wait_until. The wait operations atomically
    //      release the mutex and suspend the execution of the thread.
    //    When the condition variable is notified, a timeout expires, or a
    //      spurious wakeup occurs, the thread is awakened, and the mutex is
    //      atomically reacquired. The thread should then check the condition
    //      and resume waiting if the wake up was spurious.

    auto threadLoop = [this](){
        while(true){
            std::unique_lock<std::mutex> lock(mutex);
            while(!stop && nodesToCheck.empty()){
                condition.wait(lock);
            }

            isWorking.lock();

            // thread return condition
            if(stop && nodesToCheck.empty()){
                return;
            }

            Node* nodeToLoad = nullptr;

#if 0
            // take ownership of the first node of the queue we can lock ie remove from queue
            // unlock the queue
            for(std::deque<Node*>::iterator itNode = nodesToCheck.begin();
                    nodeToLoad == nullptr && itNode != nodesToCheck.end();
                    ++itNode){

                if((*itNode)->mutex.try_lock()){
                    nodeToLoad = *itNode;
                    nodesToCheck.erase(itNode);
                }
            }

            lock.unlock();

            // if we could lock a node
            // load data from disk to main memory if necessary
            // unlock the node
            if(nodeToLoad != nullptr){
                if(nodeToLoad->aboveCut && !nodeToLoad->loadedCache){

                    std::cout << "LOADING node " << nodeToLoad->name << std::endl;

                    nodeToLoad->cache.reserve(nodeToLoad->nStars * 7);
                    binaryFile.seekg(nodeToLoad->starByteOffset);

                    for(unsigned int iStar = 0; iStar != nodeToLoad->nStars; ++iStar){
                        nodeToLoad->cache.push_back(getBytes<float>(binaryFile)); // mPosition.mX
                        nodeToLoad->cache.push_back(getBytes<float>(binaryFile)); // mPosition.mY
                        nodeToLoad->cache.push_back(getBytes<float>(binaryFile)); // mPosition.mZ

                        nodeToLoad->cache.push_back(getBytes<float>(binaryFile)); // mColor.mR
                        nodeToLoad->cache.push_back(getBytes<float>(binaryFile)); // mColor.mG
                        nodeToLoad->cache.push_back(getBytes<float>(binaryFile)); // mColor.mB

                        nodeToLoad->cache.push_back(getBytes<float>(binaryFile)); // mAbsoluteMagnitude
                    }

                    nodeToLoad->loadedCache = true;

                    std::cout << "DONE LOADING node " << nodeToLoad->name << " with nStars " << nodeToLoad->nStars << std::endl;
                }

                nodeToLoad->mutex.unlock();
            }

#else
            nodeToLoad = nodesToCheck.front();
            nodesToCheck.pop_front();
            lock.unlock();

            if(nodeToLoad->aboveCut && !nodeToLoad->loadedCache){

                std::cout << "LOADING node " << nodeToLoad->name << std::endl;

                std::vector<float> tempCache;
                tempCache.reserve(nodeToLoad->nStars * 7);
                binaryFile.seekg(nodeToLoad->starByteOffset);

                for(unsigned int iStar = 0; iStar != nodeToLoad->nStars; ++iStar){
                    tempCache.push_back(getBytes<float>(binaryFile)); // mPosition.mX
                    tempCache.push_back(getBytes<float>(binaryFile)); // mPosition.mY
                    tempCache.push_back(getBytes<float>(binaryFile)); // mPosition.mZ

                    tempCache.push_back(getBytes<float>(binaryFile)); // mColor.mR
                    tempCache.push_back(getBytes<float>(binaryFile)); // mColor.mG
                    tempCache.push_back(getBytes<float>(binaryFile)); // mColor.mB

                    tempCache.push_back(getBytes<float>(binaryFile)); // mAbsoluteMagnitude
                }

                nodeToLoad->mutex.lock();
                if(nodeToLoad->aboveCut){
                    std::swap(nodeToLoad->cache, tempCache);
                    nodeToLoad->loadedCache = true;
                }
                nodeToLoad->mutex.unlock();

                std::cout << "DONE LOADING node " << nodeToLoad->name << " with nStars " << nodeToLoad->nStars << std::endl;
            }
#endif


            isWorking.unlock();
        }
    };

    thread = std::thread(threadLoop);
}

VistaStars::LoadingThread::~LoadingThread(){
    stop = true;
    condition.notify_all();
    thread.join();
}

bool VistaStars::LoadingThread::initStream(const std::string& filePath){
    binaryFile.open(filePath);

    return binaryFile.is_open();
}

void VistaStars::LoadingThread::addNode(Node* node){

    mutex.lock();
    nodesToCheck.push_back(node);
    mutex.unlock();

    condition.notify_one();
}

// https://en.cppreference.com/w/cpp/thread/condition_variable
// The thread that intends to modify the variable has to
//    acquire a std::mutex (typically via std::lock_guard)
//    perform the modification while the lock is held
//    execute notify_one or notify_all on the std::condition_variable (the lock does not need to be held for notification)
void VistaStars::LoadingThread::addNodes(const std::vector<Node*>& nodes){
    mutex.lock();

    // add the nodes
    for(auto& n : nodes){
        nodesToCheck.push_back(n);
    }

    mutex.unlock();
    condition.notify_all();
}

void VistaStars::LoadingThread::addNodes(const std::vector<std::pair<unsigned int, Node*>>& nodes){
    mutex.lock();

    // add the nodes
    for(auto& n : nodes){
        nodesToCheck.push_back(n.second);
    }

    mutex.unlock();
    condition.notify_all();
}

void VistaStars::LoadingThread::synchronize(){
    auto syncFunction = [this](){

        bool emptyNodesToCheck = false;
        bool idleThread = false;

        if(emptyNodesToCheck = mutex.try_lock()){
            emptyNodesToCheck = nodesToCheck.empty();
            mutex.unlock();

            if(idleThread = isWorking.try_lock()){
                isWorking.unlock();
            }
        }

        return emptyNodesToCheck && idleThread;
    };

    std::cout << "starting to synchronize" << std::endl;

    while(!syncFunction()){
    }

    std::cout << "finished synchronization" << std::endl;
}

bool VistaStars::isPositionInsideBox(const VistaVector3D& position, const Box& box){
    if(position[0] < box.min[0] || position[0] > box.max[0]
    || position[1] < box.min[1] || position[1] > box.max[1]
    || position[2] < box.min[2] || position[2] > box.max[2]){
        return false;
    }

    return true;
}

bool VistaStars::isPositionInsideBox(const VistaVector3D& position, const PrecisionBox& box){
    if(position[0] < box.min[0] || position[0] > box.max[0]
    || position[1] < box.min[1] || position[1] > box.max[1]
    || position[2] < box.min[2] || position[2] > box.max[2]){
        return false;
    }

    return true;
}

bool VistaStars::isBoxVisible(const Box& box, const VistaTransformMatrix& matMVP){
    int axis[3] = {0, 0, 0};

    const float parsecToMeter = 3.08567758e16;

    // visibility test in projected space
    // checks if the projected box intersects the AA frustum box in projected space
    for(unsigned int iCorner = 0; iCorner != 8; ++iCorner){

        // generate bbox corners in worldspace coordinates
        VistaVector3D corner = {(float)box.min[0], (float)box.min[1], (float)box.min[2]};
        if(iCorner & 1u){
            corner[0] = box.max[0];
        }
        if(iCorner & 2u){
            corner[1] = box.max[1];
        }
        if(iCorner & 4u){
            corner[2] = box.max[2];
        }

        // convert everything from parsec to meter
        corner[0] *= parsecToMeter;
        corner[1] *= parsecToMeter;
        corner[2] *= parsecToMeter;

        VistaVector3D projCorner = matMVP.Transform(corner); // box corner in projected space coordinates

        // storing projCorner position compared to frustum axis
        if(projCorner[0] < - projCorner[3]){ // projCorner too far left
            --axis[0];
        }else if(projCorner[0] > projCorner[3]){  // projCorner too far right
            ++axis[0];
        }

        if(projCorner[1] < - projCorner[3]){ // projCorner too far bottom
            --axis[1];
        }else if(projCorner[1] > projCorner[3]){ // projCorner too far top
            ++axis[1];
        }

        if(projCorner[2] < - projCorner[3]){ // projCorner too close
            --axis[2];
        }
        // no far plane for stars
    }

    // checks if all box corners are on the same side of the frustum planes
    for(unsigned int iAxis = 0; iAxis != 3; ++iAxis){
        if(axis[iAxis] == -8 || axis[iAxis] == 8){
            return false;
        }
    }

/*
    // This second half of the test is for some edge cases when the box
    // is bigger than the frustum with a very large aspect ratio

    // An imaged example
    // https://www.iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm

    // Star clusters pop in the distance when using this
    // I think this is because this test does far plane culling and it can not
    // be disabled because we are culling with the box sides and we don't know
    // which ones are used for the far plane

    axis[0] = axis[1] = axis[2] = 0;

    // visibility test in world space
    // checks if the AA box intersects with the frustum in world space
    for(unsigned int iCorner = 0; iCorner != 8; ++iCorner){
        // generate frustum box corner coordinates
        VistaVector3D corner = {-1.f, -1.f, -1.f};
        if(iCorner & 1u){
            corner[0] = 1.f;
        }
        if(iCorner & 2u){
            corner[1] = 1.f;
        }
        if(iCorner & 4u){
            corner[2] = 1.f;
        }

        VistaVector3D worldCorner = matMVP.GetInverted().Transform(corner); // in meters

        worldCorner = worldCorner / worldCorner[3];
        worldCorner /= parsecToMeter;

        // storing worldCorner position compared to frustum axis
        if(worldCorner[0] < box.min[0] * worldCorner[3]){ // worldCorner on the left of the box
            --axis[0];
        }else if(worldCorner[0] > box.max[0] * worldCorner[3]){  // worldCorner on the right of the box
            ++axis[0];
        }

        if(worldCorner[1] < box.min[1] * worldCorner[3]){ // worldCorner below the bottom of the box
            --axis[1];
        }else if(worldCorner[1] > box.max[1] * worldCorner[3]){ // worldCorner above the top of the box
            ++axis[1];
        }

        if(worldCorner[2] < box.min[2] * worldCorner[3]){ // worldCorner before the nearplane of the box
            --axis[2];
        }else(worldCorner[2] > box.max[2] * worldCorner[3]){ // worldCorner after the far plane of the box
            ++axis[2];
        }

    }

    // checking box visibility
    for(unsigned int iAxis = 0; iAxis != 3; ++iAxis){
        if(axis[iAxis] == -8 || axis[iAxis] == 8){
            return false;
        }
    }
*/

    return true;
}

bool VistaStars::isBoxVisible(const PrecisionBox& box, const VistaTransformMatrix& matMVP){
    int axis[3] = {0, 0, 0};

    const float parsecToMeter = 3.08567758e16;

    // visibility test in projected space
    // checks if the projected box intersects the AA frustum box in projected space
    for(unsigned int iCorner = 0; iCorner != 8; ++iCorner){

        // generate bbox corners in worldspace coordinates
        VistaVector3D corner = {(float)box.min[0], (float)box.min[1], (float)box.min[2]};
        if(iCorner & 1u){
            corner[0] = box.max[0];
        }
        if(iCorner & 2u){
            corner[1] = box.max[1];
        }
        if(iCorner & 4u){
            corner[2] = box.max[2];
        }

        // convert everything from parsec to meter
        corner[0] *= parsecToMeter;
        corner[1] *= parsecToMeter;
        corner[2] *= parsecToMeter;

        VistaVector3D projCorner = matMVP.Transform(corner); // box corner in projected space coordinates

        // storing projCorner position compared to frustum axis
        if(projCorner[0] < - projCorner[3]){ // projCorner too far left
            --axis[0];
        }else if(projCorner[0] > projCorner[3]){  // projCorner too far right
            ++axis[0];
        }

        if(projCorner[1] < - projCorner[3]){ // projCorner too far bottom
            --axis[1];
        }else if(projCorner[1] > projCorner[3]){ // projCorner too far top
            ++axis[1];
        }

        if(projCorner[2] < - projCorner[3]){ // projCorner too close
            --axis[2];
        }
        // no far plane for stars
    }

    // checks if all box corners are on the same side of the frustum planes
    for(unsigned int iAxis = 0; iAxis != 3; ++iAxis){
        if(axis[iAxis] == -8 || axis[iAxis] == 8){
            return false;
        }
    }

    return true;
}

float VistaStars::sphereSolidAngle(const VistaVector3D& spherePosition, const float sphereRadius, const VistaVector3D& cameraPosition){
    float sphereToCamera = (spherePosition - cameraPosition).GetLength(); // in parsec as we only want the ratio
    float apexAngle = std::abs(std::atan(sphereRadius / sphereToCamera));

    return std::abs(2 * M_PI * (1 - cos(apexAngle)));
}

long double VistaStars::sphereSolidAngle(const PreciseVec3& spherePosition, const long double sphereRadius, const VistaVector3D& cameraPosition){
    const PreciseVec3 sphereToCamera = {cameraPosition[0] - spherePosition.mX,
        cameraPosition[1] - spherePosition.mY,
        cameraPosition[2] - spherePosition.mZ};

    // in parsec
    const long double sphereToCameraDistance = std::sqrt(sphereToCamera.mX * sphereToCamera.mX
            + sphereToCamera.mY * sphereToCamera.mY
            + sphereToCamera.mZ * sphereToCamera.mZ);

    long double apexAngle = std::atan(sphereRadius / sphereToCameraDistance);

    return 2 * M_PI * (1 - cos(apexAngle));
}

float VistaStars::approximateBoxSolidAngle(const PrecisionBox& box, const VistaVector3D& cameraPosition){
    const PreciseVec3 spherePosition = {(box.max[0] + box.min[0]) / 2,
        (box.max[1] + box.min[1]) / 2,
        (box.max[2] + box.min[2]) / 2};
    const PreciseVec3 sphereRadiusVec = {(box.max[0] - box.min[0]) / 2,
        (box.max[1] - box.min[1]) / 2,
        (box.max[2] - box.min[2]) / 2};

    // in parsec
    const long double sphereRadius = std::sqrt(sphereRadiusVec.mX * sphereRadiusVec.mX
            + sphereRadiusVec.mY * sphereRadiusVec.mY
            + sphereRadiusVec.mZ * sphereRadiusVec.mZ);

    return sphereSolidAngle(spherePosition, sphereRadius, cameraPosition);
}

void VistaStars::Init(const std::string& cacheFolderPath)
{
    std::cout << "[VistaStars] Starting to read the octree structure from " << cacheFolderPath << std::endl;

    // loading the octree structure -------------------------------------------
    m_treeBinary.open(cacheFolderPath + "/octree");
    bool threadStream = m_starThread.initStream(cacheFolderPath + "/star");

    std::cout << "m_treeBinary " << m_treeBinary.is_open() << std::endl;
    std::cout << cacheFolderPath + "/octree" << std::endl;

    if((!m_treeBinary.is_open()) || (!threadStream)){
        std::cout << "[VistaStars] ALERT ! ALERT ! ALERT ! ALERT !" << std::endl;
        std::cout << "[VistaStars] Failed to open the octree / star file from " << cacheFolderPath << ". VistaStars will not work properly."
            << std::endl;
    }else{
        // loading the tree
        LoadTreeFromCache();

    }

    std::cout << "[VistaStars] Finished octree initialization" << std::endl;

    // create buffers ----------------------------------------------------------
    m_pBackgroundVBO = new VistaBufferObject();
    m_pBackgroundVAO = new VistaVertexArrayObject();

    m_pBoxIBO = new VistaBufferObject();
    m_pBoxVAO = new VistaVertexArrayObject();

    m_pPixelQueryIBO = new VistaBufferObject();
    m_pPixelQueryVAO = new VistaVertexArrayObject();

    m_pStarOutlierVBO = new VistaBufferObject();
    m_pStarOutlierVAO = new VistaVertexArrayObject();

    m_pAccumulatedSSO = new VistaBufferObject();
    m_pStarSSO = new VistaBufferObject();
    m_pEmptyVAO = new VistaVertexArrayObject();

    BuildBackgroundVAO();
    BuildBoxIBO();
    BuildPixelQueryIBO();

    // computing the number of blocks allowed by the user-defined allocation size
    const uint64_t gpuStarBlockSize = m_Octree.nStarsPerNode * 7 * sizeof(float);
    m_gpuBlocks = m_maxGPUAllocation / gpuStarBlockSize;

    std::cout << "[VistaStars] User-specified allocation size for stars allows "
        << m_gpuBlocks << " blocks of " << m_Octree.nStarsPerNode
        << " stars to be allocated" << std::endl;

    InitCut();

    InitStarRenderer(cacheFolderPath);

    std::cout << "[VistaStars] Wait for the root node stars to be loaded in main memory" << std::endl;
    m_starThread.synchronize();
    std::cout << "[VistaStars] Root node stars are loaded in main memory" << std::endl;

    UpdateGPUBlocks();
}

void VistaStars::getNodeBytes(uint64_t nodeByteOffset, Node& node){
    // moving to the node data location
    m_treeBinary.seekg(nodeByteOffset);

    node.boundaries.min[0] = getBytes<float>(m_treeBinary);
    node.boundaries.min[1] = getBytes<float>(m_treeBinary);
    node.boundaries.min[2] = getBytes<float>(m_treeBinary);

    node.boundaries.max[0] = getBytes<float>(m_treeBinary);
    node.boundaries.max[1] = getBytes<float>(m_treeBinary);
    node.boundaries.max[2] = getBytes<float>(m_treeBinary);

    node.starByteOffset = getBytes<uint64_t>(m_treeBinary);
    node.nStars = getBytes<unsigned int>(m_treeBinary);

    node.splitQualityIncrease = getBytes<float>(m_treeBinary);

    node.childrenBaseID = getBytes<unsigned int>(m_treeBinary);
    node.nChildren = getBytes<unsigned char>(m_treeBinary);
}

// TODO When building dynamically, we will need some kind of memory pool to allocate the octree efficiently
// For now, we read and build the whole octree into CPU memory
void VistaStars::LoadTreeFromCache()
{
    std::cout << "[VistaStars] Starting to read the octree data from a cache file" << std::endl;

    // header values & initialization
    m_Octree.nStarsPerNode = getBytes<unsigned int>(m_treeBinary);
    std::cout << m_Octree.nStarsPerNode << std::endl;
    m_Octree.maxDepth = getBytes<unsigned int>(m_treeBinary);
    m_Octree.outlierByteOffset = 0;
    m_Octree.nOutliers = getBytes<unsigned int>(m_treeBinary);
    m_Octree.root = new Node;
    m_Octree.root->name = "0";

    // the root is the first node to read
    std::deque<std::pair<unsigned int, Node*>> nodesToLoad;
    nodesToLoad.push_back({0, m_Octree.root});

    while(!nodesToLoad.empty()){
        constexpr uint64_t headerByteSize = 3 * sizeof(unsigned int);
        constexpr uint64_t nodeByteSize = 6 * sizeof(float)
                                        + sizeof(uint64_t)
                                        + sizeof(unsigned int)
                                        + sizeof(float)
                                        + sizeof(unsigned int)
                                        + sizeof(unsigned char);

        uint64_t nodeByteOffset = headerByteSize + nodeByteSize * nodesToLoad.front().first;

        // read the node content from the binary file
        Node& currentNode = *(nodesToLoad.front().second);
        getNodeBytes(nodeByteOffset, currentNode);

        for(unsigned int iChild = 0; iChild != currentNode.nChildren; ++iChild){
            Node* childNode = new Node;
            childNode->parent = nodesToLoad.front().second;
            childNode->name = currentNode.name + std::to_string(iChild);
            currentNode.children[iChild] = childNode;
            nodesToLoad.push_back({currentNode.childrenBaseID + iChild, childNode});
        }

        nodesToLoad.pop_front();
    }

    std::cout << "[VistaStars] Finished reading cache files" << std::endl;
}

#if 0
void VistaStars::UpdateCut(const VistaVector3D& cameraPosition,
        const VistaTransformMatrix& matMVP){

    // Computing the weights for split / collapse -----------------------------

    // evaluating the screen impact of each split
    std::deque<std::pair<float, unsigned int>> splitImprovement; // <value of split, ID of node in m_Cut.nodes>
    for(unsigned int iNode = 0; iNode != m_Cut.nodes.size(); ++iNode){
        if(!m_Cut.nodes[iNode].second->isLeaf()){

            // measure based on screen solid angle
            float solidAngle = approximateBoxSolidAngle(m_Cut.nodes[iNode].second->boundaries, cameraPosition);
            float screenVariation = m_Cut.nodes[iNode].second->splitQualityIncrease / solidAngle;

            // not visible based on frustum -> no split
            if(!isBoxVisible(m_Cut.nodes[iNode].second->boundaries, matMVP)){
                screenVariation = 0.f;
            }

            // enqueue the split if it would strictly improve the screen render
            if(screenVariation > 0.f){
                splitImprovement.emplace_back(screenVariation, iNode);
            }
        }
    }
    std::sort(splitImprovement.begin(), splitImprovement.end()); // highest improvement at the end

    // evaluating the screen impact of each possible collapse operation
    std::deque<std::pair<float, std::map<Node*, std::vector<unsigned int>>::iterator>> collapseDeterioration; // <value of collapse, nodes to remove for collapse>
    {
        for(auto itParent = m_Cut.parentNodes.begin();
            itParent != m_Cut.parentNodes.end();
            ++itParent){

            float solidAngle = approximateBoxSolidAngle(itParent->first->boundaries, cameraPosition);
            float screenVariation = itParent->first->splitQualityIncrease / solidAngle;

            if(!isBoxVisible(itParent->first->boundaries, matMVP)){
                screenVariation = -1.f; // -1.f makes sure it's lower than any other screenVariation value
            }

            collapseDeterioration.emplace_back(screenVariation, itParent);
        }
    }

    auto compareCollapse = [](const std::pair<float, std::map<Node*, std::vector<unsigned int>>::iterator> A,
            const std::pair<float, std::map<Node*, std::vector<unsigned int>>::iterator> B) -> bool {

        return A.first < B.first;
    };
    std::sort(collapseDeterioration.begin(), collapseDeterioration.end(), compareCollapse); // lowest deterioration at the beginning

    std::vector<std::pair<unsigned int, Node*>> newNodes; // <level, node>
    std::vector<char> removeNodeFromCut(m_Cut.nodes.size(), false); // flags for the elements from m_Cut.nodes

    #if 0
    // Collapsing nodes until the cut fits inside its budget
    while((m_Cut.currentGPUBudget < 0) && (!collapseDeterioration.empty())){

        auto toRemove = collapseDeterioration.front();

        // removing the child nodes from the cut
        for(auto& iChild : toRemove.second->second){
            m_Cut.nodes[iChild].second->aboveCut = false;
            removeNodeFromCut[iChild] = true;
        }

        // adding the parent node to the cut
        newNodes.emplace_back(m_Cut.nodes[toRemove.second->second[0]].first - 1,
                toRemove.second->first);

        // updating the cut budget
        m_Cut.currentGPUBudget += toRemove.second->second.size();

        collapseDeterioration.pop_front();
    }
    #endif

    // Split / Collapse logic using the weights -------------------------------

    // use screen impact to determine what node to split and what nodes to collapse
    while(!splitImprovement.empty()){

        const unsigned int& iNodeToSplit = splitImprovement.rbegin()->second; // index in m_Cut.nodes
        Node* const nodeToSplit = m_Cut.nodes[iNodeToSplit].second;
        const unsigned int requieredBudget = nodeToSplit->nChildren;

        auto makeSplit = [&](){
            removeNodeFromCut[iNodeToSplit] = true;

            for(unsigned int iChild = 0; iChild != nodeToSplit->nChildren; ++iChild){

                nodeToSplit->children[iChild]->aboveCut = true;

                newNodes.emplace_back(m_Cut.nodes[iNodeToSplit].first + 1,
                        nodeToSplit->children[iChild]);
            }

            m_Cut.currentGPUBudget -= (unsigned int)nodeToSplit->nChildren;
        };

        if(removeNodeFromCut[iNodeToSplit] == false){ // node has not been collapsed by a previous operation this frame

            // no budget limitation
            if(m_Cut.currentGPUBudget >= requieredBudget){

                makeSplit();

                // budget must be created by collapsing nodes
            }else if(!collapseDeterioration.empty()){

                // estimating the total cost of collapses to make the split
                float totalCollapseCost = 0.f;
                int virtualBudget = m_Cut.currentGPUBudget;
                std::deque<unsigned int> nodesToCollapse;
                unsigned int iCollapse = 0;

                while((virtualBudget < requieredBudget) // not quite enough budget for the split
                        && (iCollapse != collapseDeterioration.size())){ // budget can still be created by collapsing nodes

                    // check if the collapse is possible
                    // ie the collapse operation does not collapse the node to split
                    bool collapsePossible = true;
                    for(auto& iChild : collapseDeterioration[iCollapse].second->second){
                        if(iNodeToSplit == iChild || (removeNodeFromCut[iChild] == true)){
                            collapsePossible = false;
                        }
                    }

                    if(collapsePossible){ // collapse is possible with respect to the node we want to split
                        totalCollapseCost += collapseDeterioration[iCollapse].first;
                        virtualBudget += collapseDeterioration[iCollapse].second->second.size();
                        nodesToCollapse.push_back(iCollapse);
                    }
                    ++iCollapse;
                }

                if((virtualBudget >= requieredBudget) // can create enough budget for the split
                        && (totalCollapseCost < splitImprovement.rbegin()->first)){ // the split would improve the screen render

                    unsigned int iRemove = 0;
                    auto iterRemove = collapseDeterioration.begin();
                    for(auto& collapseID : nodesToCollapse){
                        while(iRemove != collapseID){
                            ++iRemove;
                            ++iterRemove;
                        }

                        // removing the child nodes from the cut
                        for(auto& iChild : iterRemove->second->second){
                            m_Cut.nodes[iChild].second->aboveCut = false;
                            removeNodeFromCut[iChild] = true;

                        }

                        // adding the parent node to the cut
                        newNodes.emplace_back(m_Cut.nodes[iterRemove->second->second[0]].first - 1,
                                iterRemove->second->first);

                        ++iRemove;
                        iterRemove = collapseDeterioration.erase(iterRemove);
                    }

                    m_Cut.currentGPUBudget = virtualBudget;

                    // making the split
                    makeSplit();
                }

                // budget needs to be created but there is no node available for collapse
            }else{
                break;

            }
        }

        splitImprovement.pop_back();
    }

    // rebuild m_Cut.nodes based on the flag and newNodes
    {
        std::vector<std::pair<unsigned int, Node*>> temp;
        for(unsigned int iCurrentNode = 0; iCurrentNode != m_Cut.nodes.size(); ++iCurrentNode){
            if(!removeNodeFromCut[iCurrentNode]){
                temp.push_back(m_Cut.nodes[iCurrentNode]);

            // unload nodes that are not requiered anymore
            }else{
                m_Cut.nodes[iCurrentNode].second->mutex.lock();

                if(!m_Cut.nodes[iCurrentNode].second->aboveCut){
                    m_Cut.nodes[iCurrentNode].second->cache.clear();
                    m_Cut.nodes[iCurrentNode].second->loadedCache = false;
                }

                m_Cut.nodes[iCurrentNode].second->mutex.unlock();
            }
        }

        temp.reserve(temp.size() + newNodes.size());
        for(auto& node : newNodes){
            temp.push_back(node);
        }

        std::swap(m_Cut.nodes, temp);
    }

    // rebuild m_Cut.parentNode
    m_Cut.parentNodes.clear();
    for(unsigned int iNode = 0; iNode != m_Cut.nodes.size(); ++iNode){
        if(m_Cut.nodes[iNode].second->parent != nullptr){ // cannot collapse the root node
            m_Cut.parentNodes[m_Cut.nodes[iNode].second->parent].push_back(iNode);
        }
    }

    for(auto itParent = m_Cut.parentNodes.begin();
            itParent != m_Cut.parentNodes.end();
            ){

        // removing parent nodes that can't be collapsed because not all children are in the cut
        // ie some children were split
        if(itParent->second.size() != itParent->first->nChildren){
            itParent = m_Cut.parentNodes.erase(itParent);

        }else{
            ++itParent;
        }
    }

    // recheck the loading state of each of the nodes from the cut
    constexpr bool filterOnMainThread = false;
    if(filterOnMainThread){
        for(auto& node : m_Cut.nodes){
            if(!node.second->loadedCache){
                m_starThread.addNode(node.second);
            }
        }

    }else{
        m_starThread.addNodes(m_Cut.nodes);

    }
}
#endif

void VistaStars::UpdateCutWithQueries(const VistaVector3D& cameraPosition){

    // Computing the weights for split / collapse -----------------------------

    // evaluating the screen impact of each split
    std::deque<std::pair<float, unsigned int>> splitImprovement; // <value of split, ID of node in m_Cut.nodes>
    for(unsigned int iNode = 0; iNode != m_Cut.nodes.size(); ++iNode){
        if(!m_Cut.nodes[iNode].second->isLeaf()){

            float screenVariation = m_Cut.nodes[iNode].second->splitQualityIncrease
                * m_Cut.nodes[iNode].second->boxPixelQuery;

            // enqueue the split if it would strictly improve the screen render
            if(screenVariation > 0.f){
                splitImprovement.emplace_back(screenVariation, iNode);
            }
        }
    }
    std::sort(splitImprovement.begin(), splitImprovement.end()); // highest improvement at the end

    // evaluating the screen impact of each possible collapse operation
    std::deque<std::pair<float, std::map<Node*, std::vector<unsigned int>>::iterator>> collapseDeterioration; // <value of collapse, nodes to remove for collapse>
    {
        for(auto itParent = m_Cut.parentNodes.begin();
            itParent != m_Cut.parentNodes.end();
            ++itParent){

            float screenVariation = itParent->first->splitQualityIncrease
                * itParent->first->boxPixelQuery;

            collapseDeterioration.emplace_back(screenVariation, itParent);
        }
    }

    auto compareCollapse = [](const std::pair<float, std::map<Node*, std::vector<unsigned int>>::iterator> A,
            const std::pair<float, std::map<Node*, std::vector<unsigned int>>::iterator> B) -> bool {

        return A.first < B.first;
    };
    std::sort(collapseDeterioration.begin(), collapseDeterioration.end(), compareCollapse); // lowest deterioration at the beginning

    std::vector<std::pair<unsigned int, Node*>> newNodes; // <level, node>
    std::vector<char> removeNodeFromCut(m_Cut.nodes.size(), false); // flags for the elements from m_Cut.nodes

    // Filtering out values smaller than a threshold ---------------------------

    // minimum screen impact of each of the nodes displayed on the screen
    constexpr unsigned int minScreenImpact = 1;

    if(minScreenImpact > 0){
        // collapsing nodes with a very low screen contribution
        for(auto iterRemove = collapseDeterioration.begin();
                iterRemove != collapseDeterioration.end();
           ){

            if(iterRemove->first < minScreenImpact){

                // removing the child nodes from the cut
                for(auto& iChild : iterRemove->second->second){
                    m_Cut.nodes[iChild].second->aboveCut = false;
                    removeNodeFromCut[iChild] = true;
                }

                // adding the parent node to the cut
                newNodes.emplace_back(m_Cut.nodes[iterRemove->second->second[0]].first - 1, // level - 1
                        iterRemove->second->first);

                // updating the cut budget
                m_Cut.currentGPUBudget += iterRemove->second->second.size();

                iterRemove = collapseDeterioration.erase(iterRemove);

            }else{
                ++iterRemove;

            }
        }

        // nodes can't be split if their screen contribution is too small
        for(auto iterRemove = splitImprovement.begin();
                iterRemove != splitImprovement.end();
           ){

            if(iterRemove->first < minScreenImpact){
                iterRemove = splitImprovement.erase(iterRemove);

            }else{
                ++iterRemove;
            }
        }
    }

    // Collapsing nodes until the cut fits inside its budget ------------------
    // occurs when the budget is reduced at runtime and the currentGPUBudget takes negative values

    while((m_Cut.currentGPUBudget < 0) && (!collapseDeterioration.empty())){

        auto toRemove = collapseDeterioration.front();

        // removing the child nodes from the cut
        for(auto& iChild : toRemove.second->second){
            m_Cut.nodes[iChild].second->aboveCut = false;
            removeNodeFromCut[iChild] = true;
        }

        // adding the parent node to the cut
        newNodes.emplace_back(m_Cut.nodes[toRemove.second->second[0]].first - 1,
                toRemove.second->first);

        // updating the cut budget
        m_Cut.currentGPUBudget += toRemove.second->second.size();

        collapseDeterioration.pop_front();
    }

    // Split / Collapse logic using the weights -------------------------------

    // use screen impact to determine what node to split and what nodes to collapse
    while(!splitImprovement.empty()){

        const unsigned int& iNodeToSplit = splitImprovement.rbegin()->second; // index in m_Cut.nodes
        Node* const nodeToSplit = m_Cut.nodes[iNodeToSplit].second;
        const int requieredBudget = nodeToSplit->nChildren;

        auto makeSplit = [&](){
            // remove parent
            removeNodeFromCut[iNodeToSplit] = true;

            // add children
            for(unsigned int iChild = 0; iChild != nodeToSplit->nChildren; ++iChild){

                nodeToSplit->children[iChild]->aboveCut = true;

                newNodes.emplace_back(m_Cut.nodes[iNodeToSplit].first + 1,
                        nodeToSplit->children[iChild]);
            }

            // update budget
            m_Cut.currentGPUBudget -= nodeToSplit->nChildren;
        };

        if(removeNodeFromCut[iNodeToSplit] == false){ // node has not been collapsed by a previous operation this frame

            // no budget limitation
            if(m_Cut.currentGPUBudget >= requieredBudget){
                makeSplit();

            // budget must be created by collapsing nodes
            }else if(!collapseDeterioration.empty()){

                // estimating the total cost of collapses to make the split
                float totalCollapseCost = 0.f;
                int virtualBudget = m_Cut.currentGPUBudget;
                std::deque<unsigned int> nodesToCollapse;
                unsigned int iCollapse = 0;

                while((virtualBudget < requieredBudget) // not quite enough budget for the split
                        && (iCollapse != collapseDeterioration.size())){ // budget can still be created by collapsing nodes

                    // check if the collapse is possible
                    // ie the collapse operation does not collapse the node to split
                    bool collapsePossible = true;
                    for(auto& iChild : collapseDeterioration[iCollapse].second->second){
                        if(iNodeToSplit == iChild || (removeNodeFromCut[iChild] == true)){
                            collapsePossible = false;
                        }
                    }

                    if(collapsePossible){ // collapse is possible with respect to the node we want to split
                        totalCollapseCost += collapseDeterioration[iCollapse].first;
                        virtualBudget += collapseDeterioration[iCollapse].second->second.size();
                        nodesToCollapse.push_back(iCollapse);
                    }
                    ++iCollapse;
                }

                if((virtualBudget >= requieredBudget) // can create enough budget for the split
                        && (totalCollapseCost < splitImprovement.rbegin()->first)){ // the split would improve the screen render

                    unsigned int iRemove = 0;
                    auto iterRemove = collapseDeterioration.begin();
                    for(auto& collapseID : nodesToCollapse){
                        while(iRemove != collapseID){
                            ++iRemove;
                            ++iterRemove;
                        }

                        // removing the child nodes from the cut
                        for(auto& iChild : iterRemove->second->second){
                            m_Cut.nodes[iChild].second->aboveCut = false;
                            removeNodeFromCut[iChild] = true;

                        }

                        // adding the parent node to the cut
                        const unsigned int someChildID = iterRemove->second->second[0];
                        newNodes.emplace_back(m_Cut.nodes[iterRemove->second->second[0]].first - 1,
                                iterRemove->second->first);

                        ++iRemove;
                        iterRemove = collapseDeterioration.erase(iterRemove);
                    }

                    m_Cut.currentGPUBudget = virtualBudget;

                    // making the split
                    makeSplit();
                }

            // budget needs to be created but there is no node available for collapse
            }else{
                // no need to pop from splitImprovement because it's not reused after this point
                break;

            }
        }

        splitImprovement.pop_back();
    }

    // Rebuilding the Cut state for next frame --------------------------------

    // rebuild m_Cut.nodes based on the flag and newNodes
    {
        std::vector<std::pair<unsigned int, Node*>> temp;

        // transfering nodes that must be kept
        for(unsigned int iCurrentNode = 0; iCurrentNode != m_Cut.nodes.size(); ++iCurrentNode){
            if(!removeNodeFromCut[iCurrentNode]){
                temp.push_back(m_Cut.nodes[iCurrentNode]);

            // unload nodes that are not requiered anymore
            }else{
                m_Cut.nodes[iCurrentNode].second->mutex.lock();

                if(!m_Cut.nodes[iCurrentNode].second->aboveCut){
                    m_Cut.nodes[iCurrentNode].second->cache.clear();
                    m_Cut.nodes[iCurrentNode].second->loadedCache = false;
                }

                m_Cut.nodes[iCurrentNode].second->mutex.unlock();
            }
        }

        // adding the nodes that were added
        temp.reserve(temp.size() + newNodes.size());
        for(auto& node : newNodes){
            temp.push_back(node);
        }

        std::swap(m_Cut.nodes, temp);
    }

    // rebuild m_Cut.parentNodes
    m_Cut.parentNodes.clear();
    for(unsigned int iNode = 0; iNode != m_Cut.nodes.size(); ++iNode){
        if(m_Cut.nodes[iNode].second->parent != nullptr){ // cannot collapse the root node
            m_Cut.parentNodes[m_Cut.nodes[iNode].second->parent].push_back(iNode);
        }
    }

    // removing parent nodes that can't be collapsed because not all children are in the cut (ie some were split)
    // this prepares the computation of the collapse cost
    for(auto itParent = m_Cut.parentNodes.begin();
            itParent != m_Cut.parentNodes.end();
            ){

        if(itParent->second.size() != itParent->first->nChildren){
            itParent = m_Cut.parentNodes.erase(itParent);

        }else{
            ++itParent;
        }
    }

    // recheck the loading state of each of the nodes from the cut
    constexpr bool filterOnMainThread = false;
    if(filterOnMainThread){
        for(auto& node : m_Cut.nodes){
            if(!(node.second->loadedCache)){
                m_starThread.addNode(node.second);
            }
        }

    }else{
        m_starThread.addNodes(m_Cut.nodes);
    }
}

void VistaStars::UpdateGPUBlocks(){

    std::vector<Node*> toDraw;
    std::vector<Node*> toUpload;

    // Octree traversal to identify --------------------------------------------
    // what nodes must be uploaded to GPU
    // ie those that are available in main memory but not already on the GPU
    // what nodes that are / will be uploaded on the GPU need to be drawn
    {
        std::deque<Node*> traversalQueue = {m_Octree.root};

        while(!traversalQueue.empty()){
            Node* const node = traversalQueue.front();
            traversalQueue.pop_front();

            if(node->isLeaf()){
                if(node->block == nullptr){
                    toUpload.push_back(node);
                }

                toDraw.push_back(node);

            // not leaf ie must have two or more children
            }else if(node->children[0]->aboveCut){ // node was split by the cut
                if(node->block == nullptr){
                    toUpload.push_back(node);
                }

                // if one of the children is not loaded we display the current node instead
                bool splitDraw = true;
                for(unsigned int iChild = 0; iChild != node->nChildren; ++iChild){
                    if(!(node->children[iChild]->loadedCache)){
                        splitDraw = false;
                    }
                }

                if(splitDraw){
                    for(unsigned int iChild = 0; iChild != node->nChildren; ++iChild){
                        traversalQueue.push_back(node->children[iChild]);
                    }

                }else{
                    toDraw.push_back(node);
                }

            }else{
                if(node->block == nullptr){
                    toUpload.push_back(node);
                }

                toDraw.push_back(node);
            }
        }
    }

    // Rebuild the bound / available queues -----------------------------------
    {

        // ---------------------------------------------------------------------

/*
        // (Hugo Rambure)
        // TODO: In theory a solution like this should be more efficient because
        //       less nodes are considered (minor improvement) but it seems that
        //       we are losing some blocks when doing this.

        // identify what previously available nodes are still available
        std::deque<BlockGPU*> newAvailable;
        for(auto& block : m_gpuAvailable){
            if(block->node == nullptr || !block->node->aboveCut){
                newAvailable.push_back(block);
            }
        }
        std::swap(m_gpuAvailable, newAvailable);

        // identify what previously bound nodes became available this frame
        for(auto& block : m_gpuBound){
            if(block->node != nullptr && !block->node->aboveCut){
                m_gpuAvailable.push_back(block);
            }
        }
        m_gpuBound.clear();
*/

        // brute-force alternative ie we check all blocks
        m_gpuAvailable.clear();
        for(auto& block : m_gpuBlockBuffer){
            if(block.node == nullptr){
                m_gpuAvailable.push_back(&block);

            }else if(!block.node->aboveCut){
                m_gpuAvailable.push_back(&block);

            }
        }

        // ---------------------------------------------------------------------

        // upload all requiered nodes
        m_pStarSSO->Bind(GL_SHADER_STORAGE_BUFFER);
        for(auto& node : toUpload){

#if 0
            // @DEBUG TODO: Remove this as this is for DEBUG purposes
            if(m_gpuAvailable.size() == 0){
                std::cout << "ABORTING BECAUSE : m_gpuAvailable == 0" << std::endl;
                abort();
            }
#endif

            // take ownership of a block
            BlockGPU* block = m_gpuAvailable.front();
            m_gpuAvailable.pop_front();

            // update the links between nodes / blocks
            if(block->node != nullptr){
                block->node->block = nullptr;
            }
            block->node = node;
            node->block = block;

            // no need to use a mutex because only the main thread is allowed
            // to clear the main memory cache of nodes

            // transmit star data from main memory to GPU

            GLint64 dataOffset = block->gpuPosition * m_Octree.nStarsPerNode * 7 * sizeof(float);
            GLuint64 dataSize = node->cache.size() * sizeof(float);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER,
                    dataOffset,
                    dataSize,
                    node->cache.data());
        }


        m_pStarSSO->Release();

        // clear star bindings
        std::fill(m_gpuAccumulated.begin(), m_gpuAccumulated.end(), 0);

        // bind the nodes to draw
        for(auto& node : toDraw){
            m_gpuBound.push_back(node->block);

            // update accumulated values
            for(unsigned int iBlock = node->block->gpuPosition + 1;
                    iBlock != m_gpuAccumulated.size();
                    ++iBlock){

                m_gpuAccumulated[iBlock] += node->nStars;

            }
        }
    }

    // transmit accumulated list to GPU
    m_pAccumulatedSSO->Bind(GL_SHADER_STORAGE_BUFFER);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER,
            0,
            m_gpuAccumulated.size() * sizeof(unsigned int),
            m_gpuAccumulated.data());
    m_pAccumulatedSSO->Release();
}

void VistaStars::IssueGPUQueries(const VistaVector3D& cameraPosition,
        const VistaTransformMatrix& matMVP,
        const unsigned int pixelsOnScreen){

    // resetting query values
    for(auto& node : m_Cut.nodes){
        node.second->boxPixelQuery = -1;
    }
    for(auto& node : m_Cut.parentNodes){
        node.first->boxPixelQuery = -1;
    }

    // query is not necessary when inside the node
    // simpler this way because the query is done without backface culling
    for(auto& node : m_Cut.nodes){
        if(isPositionInsideBox(cameraPosition, node.second->boundaries)){
            node.second->boxPixelQuery = pixelsOnScreen;
        }
    }
    for(auto& node : m_Cut.parentNodes){
        if(isPositionInsideBox(cameraPosition, node.first->boundaries)){
            node.first->boxPixelQuery = pixelsOnScreen;
        }
    }

    m_pPixelQueryVAO->Bind();
    m_pPixelQueryIBO->Bind(GL_ELEMENT_ARRAY_BUFFER);
    m_pPixelQueryShader->Bind();

    GLint boundaryMinLoc = m_pPixelQueryShader->GetUniformLocation("boundaryMin");
    GLint boundaryMaxLoc = m_pPixelQueryShader->GetUniformLocation("boundaryMax");
    GLint uMatMVPLoc = m_pPixelQueryShader->GetUniformLocation("uMatMVP");

    glUniformMatrix4fv(uMatMVPLoc, 1, GL_FALSE, matMVP.GetData());

    // saving the current pipeline state
    GLboolean depthTest = glIsEnabled(GL_DEPTH_TEST);
    GLboolean cullFace = glIsEnabled(GL_CULL_FACE);

    GLint cullFaceMode;
    glGetIntegerv(GL_CULL_FACE_MODE, &cullFaceMode);

    GLboolean colorMask[4];
    glGetBooleanv(GL_COLOR_WRITEMASK, colorMask);


    // modifying the pipeline state to make pixel queries
    glDisable(GL_DEPTH_TEST); // disable depth testing
    glEnable(GL_CULL_FACE); // enable back face culling to avoid counting a screen pixel two times
    glCullFace(GL_BACK);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // disable modification of the render buffer color

    auto drawQuery = [this, boundaryMinLoc, boundaryMaxLoc](const PrecisionBox& boundaries){

        glUniform3f(boundaryMinLoc, (float)boundaries.min[0], (float)boundaries.min[1], (float)boundaries.min[2]);
        glUniform3f(boundaryMaxLoc, (float)boundaries.max[0], (float)boundaries.max[1], (float)boundaries.max[2]);

        constexpr unsigned int nIndicesPixelQuery = 6 * 2 * 3; // 6 faces, 2 triangles per face, 3 indices per triangle
        glDrawElements(GL_TRIANGLES, nIndicesPixelQuery, GL_UNSIGNED_INT, (void*)0);
    };

    m_nodeQueryBindings.clear();
    m_nodeQueryBindings.reserve(m_Cut.parentNodes.size() + m_Cut.nodes.size());

    unsigned int iParent = 0;
    for(auto itParent = m_Cut.parentNodes.begin();
            itParent != m_Cut.parentNodes.end();
            ++itParent){

        if(itParent->first->boxPixelQuery == -1){
            glBeginQuery(GL_SAMPLES_PASSED, m_gpuSampleQueries[iParent]);
            drawQuery(itParent->first->boundaries);
            glEndQuery(GL_SAMPLES_PASSED);

            m_nodeQueryBindings.emplace_back(itParent->first, m_gpuSampleQueries[iParent]);
            ++iParent;
        }
    }

    for(unsigned int cutNode = 0; cutNode != m_Cut.nodes.size(); ++cutNode){
        if(m_Cut.nodes[cutNode].second->boxPixelQuery == -1){
            glBeginQuery(GL_SAMPLES_PASSED, m_gpuSampleQueries[iParent + cutNode]);
            drawQuery(m_Cut.nodes[cutNode].second->boundaries);
            glEndQuery(GL_SAMPLES_PASSED);

            m_nodeQueryBindings.emplace_back(m_Cut.nodes[cutNode].second, m_gpuSampleQueries[iParent + cutNode]);
        }
    }

    // restoring the previous pipeline state
    if(depthTest){
        glEnable(GL_DEPTH_TEST);
    }
    if(!cullFace){
        glDisable(GL_CULL_FACE);
    }
    glCullFace(cullFaceMode);
    glColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);

    m_pPixelQueryShader->Release();
    m_pPixelQueryIBO->Release();
    m_pPixelQueryVAO->Release();
}

void VistaStars::RetrieveGPUQueries(){

    for(auto& query : m_nodeQueryBindings){
        unsigned int queryResult;
        glGetQueryObjectuiv(query.second, GL_QUERY_RESULT, &queryResult);

        query.first->boxPixelQuery = (int)queryResult;
    }

}

void VistaStars::PurgeTree(){
    if(m_Octree.root){
        std::deque<Node*> nodesToPurge;
        nodesToPurge.push_back(m_Octree.root);

        while(!nodesToPurge.empty()){
            for(unsigned int iChild = 0; iChild != nodesToPurge.front()->nChildren; ++iChild){
                nodesToPurge.push_back(nodesToPurge.front()->children[iChild]);
            }

            delete nodesToPurge.front();
            nodesToPurge.pop_front();
        }
    }
}

void VistaStars::PurgeQueries(){
    glDeleteQueries(m_gpuSampleQueries.size(), m_gpuSampleQueries.data());
}

void VistaStars::BuildBackgroundVAO()
{
    std::vector<float> data(8);
    data[0] = -1;
    data[1] = 1;
    data[2] = 1;
    data[3] = 1;
    data[4] = -1;
    data[5] = -1;
    data[6] = 1;
    data[7] = -1;

    m_pBackgroundVBO->Bind(GL_ARRAY_BUFFER);
    m_pBackgroundVBO->BufferData(data.size() * sizeof(float), &(data[0]), GL_STATIC_DRAW);
    m_pBackgroundVBO->Release();

    // positions
    m_pBackgroundVAO->EnableAttributeArray(0);
    m_pBackgroundVAO->SpecifyAttributeArrayFloat(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0,
                                                 m_pBackgroundVBO);
}

void VistaStars::BuildBoxIBO()
{
    std::cout << "[VistaStars] Building the bounding boxes VAO" << std::endl;

    std::vector<unsigned int> indices = {0, 1, 1, 3, 3, 2, 2, 0, 0, 4, 1, 5, 2, 6, 3, 7, 4, 5, 5, 7, 7, 6, 6, 4};

    m_pBoxIBO->Bind(GL_ELEMENT_ARRAY_BUFFER);
    m_pBoxIBO->BufferData(indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    m_pBoxIBO->Release();

    std::cout << "[VistaStars] Finished building the bounding boxes VAO" << std::endl;
}

void VistaStars::BuildPixelQueryIBO()
{
    std::cout << "[VistaStars] Building the bounding box query VAO" << std::endl;

    std::vector<unsigned int> indices = {
        0, 2, 3, 0, 3, 1, // bottom face
        4, 0, 1, 4, 1, 5, // front face
        5, 1, 3, 5, 3, 7, // right face
        6, 2, 0, 6, 0, 4, // left face
        7, 3, 2, 7, 2, 6, // back face
        6, 4, 5, 6, 5, 7, // top face
    };

    m_pPixelQueryIBO->Bind(GL_ELEMENT_ARRAY_BUFFER);
    m_pPixelQueryIBO->BufferData(indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    m_pPixelQueryIBO->Release();

    std::cout << "[VistaStars] Finished building the bounding box query VAO" << std::endl;
}

void VistaStars::InitCut(){

    // load the root node in main memory
    m_starThread.addNode(m_Octree.root);

    // initializing rendering cut
    m_Cut.totalGPUBudget = m_gpuBlocks;
    m_Cut.currentGPUBudget = m_gpuBlocks - 1; // -1 block to init. with root node
    m_Cut.root = m_Octree.root;
    m_Cut.root->aboveCut = true;
    m_Cut.root->boxPixelQuery = 1; // must be > 0.f for the first frame
    m_Cut.nodes.emplace_back(0, m_Octree.root); // cut is initialized to the root

}

void VistaStars::InitStarRenderer(const std::string& cacheFolderPath){

    std::cout << "[VistaStars] Reading outlier star data from disk and buffering them on GPU" << std::endl;
    std::cout << "[VistaStars] Number of octree outliers : " << m_Octree.nOutliers << std::endl;
    if(m_Octree.nOutliers > 0){
        std::ifstream tempStarFile(cacheFolderPath + "/star");

        if(tempStarFile.is_open()){
            std::vector<float> tempCache;
            tempCache.reserve(m_Octree.nOutliers * 7);

            tempStarFile.seekg(m_Octree.outlierByteOffset);
            for(unsigned int iStar = 0; iStar != m_Octree.nOutliers; ++iStar){
                tempCache.push_back(getBytes<float>(tempStarFile)); // mPosition.mX
                tempCache.push_back(getBytes<float>(tempStarFile)); // mPosition.mY
                tempCache.push_back(getBytes<float>(tempStarFile)); // mPosition.mZ

                tempCache.push_back(getBytes<float>(tempStarFile)); // mColor.mR
                tempCache.push_back(getBytes<float>(tempStarFile)); // mColor.mG
                tempCache.push_back(getBytes<float>(tempStarFile)); // mColor.mB

                tempCache.push_back(getBytes<float>(tempStarFile)); // mAbsoluteMagnitude
            }

            std::cout << "outlier cache size " << tempCache.size() << std::endl;

            // uploading data
            m_pStarOutlierVBO->Bind(GL_ARRAY_BUFFER);
            m_pStarOutlierVBO->BufferData(tempCache.size() * sizeof(float), tempCache.data(), GL_STATIC_DRAW);
            m_pStarOutlierVBO->Release();

            // positions
            m_pStarOutlierVAO->EnableAttributeArray(0);
            m_pStarOutlierVAO->SpecifyAttributeArrayFloat(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0, m_pStarOutlierVBO);

            // color
            m_pStarOutlierVAO->EnableAttributeArray(1);
            m_pStarOutlierVAO->SpecifyAttributeArrayFloat(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 3 * sizeof(float), m_pStarOutlierVBO);

            // abs. magnitude
            m_pStarOutlierVAO->EnableAttributeArray(2);
            m_pStarOutlierVAO->SpecifyAttributeArrayFloat(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 6 * sizeof(float), m_pStarOutlierVBO);

        }else{
            std::cerr << "[VistaStars] Failed to open the star file from " << cacheFolderPath << " to load outliers data. VistaStars will not work properly" << std::endl;
        }

        tempStarFile.close();
    }

    std::cout << "[VistaStars] Allocating the octree SSOs" << std::endl;

    constexpr unsigned int accumulatedBinding = 0;
    constexpr unsigned int starDataBinding = 1;

    m_pAccumulatedSSO->Bind(GL_SHADER_STORAGE_BUFFER);
    m_pAccumulatedSSO->BufferData((m_gpuBlocks + 1) * sizeof(unsigned int), NULL, GL_STREAM_DRAW);
    m_pAccumulatedSSO->Release();

    m_pStarSSO->Bind(GL_SHADER_STORAGE_BUFFER);
    m_pStarSSO->BufferData(m_gpuBlocks * m_Octree.nStarsPerNode * 7 * sizeof(float), NULL, GL_STREAM_DRAW);

    // checking that the buffer size is what we asked for
    GLint64 buffer_size;
    glGetBufferParameteri64v(GL_SHADER_STORAGE_BUFFER, GL_BUFFER_SIZE, &buffer_size);
    if(buffer_size != (m_gpuBlocks * m_Octree.nStarsPerNode * 7 * sizeof(float))){
        std::cout << "[VistaStars] Could not allocate memory requiered for the stars on the GPU" << std::endl;
    }

    std::cout << "[VistaStars] Number of blocks allocated" << m_gpuBlocks << std::endl;
    std::cout << "[VistaStars] Expected size " << m_gpuBlocks * m_Octree.nStarsPerNode * 7 * sizeof(float)
        << " Actual size " << buffer_size << std::endl;

    std::cout << "Error " << glGetError() << std::endl;

    m_pStarSSO->Release();

    // allocating the GPU cache rendering system
    m_gpuBlockBuffer.resize(m_gpuBlocks);
    m_gpuAccumulated.resize(m_gpuBlocks + 1, 0); // last value stores the total number of stars to render
    for(unsigned int iBlock = 0; iBlock != m_gpuBlockBuffer.size(); ++iBlock){
        m_gpuBlockBuffer[iBlock].gpuPosition = iBlock;
        m_gpuAvailable.push_back(&(m_gpuBlockBuffer[iBlock])); // necessary only when using a sorted availability list
    }

    std::cout << "[VistaStars] Allocating the screen sample queries" << std::endl;

    m_gpuSampleQueries.resize(m_gpuBlocks);
    glGenQueries(m_gpuSampleQueries.size(), m_gpuSampleQueries.data());

    std::cout << "[VistaStars] Finished allocating the octree sample queries" << std::endl;
}

void VistaStars::UpdateShader()
{
    std::string defines;
    //defines += "#version 400 compatibility\n";
    defines += "#version 460 compatibility\n";

    if (m_eDrawMode == DRAWMODE_SMOOTH_POINT)
        defines += "#define DRAWMODE_SMOOTH_POINT\n";
    if (m_eDrawMode == DRAWMODE_POINT)
        defines += "#define DRAWMODE_POINT\n";
    if (m_eDrawMode == DRAWMODE_DISC)
        defines += "#define DRAWMODE_DISC\n";
    if (m_eDrawMode == DRAWMODE_SMOOTH_DISC)
        defines += "#define DRAWMODE_SMOOTH_DISC\n";
    if (m_eDrawMode == DRAWMODE_SPRITE)
        defines += "#define DRAWMODE_SPRITE\n";

    delete m_pPixelQueryShader;
    m_pPixelQueryShader = new VistaGLSLShader();
    m_pPixelQueryShader->InitVertexShaderFromString(defines + QUERY_VERT);
    //m_pPixelQueryShader->InitFragmentShaderFromString(defines + QUERY_FRAG);
    m_pPixelQueryShader->Link();

    delete m_pStarOutlierShader;
    m_pStarOutlierShader = new VistaGLSLShader();
    if (m_eDrawMode == DRAWMODE_POINT || m_eDrawMode == DRAWMODE_SMOOTH_POINT)
    {
        m_pStarOutlierShader->InitVertexShaderFromString(defines + STARS_VERT_ONE_PIXEL);
        m_pStarOutlierShader->InitFragmentShaderFromString(defines + STARS_FRAG_ONE_PIXEL);
    }
    else
    {
        m_pStarOutlierShader->InitVertexShaderFromString(defines + STARS_VERT);
        m_pStarOutlierShader->InitFragmentShaderFromString(defines + STARS_FRAG);
        m_pStarOutlierShader->InitGeometryShaderFromString(defines + STARS_GEOM);
    }
    m_pStarOutlierShader->Link();

    delete m_pStarShader;
    m_pStarShader = new VistaGLSLShader();

    if (m_eDrawMode == DRAWMODE_POINT || m_eDrawMode == DRAWMODE_SMOOTH_POINT)
    {
        m_pStarShader->InitVertexShaderFromString(defines + STARS_SSO_VERT_ONE_PIXEL);
        m_pStarShader->InitFragmentShaderFromString(defines + STARS_FRAG_ONE_PIXEL);
    }
    else
    {
        m_pStarShader->InitVertexShaderFromString(defines + STARS_SSO_VERT);
        m_pStarShader->InitFragmentShaderFromString(defines + STARS_FRAG);
        m_pStarShader->InitGeometryShaderFromString(defines + STARS_GEOM);
    }
    m_pStarShader->Link();

    delete m_pBackgroundShader;
    m_pBackgroundShader = new VistaGLSLShader();
    m_pBackgroundShader->InitVertexShaderFromString(defines + BACKGROUND_VERT);
    m_pBackgroundShader->InitFragmentShaderFromString(defines + BACKGROUND_FRAG);
    m_pBackgroundShader->Link();

    delete m_pBoxShader;
    m_pBoxShader = new VistaGLSLShader();
    m_pBoxShader->InitVertexShaderFromString(defines + BOX_VERT);
    m_pBoxShader->InitFragmentShaderFromString(defines + BOX_FRAG);
    m_pBoxShader->Link();
}
