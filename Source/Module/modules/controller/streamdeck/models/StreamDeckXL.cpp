/*
  ==============================================================================

    StreamDeckXL.cpp
    Created: 14 Feb 2020 12:29:21pm
    Author:  bkupe

  ==============================================================================
*/

#include "StreamDeckXL.h"

StreamDeckXL::StreamDeckXL(hid_device* device, String serialNumber) :
	StreamDeck(device, serialNumber, STANDARD_V2, 8, 4, false, 96, 4)
{
	imagePacketLength = 1024;
	imageHeaderLength = 8;

	brightnessData.add(
		0x03, 0x08, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	);

	resetData.add(
		0x03,
		0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	);
}

StreamDeckXL::~StreamDeckXL()
{
}

void StreamDeckXL::setBrightnessInternal(float brightness)
{
	brightnessData.set(2, (int)(brightness * 100));
}

void StreamDeckXL::writeImageDataHeader(MemoryOutputStream& stream, int keyIndex, int partIndex, bool isLast, int bodyLength)
{
	stream.writeByte(0x02);
	stream.writeByte(0x07);
	stream.writeByte(keyIndex);
	stream.writeByte(isLast ? 1 : 0);
	stream.writeShort(bodyLength);
	stream.writeShort(partIndex);
}

void StreamDeckXL::writeImageData(MemoryOutputStream& stream, Image& img)
{
	Image gImg(Image::RGB, img.getWidth(), img.getHeight(), false);
	Graphics g(gImg);
	g.drawImageTransformed(img, AffineTransform::rotation(float_Pi, g.getClipBounds().getCentreX(), g.getClipBounds().getCentreY()));

	JPEGImageFormat format;
	format.setQuality(1.f);
	format.writeImageToStream(gImg, stream);
}
