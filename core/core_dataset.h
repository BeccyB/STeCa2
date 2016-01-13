#ifndef CORE_DATASET_H
#define CORE_DATASET_H

#include "core_defs.h"
#include "core_image.h"
#include <QSize>
#include <QVector>
#include <QMetaType>

namespace core {

class File;
class Dataset {
public:
  Dataset(File&,QSize const&, int const* image,
          rcstr comment,
          float motorXT,  float motorYT,  float motorZT, float motorOmg, float motorTth,
          float motorPhi, float motorChi,
          double mon, double deltaTime,
          float motorPST, float motorSST, float motorOMGM);

  rcstr        getComment() const { return comment; }
  Image const& getImage()   const { return image;   }
  File  const& getFile()    const { return file;    }

private:
  File &file;
  str   comment;
  Image image;

  float
    motorXT,  motorYT,  motorZT,  motorOmg, motorTth,
    motorPhi, motorChi, motorPST, motorSST, motorOMGM;

  double mon, deltaTime;
};

class Datasets final: public QVector<QSharedPointer<Dataset>> {
public:
  Datasets();
};

}

// used in signals
typedef core::Dataset const *pcCoreDataset;
Q_DECLARE_METATYPE(pcCoreDataset)

#endif
