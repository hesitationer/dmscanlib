#ifndef DECODER_H_
#define DECODER_H_

/*
 * Dmscanlib is a software library and standalone application that scans
 * and decodes libdmtx compatible test-tubes. It is currently designed
 * to decode 12x8 pallets that use 2D data-matrix laser etched test-tubes.
 * Copyright (C) 2010 Canadian Biosample Repository
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "DecodedWell.h"

#include <dmtx.h>
#include <string>

#ifdef _VISUALC_
#   include <memory>
#else
#   include <tr1/memory>
#endif

#ifdef WIN32
#include <windows.h>
#endif

using namespace std;

class Dib;
struct RgbQuad;
class BinRegion;

class Decoder {
public:
    Decoder(unsigned dpi, double scanGap, unsigned squareDev,
            unsigned edgeThresh, unsigned corrections, double cellDistance);
    virtual ~Decoder();

    void decodeImage(const Dib & dib, DecodedWell & decodedWell);

private:
    static DmtxImage * createDmtxImageFromDib(const Dib & dib);
    void getDecodeInfo(DmtxDecode *dec, DmtxRegion *reg, DmtxMessage *msg,
                       DecodedWell & DecodedWell);

    void writeDiagnosticImage(DmtxDecode *dec, const string & id);

    void showStats(DmtxDecode *dec, DmtxRegion *reg, DmtxMessage *msg);

    unsigned dpi;
    double scanGap;
    unsigned squareDev;
    unsigned edgeThresh;
    unsigned corrections;
    double cellDistance;
};

#endif /* DECODER_H_ */
