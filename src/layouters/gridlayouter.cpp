/**
 * Copyright (c) 2010-2010 Andrey AndryBlack Kunitsyn
 * email:support.andryblack@gmail.com
 *
 * Report bugs and download new versions at http://code.google.com/p/fontbuilder
 *
 * This software is distributed under the MIT License.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "gridlayouter.h"
#include "../rendererdata.h"
#include "../layoutdata.h"

GridLayouter::GridLayouter(QObject *parent) :
    AbstractLayouter(parent)
{
}

#define GRID_ROWS 16
#define GRID_COLS 16

void GridLayouter::PlaceImages(const QVector<LayoutChar>& chars) {
    if (chars.isEmpty()) return;

    int min_y = chars.front().y;
    int min_x = chars.front().x;
    int max_y = chars.front().y;
    int max_x = chars.front().x;
    int max_w, max_h;

    foreach (const LayoutChar& c, chars) {
        if (c.x < min_x)
            min_x = c.x;
        if (c.x + c.w > max_x)
            max_x = c.x + c.w;
        if (c.y < min_y)
            min_y = c.y;
        if (c.y + c.h > max_y)
            max_y = c.y + c.h;
    }

    max_w = max_x - min_x;
    max_h = max_y - min_y;

    resize(max_w * GRID_COLS, max_h * GRID_ROWS);

    foreach (const LayoutChar& c, chars) {
        LayoutChar l = c;
        int column = c.symbol % GRID_COLS;
        int row = c.symbol / GRID_COLS ;

        l.x = column * width() / GRID_COLS + l.x - min_x;
        l.y = row * height() / GRID_ROWS + l.y - min_y;
        place(l);
    }
}


AbstractLayouter* GridLayouterFactoryFunc (QObject* parent) {
    return new GridLayouter(parent);
}
