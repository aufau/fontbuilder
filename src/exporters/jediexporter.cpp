#include "jediexporter.h"
#include "../fontconfig.h"
#include "../layoutdata.h"
#include <QTextCodec>

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

  uint i = 0;

  QVector<Symbol> syms{};
  auto codec = QTextCodec::codecForName("cp1250");

  foreach ( const Symbol& c, symbols() )
  {
    QChar qchar = QChar(c.id);
    QString qstring = QString(qchar);
    QByteArray bytes = codec->fromUnicode(qstring);
    unsigned char byte = bytes.at(0);

    Symbol symb = c;
    symb.id = byte;
    syms.push_back(symb);
  }

  qSort(syms);

  foreach ( const Symbol& c, syms )
  {
    if ( c.id >= 0x100 ) {
      continue;
    }

    while ( i < c.id ) {
      out.append( QByteArray( 28, '\0' ) );
      i++;
    }

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

    i++;
  }

  while ( i < 0x100 ) {
    out.append( QByteArray( 28, '\0' ) );
    i++;
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
