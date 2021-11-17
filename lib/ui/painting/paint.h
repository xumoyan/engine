// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_PAINT_H_
#define FLUTTER_LIB_UI_PAINTING_PAINT_H_

#include "third_party/skia/include/core/SkPaint.h"
#include "third_party/tonic/converter/dart_converter.h"

#include "flow/display_list.h"

namespace flutter {

class Paint {
 public:
  Paint() = default;
  Paint(Dart_Handle paint_objects, Dart_Handle paint_data);

  const SkPaint* paint(SkPaint& paint) const;

  /// Synchronize the Dart properties to the display list according
  /// to the attribute flags that indicate which properties are needed.
  /// The return value indicates if the paint was non-null and can
  /// either be DCHECKed or used to indicate to the DisplayList
  /// draw operation whether or not to use the synchronized attributes
  /// (mainly the drawImage and saveLayer methods).
  bool sync_to(DisplayListBuilder* builder,
               const DisplayListAttributeFlags& flags) const;

  bool isNull() const { return Dart_IsNull(paint_data_); }
  bool isNotNull() const { return !Dart_IsNull(paint_data_); }

 private:
  friend struct tonic::DartConverter<Paint>;

  Dart_Handle paint_objects_;
  Dart_Handle paint_data_;
};

// The PaintData argument is a placeholder to receive encoded data for Paint
// objects. The data is actually processed by DartConverter<Paint>, which reads
// both at the given index and at the next index (which it assumes is a byte
// data for a Paint object).
class PaintData {};

}  // namespace flutter

namespace tonic {

template <>
struct DartConverter<flutter::Paint> {
  static flutter::Paint FromArguments(Dart_NativeArguments args,
                                      int index,
                                      Dart_Handle& exception);
};

template <>
struct DartConverter<flutter::PaintData> {
  static flutter::PaintData FromArguments(Dart_NativeArguments args,
                                          int index,
                                          Dart_Handle& exception);
};

}  // namespace tonic

#endif  // FLUTTER_LIB_UI_PAINTING_PAINT_H_
