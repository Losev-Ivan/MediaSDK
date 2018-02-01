// Copyright (c) 2017 Intel Corporation
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "umc_defs.h"
#ifdef UMC_ENABLE_VP9_VIDEO_DECODER

#include "umc_structures.h"
#include "umc_vp9_utils.h"
#include "umc_vp9_frame.h"

namespace UMC_VP9_DECODER
{
    static const mfxI16 dc_qlookup_8[QINDEX_RANGE] =
    {
        4,       8,    8,    9,   10,   11,   12,   12,
        13,     14,   15,   16,   17,   18,   19,   19,
        20,     21,   22,   23,   24,   25,   26,   26,
        27,     28,   29,   30,   31,   32,   32,   33,
        34,     35,   36,   37,   38,   38,   39,   40,
        41,     42,   43,   43,   44,   45,   46,   47,
        48,     48,   49,   50,   51,   52,   53,   53,
        54,     55,   56,   57,   57,   58,   59,   60,
        61,     62,   62,   63,   64,   65,   66,   66,
        67,     68,   69,   70,   70,   71,   72,   73,
        74,     74,   75,   76,   77,   78,   78,   79,
        80,     81,   81,   82,   83,   84,   85,   85,
        87,     88,   90,   92,   93,   95,   96,   98,
        99,    101,  102,  104,  105,  107,  108,  110,
        111,   113,  114,  116,  117,  118,  120,  121,
        123,   125,  127,  129,  131,  134,  136,  138,
        140,   142,  144,  146,  148,  150,  152,  154,
        156,   158,  161,  164,  166,  169,  172,  174,
        177,   180,  182,  185,  187,  190,  192,  195,
        199,   202,  205,  208,  211,  214,  217,  220,
        223,   226,  230,  233,  237,  240,  243,  247,
        250,   253,  257,  261,  265,  269,  272,  276,
        280,   284,  288,  292,  296,  300,  304,  309,
        313,   317,  322,  326,  330,  335,  340,  344,
        349,   354,  359,  364,  369,  374,  379,  384,
        389,   395,  400,  406,  411,  417,  423,  429,
        435,   441,  447,  454,  461,  467,  475,  482,
        489,   497,  505,  513,  522,  530,  539,  549,
        559,   569,  579,  590,  602,  614,  626,  640,
        654,   668,  684,  700,  717,  736,  755,  775,
        796,   819,  843,  869,  896,  925,  955,  988,
        1022, 1058, 1098, 1139, 1184, 1232, 1282, 1336,
    };

    static const mfxI16 dc_qlookup_10[QINDEX_RANGE] =
    {
        4,     9,     10,    13,    15,    17,    20,    22,
        25,    28,    31,    34,    37,    40,    43,    47,
        50,    53,    57,    60,    64,    68,    71,    75,
        78,    82,    86,    90,    93,    97,    101,   105,
        109,   113,   116,   120,   124,   128,   132,   136,
        140,   143,   147,   151,   155,   159,   163,   166,
        170,   174,   178,   182,   185,   189,   193,   197,
        200,   204,   208,   212,   215,   219,   223,   226,
        230,   233,   237,   241,   244,   248,   251,   255,
        259,   262,   266,   269,   273,   276,   280,   283,
        287,   290,   293,   297,   300,   304,   307,   310,
        314,   317,   321,   324,   327,   331,   334,   337,
        343,   350,   356,   362,   369,   375,   381,   387,
        394,   400,   406,   412,   418,   424,   430,   436,
        442,   448,   454,   460,   466,   472,   478,   484,
        490,   499,   507,   516,   525,   533,   542,   550,
        559,   567,   576,   584,   592,   601,   609,   617,
        625,   634,   644,   655,   666,   676,   687,   698,
        708,   718,   729,   739,   749,   759,   770,   782,
        795,   807,   819,   831,   844,   856,   868,   880,
        891,   906,   920,   933,   947,   961,   975,   988,
        1001,  1015,  1030,  1045,  1061,  1076,  1090,  1105,
        1120,  1137,  1153,  1170,  1186,  1202,  1218,  1236,
        1253,  1271,  1288,  1306,  1323,  1342,  1361,  1379,
        1398,  1416,  1436,  1456,  1476,  1496,  1516,  1537,
        1559,  1580,  1601,  1624,  1647,  1670,  1692,  1717,
        1741,  1766,  1791,  1817,  1844,  1871,  1900,  1929,
        1958,  1990,  2021,  2054,  2088,  2123,  2159,  2197,
        2236,  2276,  2319,  2363,  2410,  2458,  2508,  2561,
        2616,  2675,  2737,  2802,  2871,  2944,  3020,  3102,
        3188,  3280,  3375,  3478,  3586,  3702,  3823,  3953,
        4089,  4236,  4394,  4559,  4737,  4929,  5130,  5347,
    };

    static const mfxI16 dc_qlookup_12[QINDEX_RANGE] =
    {
        4,     12,    18,    25,    33,    41,    50,    60,
        70,    80,    91,    103,   115,   127,   140,   153,
        166,   180,   194,   208,   222,   237,   251,   266,
        281,   296,   312,   327,   343,   358,   374,   390,
        405,   421,   437,   453,   469,   484,   500,   516,
        532,   548,   564,   580,   596,   611,   627,   643,
        659,   674,   690,   706,   721,   737,   752,   768,
        783,   798,   814,   829,   844,   859,   874,   889,
        904,   919,   934,   949,   964,   978,   993,   1008,
        1022,  1037,  1051,  1065,  1080,  1094,  1108,  1122,
        1136,  1151,  1165,  1179,  1192,  1206,  1220,  1234,
        1248,  1261,  1275,  1288,  1302,  1315,  1329,  1342,
        1368,  1393,  1419,  1444,  1469,  1494,  1519,  1544,
        1569,  1594,  1618,  1643,  1668,  1692,  1717,  1741,
        1765,  1789,  1814,  1838,  1862,  1885,  1909,  1933,
        1957,  1992,  2027,  2061,  2096,  2130,  2165,  2199,
        2233,  2267,  2300,  2334,  2367,  2400,  2434,  2467,
        2499,  2532,  2575,  2618,  2661,  2704,  2746,  2788,
        2830,  2872,  2913,  2954,  2995,  3036,  3076,  3127,
        3177,  3226,  3275,  3324,  3373,  3421,  3469,  3517,
        3565,  3621,  3677,  3733,  3788,  3843,  3897,  3951,
        4005,  4058,  4119,  4181,  4241,  4301,  4361,  4420,
        4479,  4546,  4612,  4677,  4742,  4807,  4871,  4942,
        5013,  5083,  5153,  5222,  5291,  5367,  5442,  5517,
        5591,  5665,  5745,  5825,  5905,  5984,  6063,  6149,
        6234,  6319,  6404,  6495,  6587,  6678,  6769,  6867,
        6966,  7064,  7163,  7269,  7376,  7483,  7599,  7715,
        7832,  7958,  8085,  8214,  8352,  8492,  8635,  8788,
        8945,  9104,  9275,  9450,  9639,  9832,  10031, 10245,
        10465, 10702, 10946, 11210, 11482, 11776, 12081, 12409,
        12750, 13118, 13501, 13913, 14343, 14807, 15290, 15812,
        16356, 16943, 17575, 18237, 18949, 19718, 20521, 21387,
    };

    static const mfxI16 ac_qlookup_8[QINDEX_RANGE] =
    {
        4,       8,    9,   10,   11,   12,   13,   14,
        15,     16,   17,   18,   19,   20,   21,   22,
        23,     24,   25,   26,   27,   28,   29,   30,
        31,     32,   33,   34,   35,   36,   37,   38,
        39,     40,   41,   42,   43,   44,   45,   46,
        47,     48,   49,   50,   51,   52,   53,   54,
        55,     56,   57,   58,   59,   60,   61,   62,
        63,     64,   65,   66,   67,   68,   69,   70,
        71,     72,   73,   74,   75,   76,   77,   78,
        79,     80,   81,   82,   83,   84,   85,   86,
        87,     88,   89,   90,   91,   92,   93,   94,
        95,     96,   97,   98,   99,  100,  101,  102,
        104,   106,  108,  110,  112,  114,  116,  118,
        120,   122,  124,  126,  128,  130,  132,  134,
        136,   138,  140,  142,  144,  146,  148,  150,
        152,   155,  158,  161,  164,  167,  170,  173,
        176,   179,  182,  185,  188,  191,  194,  197,
        200,   203,  207,  211,  215,  219,  223,  227,
        231,   235,  239,  243,  247,  251,  255,  260,
        265,   270,  275,  280,  285,  290,  295,  300,
        305,   311,  317,  323,  329,  335,  341,  347,
        353,   359,  366,  373,  380,  387,  394,  401,
        408,   416,  424,  432,  440,  448,  456,  465,
        474,   483,  492,  501,  510,  520,  530,  540,
        550,   560,  571,  582,  593,  604,  615,  627,
        639,   651,  663,  676,  689,  702,  715,  729,
        743,   757,  771,  786,  801,  816,  832,  848,
        864,   881,  898,  915,  933,  951,  969,  988,
        1007, 1026, 1046, 1066, 1087, 1108, 1129, 1151,
        1173, 1196, 1219, 1243, 1267, 1292, 1317, 1343,
        1369, 1396, 1423, 1451, 1479, 1508, 1537, 1567,
        1597, 1628, 1660, 1692, 1725, 1759, 1793, 1828,
    };

    static const mfxI16 ac_qlookup_10[QINDEX_RANGE] =
    {
        4,     9,     11,    13,    16,    18,    21,    24,
        27,    30,    33,    37,    40,    44,    48,    51,
        55,    59,    63,    67,    71,    75,    79,    83,
        88,    92,    96,    100,   105,   109,   114,   118,
        122,   127,   131,   136,   140,   145,   149,   154,
        158,   163,   168,   172,   177,   181,   186,   190,
        195,   199,   204,   208,   213,   217,   222,   226,
        231,   235,   240,   244,   249,   253,   258,   262,
        267,   271,   275,   280,   284,   289,   293,   297,
        302,   306,   311,   315,   319,   324,   328,   332,
        337,   341,   345,   349,   354,   358,   362,   367,
        371,   375,   379,   384,   388,   392,   396,   401,
        409,   417,   425,   433,   441,   449,   458,   466,
        474,   482,   490,   498,   506,   514,   523,   531,
        539,   547,   555,   563,   571,   579,   588,   596,
        604,   616,   628,   640,   652,   664,   676,   688,
        700,   713,   725,   737,   749,   761,   773,   785,
        797,   809,   825,   841,   857,   873,   889,   905,
        922,   938,   954,   970,   986,   1002,  1018,  1038,
        1058,  1078,  1098,  1118,  1138,  1158,  1178,  1198,
        1218,  1242,  1266,  1290,  1314,  1338,  1362,  1386,
        1411,  1435,  1463,  1491,  1519,  1547,  1575,  1603,
        1631,  1663,  1695,  1727,  1759,  1791,  1823,  1859,
        1895,  1931,  1967,  2003,  2039,  2079,  2119,  2159,
        2199,  2239,  2283,  2327,  2371,  2415,  2459,  2507,
        2555,  2603,  2651,  2703,  2755,  2807,  2859,  2915,
        2971,  3027,  3083,  3143,  3203,  3263,  3327,  3391,
        3455,  3523,  3591,  3659,  3731,  3803,  3876,  3952,
        4028,  4104,  4184,  4264,  4348,  4432,  4516,  4604,
        4692,  4784,  4876,  4972,  5068,  5168,  5268,  5372,
        5476,  5584,  5692,  5804,  5916,  6032,  6148,  6268,
        6388,  6512,  6640,  6768,  6900,  7036,  7172,  7312,
    };

    static const mfxI16 ac_qlookup_12[QINDEX_RANGE] =
    {
        4,     13,    19,    27,    35,    44,    54,    64,
        75,    87,    99,    112,   126,   139,   154,   168,
        183,   199,   214,   230,   247,   263,   280,   297,
        314,   331,   349,   366,   384,   402,   420,   438,
        456,   475,   493,   511,   530,   548,   567,   586,
        604,   623,   642,   660,   679,   698,   716,   735,
        753,   772,   791,   809,   828,   846,   865,   884,
        902,   920,   939,   957,   976,   994,   1012,  1030,
        1049,  1067,  1085,  1103,  1121,  1139,  1157,  1175,
        1193,  1211,  1229,  1246,  1264,  1282,  1299,  1317,
        1335,  1352,  1370,  1387,  1405,  1422,  1440,  1457,
        1474,  1491,  1509,  1526,  1543,  1560,  1577,  1595,
        1627,  1660,  1693,  1725,  1758,  1791,  1824,  1856,
        1889,  1922,  1954,  1987,  2020,  2052,  2085,  2118,
        2150,  2183,  2216,  2248,  2281,  2313,  2346,  2378,
        2411,  2459,  2508,  2556,  2605,  2653,  2701,  2750,
        2798,  2847,  2895,  2943,  2992,  3040,  3088,  3137,
        3185,  3234,  3298,  3362,  3426,  3491,  3555,  3619,
        3684,  3748,  3812,  3876,  3941,  4005,  4069,  4149,
        4230,  4310,  4390,  4470,  4550,  4631,  4711,  4791,
        4871,  4967,  5064,  5160,  5256,  5352,  5448,  5544,
        5641,  5737,  5849,  5961,  6073,  6185,  6297,  6410,
        6522,  6650,  6778,  6906,  7034,  7162,  7290,  7435,
        7579,  7723,  7867,  8011,  8155,  8315,  8475,  8635,
        8795,  8956,  9132,  9308,  9484,  9660,  9836,  10028,
        10220, 10412, 10604, 10812, 11020, 11228, 11437, 11661,
        11885, 12109, 12333, 12573, 12813, 13053, 13309, 13565,
        13821, 14093, 14365, 14637, 14925, 15213, 15502, 15806,
        16110, 16414, 16734, 17054, 17390, 17726, 18062, 18414,
        18766, 19134, 19502, 19886, 20270, 20670, 21070, 21486,
        21902, 22334, 22766, 23214, 23662, 24126, 24590, 25070,
        25551, 26047, 26559, 27071, 27599, 28143, 28687, 29247,
    };

    inline
    int16_t vp9_dc_quant(int32_t qindex, int32_t delta, uint32_t bit_depth)
    {
        switch (bit_depth) {
            case 8:
                return dc_qlookup_8[clamp(qindex + delta, 0, MAXQ)];
            case 10:
                return dc_qlookup_10[clamp(qindex + delta, 0, MAXQ)];
            case 12:
                return dc_qlookup_12[clamp(qindex + delta, 0, MAXQ)];
            default:
                VM_ASSERT(0);
                return -1;
          }
    }

    inline
    int16_t vp9_ac_quant(int32_t qindex, int32_t delta, uint32_t bit_depth)
    {
        switch (bit_depth) {
            case 8:
                return ac_qlookup_8[clamp(qindex + delta, 0, MAXQ)];
            case 10:
                return ac_qlookup_10[clamp(qindex + delta, 0, MAXQ)];
            case 12:
                return ac_qlookup_12[clamp(qindex + delta, 0, MAXQ)];
            default:
                VM_ASSERT(0);
                return -1;
          }
    }

    int32_t GetQIndex(VP9Segmentation const & seg, uint8_t segmentId, int32_t baseQIndex)
    {
        if (!IsSegFeatureActive(seg, segmentId, SEG_LVL_ALT_Q))
            return baseQIndex;
        else
        {
            const int32_t data = GetSegData(seg, segmentId, UMC_VP9_DECODER::SEG_LVL_ALT_Q);
            return
                 seg.absDelta == SEGMENT_ABSDATA ?
                 data :  // Abs value
                 clamp(baseQIndex + data, 0, MAXQ);  // Delta value
        }
    }

    void InitDequantizer(VP9DecoderFrame* info)
    {
        for (mfxU16 q = 0; q < UMC_VP9_DECODER::QINDEX_RANGE; ++q)
        {
            info->yDequant[q][0] = vp9_dc_quant(q, info->y_dc_delta_q, info->bit_depth);
            info->yDequant[q][1] = vp9_ac_quant(q, 0, info->bit_depth);

            info->uvDequant[q][0] = vp9_dc_quant(q, info->uv_dc_delta_q, info->bit_depth);
            info->uvDequant[q][1] = vp9_ac_quant(q, info->uv_ac_delta_q, info->bit_depth);
        }
    }

    UMC::ColorFormat GetUMCColorFormat_VP9(VP9DecoderFrame const* info)
    {
        if (info->color_space == SRGB)
            //only 4:4:4 chroma sampling is allowed for SRGB
            return UMC::RGB24;

        if (!info->subsamplingX)
        {
            VM_ASSERT(info->subsamplingY == 0);

            //4:4:4
            switch (info->bit_depth)
            {
                case  8: return UMC::AYUV;
                case 10: return UMC::Y410;
                case 12: return UMC::Y416;
                default: return UMC::NONE;
            }
        }
        else if (info->subsamplingY)
        {
            //4:2:0
            switch (info->bit_depth)
            {
                case  8: return UMC::YUV420;
                case 10: return UMC::P010;
                case 12: return UMC::P016;
                default: return UMC::NONE;
            }
        }
        else
        {
            //4:2:2
            switch (info->bit_depth)
            {
                case  8: return UMC::YUV422;
                case 10: return UMC::P210;
                case 12: return UMC::P216;
                default: return UMC::NONE;
            }
        }

    }

    void SetSegData(VP9Segmentation & seg, uint8_t segmentId, SEG_LVL_FEATURES featureId, int32_t seg_data)
    {
        VM_ASSERT(seg_data <= SEG_FEATURE_DATA_MAX[featureId]);
        if (seg_data < 0)
        {
            VM_ASSERT(SEG_FEATURE_DATA_SIGNED[featureId]);
            VM_ASSERT(-seg_data <= SEG_FEATURE_DATA_MAX[featureId]);
        }

        seg.featureData[segmentId][featureId] = seg_data;
    }

    void SetupPastIndependence(UMC_VP9_DECODER::VP9DecoderFrame & info)
    {
        // Reset the segment feature data to the default stats:
        // Features disabled, 0, with delta coding (Default state).

        ClearAllSegFeatures(info.segmentation);
        info.segmentation.absDelta = UMC_VP9_DECODER::SEGMENT_DELTADATA;

        // set_default_lf_deltas()
        info.lf.modeRefDeltaEnabled = 1;
        info.lf.modeRefDeltaUpdate = 1;

        info.lf.refDeltas[INTRA_FRAME] = 1;
        info.lf.refDeltas[LAST_FRAME] = 0;
        info.lf.refDeltas[GOLDEN_FRAME] = -1;
        info.lf.refDeltas[ALTREF_FRAME] = -1;

        info.lf.modeDeltas[0] = 0;
        info.lf.modeDeltas[1] = 0;

        memset(info.refFrameSignBias, 0, sizeof(info.refFrameSignBias));
    }

    void GetTileNBits(const int32_t miCols, int32_t & minLog2TileCols, int32_t & maxLog2TileCols)
    {
        const int32_t sbCols = ALIGN_POWER_OF_TWO(miCols, MI_BLOCK_SIZE_LOG2) >> MI_BLOCK_SIZE_LOG2;
        int32_t minLog2 = 0, maxLog2 = 0;

        while ((sbCols >> maxLog2) >= MIN_TILE_WIDTH_B64)
            ++maxLog2;
        --maxLog2;
        if (maxLog2 < 0)
            maxLog2 = 0;

        while ((MAX_TILE_WIDTH_B64 << minLog2) < sbCols)
            ++minLog2;

        VM_ASSERT(minLog2 <= maxLog2);

        minLog2TileCols = minLog2;
        maxLog2TileCols = maxLog2;
    }
} //UMC_VP9_DECODER

#endif //__UMC_VP9_UTILS_H_
