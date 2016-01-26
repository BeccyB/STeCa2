#include "core_file.h"
#include "core_loadcaress.h"

namespace core {

File::File(): File(NULL_STR) {
}

File::File(rcstr fileName): info(fileName) {
}

File::~File() {
}

QByteArray File::peek(uint maxLen) {
  QFile file(info.filePath());
  file.open(QFile::ReadOnly);
  return file.read(maxLen); // returns an empty QByteArray if there is an error
}

void File::load() THROWS {
  RUNTIME_CHECK(info.exists(), "File " % info.filePath() % " does not exist");

  static QByteArray const caressHead("\020\012DEFCMD DAT");
  if (caressHead == peek(caressHead.size())) {
    // looks like Caress
    loadCaress(info.filePath(),datasets);
  } else {
    THROW("unknown file type");
  }

  RUNTIME_CHECK(!datasets.isEmpty(), "File " % info.filePath() % " is empty");
}

void File::sumDatasets() {
  while (datasets.count() > 1) {
    datasets[0]->image.sumIntensities(datasets.last()->image.getData());
    datasets.removeLast();
  }
}

Files::Files() {
}

}

// eof
