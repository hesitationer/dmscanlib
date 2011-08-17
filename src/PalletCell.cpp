#include "PalletCell.h"
#include "PalletGrid.h"
#include "Dib.h"
#include "Decoder.h"
#include "UaAssert.h"
#include "UaLogger.h"

#ifdef _VISUALC_
#   include <functional>
#else
#   include <tr1/functional>
#endif

PalletCell::PalletCell(PalletGrid & pg, unsigned r, unsigned c,
                       CvRect & pos) :
		grid(pg), row(r), col(c), parentRect(pos) {
}

PalletCell::~PalletCell() {
}

/**
 * Invoked in its own thread.
 */
void PalletCell::run() {
    Decoder::DecodeCallback callback = std::tr1::bind(
            &PalletCell::decodeCallback, this, std::tr1::placeholders::_1,
            std::tr1::placeholders::_2);
    ostringstream id;
    id << row << "-" << col;
    grid.getDecoder().decodeImage(getImage(), id.str(), callback);
}

void PalletCell::decodeCallback(std::string & msg, CvPoint(&corners)[4]) {
    decodedMsg = msg;
    grid.registerBarcodeMsg(decodedMsg);

    for (unsigned i = 0; i < 4; ++i) {
        barcodeCorners[i] = corners[i];
    }

    UA_DOUT(3, 5, "PalletCell::setDecodeInfo: message/"
            << decodedMsg
            << " tlCorner/(" << parentRect.x << "," << parentRect.y
            << ")  brCorner/(" << parentRect.x + parentRect.width
            << "," << parentRect.y + parentRect.height << ")");
}

std::tr1::shared_ptr<const Dib> PalletCell::getImage() {
    if (cellImage.get() == NULL) {
        cellImage = grid.getCellImage(*this);
    }
    return cellImage;
}

const string & PalletCell::getBarcodeMsg() {
    UA_ASSERT(!decodedMsg.empty());
    return decodedMsg;
}

void PalletCell::writeImage(std::string basename) {
    // do not write diagnostic image is log level is less than 9
    if (ua::Logger::Instance().levelGet(3) < 9) return;

    ostringstream fname;
    fname << basename << "-" << row << "-" << col << ".bmp";
    cellImage->writeToFile(fname.str().c_str());
}

void PalletCell::drawCellBox(Dib & image, const RgbQuad & color) const {
	image.rectangle(parentRect.x, parentRect.y, parentRect.width, parentRect.height,
			color);
}

void PalletCell::drawBarcodeBox(Dib & image, const RgbQuad & color) const {
	CvPoint corners[4];

	for (unsigned i = 0; i < 4; ++i) {
		corners[i].x = barcodeCorners[i].x + parentRect.x;
		corners[i].y = barcodeCorners[i].y + parentRect.y;
	}

	image.line(corners[0], corners[1], color);
	image.line(corners[1], corners[2], color);
	image.line(corners[2], corners[3], color);
	image.line(corners[3], corners[0], color);
}

ostream & operator<<(ostream &os, PalletCell & m) {
	os << "\"" << m.decodedMsg << "\" (" << m.barcodeCorners[0].x << ", "
			<< m.barcodeCorners[0].y << "), " << "(" << m.barcodeCorners[2].x
			<< ", " << m.barcodeCorners[2].y << "), " << "("
			<< m.barcodeCorners[3].x << ", " << m.barcodeCorners[3].y << "), "
			<< "(" << m.barcodeCorners[1].x << ", " << m.barcodeCorners[1].y
			<< ")";
	return os;
}
