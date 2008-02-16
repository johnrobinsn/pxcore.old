// pxCore CopyRight 2007 John Robinson
// Portable Framebuffer and Windowing Library
// pxBuffer.h

#ifndef PX_BUFFER_H
#define PX_BUFFER_H

#include "pxColors.h"
#include "pxRect.h"
#include "pxCore.h"

// This class is used to point to and describe a 32bpp framebuffer
// The memory for this framebuffer is allocated and managed external
// to this class.
//

class pxBuffer
{
public:

    pxBuffer(): mWidth(0), mHeight(0), mStride(0), mUpsideDown(false)
    {
    }

    void* base() const { return mBase; }
    void setBase(void* p) { mBase = p; }
    
    int width()  const { return mWidth; }
    void setWidth(int width) { mWidth = width; }

    int height() const { return mHeight; }
    void setHeight(int height) { mHeight = height; }
    
    int stride() const { return mStride; }
    void setStride(int stride) { mStride = stride; }

    bool upsideDown() const { return mUpsideDown; }
    void setUpsideDown(bool upsideDown) { mUpsideDown = upsideDown; }
    
    inline unsigned long *scanlineInt32(int line) const
    {
		return (unsigned long*)((unsigned char*)mBase + 
			((mUpsideDown?(mHeight-line-1):line) * mStride));
    }

    inline pxPixel *scanline(int line) const
    {
		return (pxPixel*)((unsigned char*)mBase + 
			((mUpsideDown?(mHeight-line-1):line) * mStride));
    }

    inline pxPixel *pixel(int x, int y) 
    { 
        return scanline(y) + x; 
    }

    pxRect size()
    {
        return pxRect(0, 0, width(), height());
    }

    pxRect bounds() const
    {
        return pxRect(0, 0, width(), height());
    }

    void fill(const pxRect& r, const pxColor& color)
    {
        // calc clip
        pxRect c = bounds();
        c.intersect(r);

	for (int i = c.top(); i < c.bottom(); i++)
	{
    	     pxPixel *p = pixel(c.left(), i);
             pxPixel *pe = p + c.width();
             while (p < pe)
	     {
                 *p++ = color;
	     }
        }
    }

    void fill(const pxColor& color)
    {
	for (int i = 0; i < height(); i++)
	{
		pxPixel *p = scanline(i);
		for (int j = 0; j < width(); j++)
		{
			*p++ = color;
		}
	}
    }

    void fillAlpha(unsigned char alpha)
    {
	    for (int i = 0; i < height(); i++)
	    {
		    pxPixel*p = scanline(i);
		    for (int j = 0; j < width(); j++)
		    {
			    p->a = alpha;
			    p++;
		    }
	    }
    }

    void blit(pxSurfaceNative s, int dstLeft, int dstRight, 
              int dstWidth, int dstHeight, 
              int srcLeft, int srcRight,
              int srcWidth, int srcHeight);
    

    inline void blit(pxSurfaceNative s, int dstLeft, int dstTop, 
              int dstWidth, int dstHeight, 
              int srcLeft, int srcRight)
    {
        blit(s, dstLeft, dstTop, dstWidth, dstHeight, srcLeft, srcRight, width(), height());
    }

    inline void blit(pxSurfaceNative s)
    {
        blit(s, 0, 0, width(), height(), 0, 0);
    }
    
    inline void blit(pxBuffer& b, int dstLeft, int dstTop, 
              int dstWidth, int dstHeight, 
              int srcLeft, int srcTop)
    {
        pxRect srcBounds = bounds();
        pxRect dstBounds = b.bounds();

        pxRect srcRect(srcLeft, srcTop, srcLeft+dstWidth, srcTop+dstHeight);
        pxRect dstRect(dstLeft, dstTop, dstLeft+dstWidth, dstTop+dstHeight);

        srcBounds.intersect(srcRect);
        dstBounds.intersect(dstRect);

        int w = pxMin<int>(srcBounds.width(), dstBounds.width());
        int h = pxMin<int>(srcBounds.height(), dstBounds.height());

        for (int y = 0; y < h; y++)
        {
            pxPixel *s = scanline(y+srcBounds.top());
            pxPixel *se = s + w;
            pxPixel *d = b.scanline(y+dstBounds.top());
            while(s < se)
            {
                *d++ = *s++;
            }
        }
    }

    inline void blit(pxBuffer b)
    {
        blit(b, 0, 0, width(), height(), 0, 0);
    }

protected:
    void* mBase;
    int mWidth;
    int mHeight;
    int mStride;
    bool mUpsideDown;
};

#endif

