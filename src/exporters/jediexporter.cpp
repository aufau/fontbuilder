#include "jediexporter.h"
#include "../fontconfig.h"
#include "../layoutdata.h"

JediExporter::JediExporter(QObject *parent) :
    AbstractExporter(parent)
{
    setExtension("fontdat");
}

bool JediExporter::Export(QByteArray& out) {
  short MaxPointSize = fontConfig()->size();
  short MaxHeight = 0;
  short MaxAscender = 0;
  short MaxDescender = 0;
  int PageWidth = texWidth();
  int PageHeight = texHeight();
  unsigned char Padding[4] = { 0, 0, 0, 0 };
  zglTCharDesc CharDesc;

  int i = 0;
  sortSymbols();

  foreach ( const Symbol& c, symbols() )
  {
    int id = c.id;

    if ( i < id ) {
      for ( int j = 0; j < (id - i) * 28; j++ ) {
        out.append( "\0", 1 );
        //out.append( "\0", (id - i) * 28 );
      }
      i = id;
    }
    i++;

    if ( c.placeH - c.offsetY > MaxDescender )
      MaxDescender = c.placeH - c.offsetY;
    if ( c.offsetY > MaxAscender )
      MaxAscender = c.offsetY;

    float u = 1.f / PageWidth, v = 1.f / PageHeight;

    CharDesc.Width = c.placeW;
    CharDesc.Height = c.placeH;
    CharDesc.advance = c.advance;
    CharDesc.ShiftX = c.offsetX;
    CharDesc.ShiftY = c.offsetY;

    CharDesc.s = (float)u * c.placeX;
    CharDesc.t = (float)v * c.placeY;
    CharDesc.s2 = (float)u * ( c.placeX + c.placeW );
    CharDesc.t2 = (float)v * ( c.placeY + c.placeH );

    out.append( (char*)&CharDesc.Width, 2 );
    out.append( (char*)&CharDesc.Height, 2 );
    out.append( (char*)&CharDesc.advance, 2 );
    out.append( (char*)&CharDesc.ShiftX, 2 );
    out.append( (char*)&CharDesc.ShiftY, 4 );
    out.append( (char*)&CharDesc.s, 4 );
    out.append( (char*)&CharDesc.t, 4 );
    out.append( (char*)&CharDesc.s2, 4 );
    out.append( (char*)&CharDesc.t2, 4 );
  }

  MaxPointSize = metrics().height;
  MaxHeight = MaxAscender + MaxDescender;

  out.append( (char*)&MaxPointSize, 2 );
  out.append( (char*)&MaxHeight, 2 );
  out.append( (char*)&MaxAscender, 2 );
  out.append( (char*)&MaxDescender, 2 );
  out.append( (char*)&Padding, 4 );

  return true;
}


AbstractExporter* JediExporterFactoryFunc (QObject* parent) {
    return new JediExporter(parent);
}
