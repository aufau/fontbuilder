#ifndef JEDIEXPORTER_H
#define JEDIEXPORTER_H

#include "../abstractexporter.h"

typedef struct
{
  float X;
  float Y;
} zglTPoint2D;

typedef struct
{
  short Page;
  short Width;
  short Height;
  short advance;
  short ShiftX;
  int ShiftY;
  float s;
  float t;
  float s2;
  float t2;
} zglTCharDesc;

class JediExporter : public AbstractExporter
{
Q_OBJECT
public:
    explicit JediExporter(QObject *parent = 0);
protected:
    virtual bool Export(QByteArray& out);
signals:

public slots:

};

#endif // JEDIEXPORTER_H
