////////////////////////////////////////////////////////////////////////////////
//                   This file is part of VistaStars                          //
//             Copyright: (c) German Aerospace Center (DLR)                   //
////////////////////////////////////////////////////////////////////////////////

#ifndef VISTASTARS_H
#define VISTASTARS_H

/*============================================================================*/
/* MACROS AND DEFINES                                                         */
/*============================================================================*/

/*============================================================================*/
/* INCLUDES                                                                   */
/*============================================================================*/
#include <VistaBase/VistaColor.h>
#include <VistaKernel/GraphicsManager/VistaOpenGLDraw.h>
#include <VistaBase/VistaVector3D.h>
#include <VistaBase/VistaTransformMatrix.h>

#include <glm/vec4.hpp>

#include <fstream>
#include <vector>
#include <deque>
#include <map>

#include <mutex>
#include <thread>
#include <condition_variable>

#include "VistaStarsExport.hpp"

class VistaGLSLShader;
class VistaTexture;
class VistaVertexArrayObject;
class VistaBufferObject;

/*============================================================================*/
/* CLASS DEFINITIONS                                                          */
/*============================================================================*/
/**
 * If added to the scene graph, this will draw a configurable star background.
 * It is possible to limit the drawn stars by magnitude, adjust their diameter,
 * texture and opacity.
 * Furthermore it is possible to draw multiple sky dome images additively on top
 * in order to visualize additional information such as a constellations or
 * grid lines.
 */
class VISTASTARS_API VistaStars : public IVistaOpenGLDraw
{
public:
    enum DrawMode
    {
        DRAWMODE_POINT,
        DRAWMODE_SMOOTH_POINT,
        DRAWMODE_DISC,
        DRAWMODE_SMOOTH_DISC,
        DRAWMODE_SPRITE
    };

    /**
     * Creates a new instance of this class.
     * @param cacheFolderPath containing the octree cache files
     * @param maxGPUStarsInMbytes maximum size of the allocation for star data on the GPU
     */
    VistaStars(const std::string& cacheFolderPath, const uint32_t maxGPUStarsInMbytes);

    virtual ~VistaStars();

    /**
     * Sets the draw mode of the stars.
     * Default is DRAWMODE_CONE.
     * @param fValue   In steradians.
     */
    void     SetDrawMode(DrawMode eValue);
    DrawMode GetDrawMode() const;

    /**
     * Sets the size of the stars.
     * Stars will be drawn covering this solid angle. This has no effect if DrawMode
     * is set to DRAWMODE_POINT. Default is 0.01f.
     * @param fValue   In steradians.
     */
    void  SetSolidAngle(float fValue);
    float GetSolidAngle() const;

    /**
     * Sets the maximum apparent magnitude of the visible stars.
     * Stars with a higher magnitude will not be drawn.
     * Default is 15.f
     */
    void  SetLuminanceMultiplicator(float fValue);
    float GetLuminanceMultiplicator() const;

    /**
     * Toggles the display of the MultiLevelGrid bounding boxes
     */
    void  SetEnableBoundingBoxes(bool bValue);
    bool GetEnableBoundingBoxes() const;

    /**
     * Toggles the display of the MultiLevelGrid bounding boxes
     */
    void  SetEnableStarsDisplay(bool bValue);
    bool GetEnableStarsDisplay() const;

    /**
     * Sets the proportion of the GPU allocation that the Cut is allowed to use
     * Default is 1.f ie all budget
     */
    void SetPercentBudgetCut(float fValue);
    float GetPercentBudgetCut() const;

    /**
     * Toogles the freezing of the state of the cut
     * Default is false
     */
    void SetEnableFreezeCut(bool bValue);
    bool GetEnableFreezeCut() const;

    /**
     * Adds a skydome texture.
     * The given texture is projected via equirectangular projection onto the
     * background and blended additively.
     * @param sFilename    A path to an uncompressed TGA image or "" to disable
     *                     this image.
     */
    void SetBackgroundTexture1(const std::string& sFilename);
    void SetBackgroundTexture2(const std::string& sFilename);

    /**
     * Colorizes the skydome texture.
     * Since the textures are blended additively, the alpha component modulates
     * the brightness only.
     * @param cValue    A RGBA color.
     */
    void              SetBackgroundColor1(const VistaColor& cValue);
    const VistaColor& GetBackgroundColor1() const;
    void              SetBackgroundColor2(const VistaColor& cValue);
    const VistaColor& GetBackgroundColor2() const;

    // ---------------------------------------
    // INTERFACE IMPLEMENTATION OFIVistaOpenGLDraw
    // ---------------------------------------

    /**
     * The method Do() gets the callback from scene graph during the rendering
     * process.
     */
    virtual bool Do();

    /**
     * This method should return the bounding box of the openGL object you draw
     * in the method Do().
     */
    virtual bool GetBoundingBox(VistaBoundingBox& bb);

private:
    struct Vec3{
        float mX;
        float mY;
        float mZ;
    };

    struct Box{
        float min[3];
        float max[3];
    };

    struct PreciseVec3{
        long double mX;
        long double mY;
        long double mZ;
    };

    struct PrecisionBox{
        long double min[3];
        long double max[3];
    };

    struct Node; // forward declaration

    struct BlockGPU{
        Node* node = nullptr;
        unsigned int gpuPosition = 0;
    };

    struct Node{
        // data
        std::string name;
        PrecisionBox boundaries;

        uint64_t starByteOffset;
        unsigned int nStars;

        float splitQualityIncrease = 0.f; // luminosity_children / luminosity_this
        int boxPixelQuery = -1;

        // links
        unsigned int childrenBaseID;
        unsigned char nChildren;
        Node* children[8] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
        Node* parent = nullptr;

        bool isLeaf();

        // loading / display status
        bool aboveCut = false; // nodes from m_Cut.nodes and their parents
        BlockGPU* block = nullptr;

        bool loadedCache = false;
        std::vector<float> cache;

        // theoretically not thread safe
        bool isLoadedNoLock();

        bool isLoadedLock();
        bool isLoadedTryLock();

        // async loading
        std::mutex mutex;
    };

    struct Octree{
        unsigned int nStarsPerNode = 0;
        unsigned int maxDepth = 0;

        uint64_t outlierByteOffset = 0;
        unsigned int nOutliers = 0;

        Node* root = nullptr;
    };

    struct Cut{
        int currentGPUBudget = 0;
        unsigned int totalGPUBudget = 0;
        Node* root = nullptr;

        std::map<Node*, std::vector<unsigned int>> parentNodes; // <parentNode, childrenID in nodes>
        std::vector<std::pair<unsigned int, Node*>> nodes; // <level, node>
    };

    struct LoadingThread{
        LoadingThread();
        ~LoadingThread();

        bool initStream(const std::string& filePath);

        void addNode(Node* node);
        void addNodes(const std::vector<Node*>& nodes);
        void addNodes(const std::vector<std::pair<unsigned int, Node*>>& nodes);

        void synchronize();

        std::deque<Node*> nodesToCheck;

        std::thread thread;
        std::mutex isWorking;
        bool stop = false;

        std::mutex mutex;
        std::condition_variable condition;

        std::ifstream binaryFile;
    };

    /**
     * Checks if position is inside an axis aligned bounding box
     */
    bool isPositionInsideBox(const VistaVector3D& position, const Box& box);
    bool isPositionInsideBox(const VistaVector3D& position, const PrecisionBox& box);

     /**
     * Checks if the box or part of the box is visible by the view frustum
     */
    bool isBoxVisible(const Box& box, const VistaTransformMatrix& matMVP);
    bool isBoxVisible(const PrecisionBox& box, const VistaTransformMatrix& matMVP);

    /**
     * Compute the solid angle of a sphere
     * https://en.wikipedia.org/wiki/Solid_angle#Cone,_spherical_cap,_hemisphere
     */
    float sphereSolidAngle(const VistaVector3D& spherePosition, const float sphereRadius, const VistaVector3D& cameraPosition);
    long double sphereSolidAngle(const PreciseVec3& spherePosition, const long double sphereRadius, const VistaVector3D& cameraPosition);

    /**
     * Approximate the solid angle of a box with the solid angle of its circumscribed sphere
     */
    float approximateBoxSolidAngle(const PrecisionBox& box, const VistaVector3D& cameraPosition);

    /**
     * Compute the distance at which the octree boxes must be drawn for each level
     */
    void UpdateRenderDistanceThresholds();

    /**
     * Called by the constructors.
     */
    void Init(const std::string& cacheFolderPath);

     /**
     * Fill the node with data starting at nodeByteOffset in a given stream
     */
    void getNodeBytes(uint64_t nodeByteOffset, Node& node);

    /**
     * Reads and allocated the whole octree from m_treeBinary
     */
    void LoadTreeFromCache();

    /**
     * De-allocate the whole octree from m_treeBinary
     */
    void PurgeTree();

    /**
     * De-allocate the whole queries
     */
    void PurgeQueries();

     /**
     * Reads the whole octree from m_treeBinary
     * Returns a list of nodes to unload
     */
    void UpdateCut(const VistaVector3D& cameraPosition,
            const VistaTransformMatrix& matMVP);
    void UpdateCutWithQueries(const VistaVector3D& cameraPosition);

    /**
     * Uploads star data from main memory into the GPU
     */
    void UpdateGPUBlocks();

    /**
     * Uploads star data from main memory into the GPU
     */
    void IssueGPUQueries(const VistaVector3D& cameraPosition,
            const VistaTransformMatrix& matMVP,
            const unsigned int pixelsOnScreen);
    void RetrieveGPUQueries();

    /**
     * Build vertex array objects from given star list.
     */
    void BuildBackgroundVAO();
    void BuildBoxIBO();
    void BuildPixelQueryIBO();

    /**
     * Initialize the cut
     */
    void InitCut();

    /**
     * Allocating containers used for the star rendering, both on CPU & GPU
     * @param cacheFolderPath containing the binary star file to import outlier star data
     */
    void InitStarRenderer(const std::string& cacheFolderPath);

    void UpdateShader();

    VistaGLSLShader* m_pPixelQueryShader = nullptr;
    VistaGLSLShader* m_pStarOutlierShader = nullptr;
    VistaGLSLShader* m_pStarShader       = nullptr;
    VistaGLSLShader* m_pBackgroundShader = nullptr;
    VistaGLSLShader* m_pBoxShader = nullptr;

    VistaTexture* m_pBackgroundTexture1 = nullptr;
    VistaTexture* m_pBackgroundTexture2 = nullptr;
    VistaColor    m_cBackgroundColor1;
    VistaColor    m_cBackgroundColor2;

    VistaVertexArrayObject* m_pBackgroundVAO = nullptr;
    VistaBufferObject*      m_pBackgroundVBO = nullptr;

    VistaVertexArrayObject* m_pBoxVAO = nullptr;
    VistaBufferObject* m_pBoxIBO = nullptr;

    VistaVertexArrayObject* m_pPixelQueryVAO = nullptr;
    VistaBufferObject* m_pPixelQueryIBO = nullptr;

    VistaBufferObject* m_pStarOutlierVBO = nullptr;
    VistaVertexArrayObject* m_pStarOutlierVAO = nullptr;

    VistaBufferObject* m_pAccumulatedSSO = nullptr;
    VistaBufferObject* m_pStarSSO = nullptr;
    VistaVertexArrayObject* m_pEmptyVAO = nullptr;

    std::ifstream m_treeBinary;
    LoadingThread m_starThread;
    Octree m_Octree;
    Cut m_Cut;

    // star rendering system allocation
    std::vector<BlockGPU> m_gpuBlockBuffer;
    std::vector<unsigned int> m_gpuAccumulated;
    std::vector<unsigned int> m_gpuSampleQueries;
    std::vector<std::pair<Node*, unsigned int>> m_nodeQueryBindings;
    std::deque<BlockGPU*> m_gpuBound;
    std::deque<BlockGPU*> m_gpuAvailable;

    DrawMode m_eDrawMode               = DRAWMODE_POINT;
    float    m_fSolidAngle             = 0.01f; // solid angle of the stars
    float    m_fLuminanceMultiplicator = 1.f;
    bool m_bEnableBoundingBoxes = false;
    bool m_bEnableStarsDisplay = false;
    float m_fPercentBudgetCut = 1.f;
    bool m_bEnableFreezeCut = false;

    uint64_t m_maxGPUAllocation = 0; // in bytes
    uint64_t m_gpuBlocks = 0;
    uint64_t m_targetCutBudget = 0;

    bool     m_bShaderDirty            = true;

    static const std::string STARS_VERT;
    static const std::string STARS_SSO_VERT;
    static const std::string STARS_FRAG;
    static const std::string STARS_GEOM;
    static const std::string STARS_VERT_ONE_PIXEL;
    static const std::string STARS_SSO_VERT_ONE_PIXEL;
    static const std::string STARS_FRAG_ONE_PIXEL;
    static const std::string BACKGROUND_VERT;
    static const std::string BACKGROUND_FRAG;
    static const std::string BOX_VERT;
    static const std::string BOX_FRAG;
    static const std::string QUERY_VERT;
    static const std::string QUERY_FRAG;
};

#endif // VISTASTARS_H
