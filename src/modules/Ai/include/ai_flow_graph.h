#pragma once

#include "core.h"
#include "c_vector.h"
#include "storm_assert.h"
#include "file_service.h"
#include "math_inlines.h"

#include <algorithm>
#include <string>
#include <vector>

#include "Filesystem/Config/Config.hpp"

#define INVALID_ARRAY_INDEX 0xFFFFFFFF

class AIFlowGraph
{
  public:
    struct edge_t
    {
        uint32_t dw1, dw2; // first and second point index
        float fLen;        // edge len

        edge_t()
        {
        }

        edge_t(uint32_t _dw1, uint32_t _dw2, float _fLen)
        {
            dw1 = _dw1;
            dw2 = _dw2;
            fLen = _fLen;
        }

        inline bool operator==(const edge_t &e) const
        {
            return ((e.dw1 == dw1 && e.dw2 == dw2) || (e.dw1 == dw2 && e.dw2 == dw1));
        }
    };

    struct npoint_t
    {
        uint32_t dwPnt;
        float fDistance;
        float fTemp;

        inline bool operator<(const npoint_t &n) const
        {
            return (fDistance < n.fDistance);
        }
    };

    struct point_t
    {
        CVECTOR vPos;
        uint32_t dwFlags;
        std::vector<std::size_t> aEdges;

        point_t()
            : vPos(), dwFlags(0)
        {
        }

        point_t(CVECTOR _vPos)
            : dwFlags(0)
        {
            vPos = _vPos;
        }

        inline bool operator==(const point_t &p) const
        {
            return ((~(p.vPos - vPos)) < 1e-5f);
        }
    };

  protected:
    std::vector<edge_t> aEdges;
    std::vector<point_t> aPoints;
    std::string sSectionName;

    struct table_t
    {
        uint32_t p;
        float d;
    };

    table_t *pTable;

  public:
    AIFlowGraph()
    {
        pTable = nullptr;
        sSectionName = "GraphPoints";
    }

    ~AIFlowGraph()
    {
        STORM_DELETE(pTable);
    }

    // save/load/release section
    void ReleaseAll();
    bool Load(const std::string_view& config_path);

    size_t GetNumEdges()
    {
        return aEdges.size();
    }

    CVECTOR GetPointPos(size_t dwPnt)
    {
        Assert(dwPnt < aPoints.size());
        return aPoints[dwPnt].vPos;
    }

    edge_t *GetEdge(size_t dwEdgeIdx);
    float GetPathDistance(size_t dwP1, size_t dwP2);
    float GetDistance(size_t dwP1, size_t dwP2);
    size_t GetOtherEdgePoint(size_t dwEdgeIdx, size_t dwPnt);
    std::vector<npoint_t> *GetNearestPoints(CVECTOR &vP);

    decltype(aEdges)::difference_type AddEdge(size_t dwEdgePnt1, size_t dwEdgePnt2);

    void BuildTable();

  private:
};

inline void AIFlowGraph::ReleaseAll()
{
    aEdges.clear();
    aPoints.clear();
}

inline bool AIFlowGraph::Load(const std::string_view& config_path)
{
    ReleaseAll();

    auto config = Storm::Filesystem::Config::load(config_path);
    std::ignore = config.select_section(sSectionName);

    while (true) {
        const std::string key{"pnt" + std::to_string(std::size(aPoints))};
        auto array = config.get_array<std::string>(key);
        if (array.empty()) {
            break;
        }
        point_t point(CVECTOR(std::stof(array[0]), 0.0f, std::stof(array[1])));
        aPoints.push_back(point);

        for (int i = 3; i < std::size(array); i += 2) {
            aPoints[i].aEdges.push_back(AddEdge(std::stol(array[i]), std::stol(array[i + 1])));
        }
    }

    return false;
}

inline decltype(AIFlowGraph::aEdges)::difference_type AIFlowGraph::AddEdge(size_t dwEdgePnt1, size_t dwEdgePnt2)
{
    Assert(dwEdgePnt1 < aPoints.size() && dwEdgePnt2 < aPoints.size());

    const edge_t e(dwEdgePnt1, dwEdgePnt2, sqrtf(~(GetPointPos(dwEdgePnt1) - GetPointPos(dwEdgePnt2))));

    const auto it = std::find(aEdges.begin(), aEdges.end(), e);
    if (it != aEdges.end())
        return it - aEdges.begin();

    aEdges.push_back(e);
    return aEdges.size() - 1;
}

inline AIFlowGraph::edge_t *AIFlowGraph::GetEdge(size_t dwEdgeIdx)
{
    Assert(dwEdgeIdx < aEdges.size());
    return &aEdges[dwEdgeIdx];
}

inline size_t AIFlowGraph::GetOtherEdgePoint(size_t dwEdgeIdx, size_t dwPnt)
{
    Assert(dwEdgeIdx < aEdges.size());
    if (aEdges[dwEdgeIdx].dw1 == dwPnt)
        return aEdges[dwEdgeIdx].dw2;
    return aEdges[dwEdgeIdx].dw1;
}

inline void AIFlowGraph::BuildTable()
{
    uint32_t i, j, k, x, y;
    const auto dwNumPoints = aPoints.size();

    STORM_DELETE(pTable);
    pTable = new table_t[SQR(dwNumPoints)];
    for (i = 0; i < SQR(dwNumPoints); i++)
    {
        pTable[i].p = INVALID_ARRAY_INDEX;
        pTable[i].d = 1e8f;
    }
    for (i = 0; i < dwNumPoints; i++)
    {
        point_t *pP = &aPoints[i];
        table_t *pTableRow = &pTable[i * dwNumPoints];
        for (j = 0; j < pP->aEdges.size(); j++)
        {
            const uint32_t dwPnt = GetOtherEdgePoint(pP->aEdges[j], i);
            pTableRow[dwPnt].p = dwPnt;
            pTableRow[dwPnt].d = aEdges[pP->aEdges[j]].fLen;
        }
    }
    for (k = 0; k < dwNumPoints; k++)
    {
        bool bF = true;
        for (y = 0; y < dwNumPoints; y++)
        {
            for (x = 0; x < dwNumPoints; x++)
                if (x != y)
                {
                    point_t *pP = &aPoints[y];
                    float d = pTable[x + y * dwNumPoints].d;
                    for (j = 0; j < pP->aEdges.size(); j++)
                    {
                        const uint32_t dwPnt = GetOtherEdgePoint(pP->aEdges[j], y);
                        const float d1 = pTable[dwPnt + y * dwNumPoints].d;
                        const float d2 = pTable[x + dwPnt * dwNumPoints].d;
                        if (d1 + d2 < d && fabsf((d1 + d2) - d) > 0.01f)
                        {
                            d = d1 + d2;
                            pTable[x + y * dwNumPoints].d = d;
                            pTable[x + y * dwNumPoints].p = dwPnt;
                            bF = false;
                        }
                    }
                }
        }
        if (bF)
            break;
    }
}

inline float AIFlowGraph::GetDistance(size_t dwP1, size_t dwP2)
{
    return sqrtf(~(GetPointPos(dwP2) - GetPointPos(dwP1)));
}

inline float AIFlowGraph::GetPathDistance(size_t dwP1, size_t dwP2)
{
    Assert(dwP1 < aPoints.size() && dwP2 < aPoints.size());
    if (dwP1 == dwP2)
        return 0.0f;
    const auto dwNumPoints = aPoints.size();

    float fDistance = 0.0f;
    uint32_t dwPnt = pTable[dwP2 + dwP1 * dwNumPoints].p;
    while (dwPnt != INVALID_ARRAY_INDEX)
    {
        fDistance += GetDistance(dwP1, dwPnt);
        dwP1 = dwPnt;
        dwPnt = pTable[dwP2 + dwPnt * dwNumPoints].p;
    }

    return fDistance;
}

inline std::vector<AIFlowGraph::npoint_t> *AIFlowGraph::GetNearestPoints(CVECTOR &vP)
{
    auto aNearestPoints = new std::vector<npoint_t>(aPoints.size());
    for (uint32_t i = 0; i < aPoints.size(); i++)
    {
        // npoint_t * pN = &(*aNearestPoints)[(*aNearestPoints).Add()];
        // pN->fDistance = sqrtf(~(vP - aPoints[i].vPos));
        // pN->dwPnt = i;
        (*aNearestPoints)[i].fDistance = sqrtf(~(vP - aPoints[i].vPos));
        (*aNearestPoints)[i].dwPnt = i;
    }
    //(*aNearestPoints).Sort();
    std::sort(aNearestPoints->begin(), aNearestPoints->end());

    return aNearestPoints;
}
