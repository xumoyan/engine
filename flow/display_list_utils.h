// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_DISPLAY_LIST_UTILS_H_
#define FLUTTER_FLOW_DISPLAY_LIST_UTILS_H_

#include <optional>

#include "flutter/flow/display_list.h"
#include "flutter/fml/logging.h"
#include "flutter/fml/macros.h"

#include "third_party/skia/include/core/SkMaskFilter.h"

// This file contains various utility classes to ease implementing
// a Flutter DisplayList Dispatcher, including:
//
// IgnoreAttributeDispatchHelper:
// IgnoreClipDispatchHelper:
// IgnoreTransformDispatchHelper
//     Empty overrides of all of the associated methods of Dispatcher
//     for dispatchers that only track some of the rendering operations
//
// SkPaintAttributeDispatchHelper:
//     Tracks the attribute methods and maintains their state in an
//     SkPaint object.
// SkMatrixTransformDispatchHelper:
//     Tracks the transform methods and maintains their state in a
//     (save/restore stack of) SkMatrix object.
// ClipBoundsDispatchHelper:
//     Tracks the clip methods and maintains a culling box in a
//     (save/restore stack of) SkRect culling rectangle.
//
// DisplayListBoundsCalculator:
//     A class that can traverse an entire display list and compute
//     a conservative estimate of the bounds of all of the rendering
//     operations.

namespace flutter {

// A utility class that will ignore all Dispatcher methods relating
// to the setting of attributes.
class IgnoreAttributeDispatchHelper : public virtual Dispatcher {
 public:
  void setAntiAlias(bool aa) override {}
  void setDither(bool dither) override {}
  void setInvertColors(bool invert) override {}
  void setStrokeCap(SkPaint::Cap cap) override {}
  void setStrokeJoin(SkPaint::Join join) override {}
  void setStyle(SkPaint::Style style) override {}
  void setStrokeWidth(SkScalar width) override {}
  void setStrokeMiter(SkScalar limit) override {}
  void setColor(SkColor color) override {}
  void setBlendMode(SkBlendMode mode) override {}
  void setBlender(sk_sp<SkBlender> blender) override {}
  void setShader(sk_sp<SkShader> shader) override {}
  void setImageFilter(sk_sp<SkImageFilter> filter) override {}
  void setColorFilter(sk_sp<SkColorFilter> filter) override {}
  void setPathEffect(sk_sp<SkPathEffect> effect) override {}
  void setMaskFilter(sk_sp<SkMaskFilter> filter) override {}
  void setMaskBlurFilter(SkBlurStyle style, SkScalar sigma) override {}
};

// A utility class that will ignore all Dispatcher methods relating
// to setting a clip.
class IgnoreClipDispatchHelper : public virtual Dispatcher {
  void clipRect(const SkRect& rect, SkClipOp clip_op, bool is_aa) override {}
  void clipRRect(const SkRRect& rrect, SkClipOp clip_op, bool is_aa) override {}
  void clipPath(const SkPath& path, SkClipOp clip_op, bool is_aa) override {}
};

// A utility class that will ignore all Dispatcher methods relating
// to modifying the transform.
class IgnoreTransformDispatchHelper : public virtual Dispatcher {
 public:
  void translate(SkScalar tx, SkScalar ty) override {}
  void scale(SkScalar sx, SkScalar sy) override {}
  void rotate(SkScalar degrees) override {}
  void skew(SkScalar sx, SkScalar sy) override {}
  // clang-format off
  // 2x3 2D affine subset of a 4x4 transform in row major order
  void transform2DAffine(SkScalar mxx, SkScalar mxy, SkScalar mxt,
                         SkScalar myx, SkScalar myy, SkScalar myt) override {}
  // full 4x4 transform in row major order
  void transformFullPerspective(
      SkScalar mxx, SkScalar mxy, SkScalar mxz, SkScalar mxt,
      SkScalar myx, SkScalar myy, SkScalar myz, SkScalar myt,
      SkScalar mzx, SkScalar mzy, SkScalar mzz, SkScalar mzt,
      SkScalar mwx, SkScalar mwy, SkScalar mwz, SkScalar mwt) override {}
  // clang-format on
};

// A utility class that will monitor the Dispatcher methods relating
// to the rendering attributes and accumulate them into an SkPaint
// which can be accessed at any time via paint().
class SkPaintDispatchHelper : public virtual Dispatcher {
 public:
  void setAntiAlias(bool aa) override;
  void setDither(bool dither) override;
  void setStyle(SkPaint::Style style) override;
  void setColor(SkColor color) override;
  void setStrokeWidth(SkScalar width) override;
  void setStrokeMiter(SkScalar limit) override;
  void setStrokeCap(SkPaint::Cap cap) override;
  void setStrokeJoin(SkPaint::Join join) override;
  void setShader(sk_sp<SkShader> shader) override;
  void setColorFilter(sk_sp<SkColorFilter> filter) override;
  void setInvertColors(bool invert) override;
  void setBlendMode(SkBlendMode mode) override;
  void setBlender(sk_sp<SkBlender> blender) override;
  void setPathEffect(sk_sp<SkPathEffect> effect) override;
  void setMaskFilter(sk_sp<SkMaskFilter> filter) override;
  void setMaskBlurFilter(SkBlurStyle style, SkScalar sigma) override;
  void setImageFilter(sk_sp<SkImageFilter> filter) override;

  const SkPaint& paint() { return paint_; }

 private:
  SkPaint paint_;
  bool invert_colors_ = false;
  sk_sp<SkColorFilter> color_filter_;

  sk_sp<SkColorFilter> makeColorFilter();
};

class SkMatrixSource {
 public:
  // The current full 4x4 transform matrix. Not generally needed
  // for 2D operations. See |matrix|.
  virtual const SkM44& m44() const = 0;

  // The current matrix expressed as an SkMatrix. The data held
  // in an SkMatrix is enough to perform point and rect transforms
  // assuming input coordinates have only an X and Y and an assumed
  // Z of 0 and an assumed W of 1.
  // See the block comment on the transform methods in |Dispatcher|
  // for a detailed explanation.
  virtual const SkMatrix& matrix() const = 0;
};

// A utility class that will monitor the Dispatcher methods relating
// to the transform and accumulate them into an SkMatrix which can
// be accessed at any time via matrix().
//
// This class also implements an appropriate stack of transforms via
// its save() and restore() methods so those methods will need to be
// forwarded if overridden in more than one super class.
class SkMatrixDispatchHelper : public virtual Dispatcher,
                               public virtual SkMatrixSource {
 public:
  void translate(SkScalar tx, SkScalar ty) override;
  void scale(SkScalar sx, SkScalar sy) override;
  void rotate(SkScalar degrees) override;
  void skew(SkScalar sx, SkScalar sy) override;

  // clang-format off

  // 2x3 2D affine subset of a 4x4 transform in row major order
  void transform2DAffine(SkScalar mxx, SkScalar mxy, SkScalar mxt,
                         SkScalar myx, SkScalar myy, SkScalar myt) override;
  // full 4x4 transform in row major order
  void transformFullPerspective(
      SkScalar mxx, SkScalar mxy, SkScalar mxz, SkScalar mxt,
      SkScalar myx, SkScalar myy, SkScalar myz, SkScalar myt,
      SkScalar mzx, SkScalar mzy, SkScalar mzz, SkScalar mzt,
      SkScalar mwx, SkScalar mwy, SkScalar mwz, SkScalar mwt) override;

  // clang-format on

  void save() override;
  void restore() override;

  const SkM44& m44() const override { return matrix_; }
  const SkMatrix& matrix() const override { return matrix33_; }

 protected:
  void reset();

 private:
  SkM44 matrix_;
  SkMatrix matrix33_;
  std::vector<SkM44> saved_;
};

// A utility class that will monitor the Dispatcher methods relating
// to the clip and accumulate a conservative bounds into an SkRect
// which can be accessed at any time via getCullingBounds().
//
// The subclass must implement a single virtual method matrix()
// which will happen automatically if the subclass also inherits
// from SkMatrixTransformDispatchHelper.
//
// This class also implements an appropriate stack of transforms via
// its save() and restore() methods so those methods will need to be
// forwarded if overridden in more than one super class.
class ClipBoundsDispatchHelper : public virtual Dispatcher,
                                 private virtual SkMatrixSource {
 public:
  ClipBoundsDispatchHelper() : ClipBoundsDispatchHelper(nullptr) {}

  ClipBoundsDispatchHelper(const SkRect* cull_rect)
      : has_clip_(cull_rect),
        bounds_(cull_rect && !cull_rect->isEmpty() ? *cull_rect
                                                   : SkRect::MakeEmpty()) {}

  void clipRect(const SkRect& rect, SkClipOp clip_op, bool is_aa) override;
  void clipRRect(const SkRRect& rrect, SkClipOp clip_op, bool is_aa) override;
  void clipPath(const SkPath& path, SkClipOp clip_op, bool is_aa) override;

  void save() override;
  void restore() override;

  bool has_clip() const { return has_clip_; }
  const SkRect& clip_bounds() const { return bounds_; }

 protected:
  void reset(const SkRect* cull_rect);

 private:
  bool has_clip_;
  SkRect bounds_;
  std::vector<SkRect> saved_;

  void intersect(const SkRect& clipBounds, bool is_aa);
};

class BoundsAccumulator {
 public:
  void accumulate(const SkPoint& p) { accumulate(p.fX, p.fY); }
  void accumulate(SkScalar x, SkScalar y) {
    if (min_x_ > x)
      min_x_ = x;
    if (min_y_ > y)
      min_y_ = y;
    if (max_x_ < x)
      max_x_ = x;
    if (max_y_ < y)
      max_y_ = y;
  }
  void accumulate(const SkRect& r) {
    if (r.fLeft <= r.fRight && r.fTop <= r.fBottom) {
      accumulate(r.fLeft, r.fTop);
      accumulate(r.fRight, r.fBottom);
    }
  }

  bool is_empty() const { return min_x_ >= max_x_ || min_y_ >= max_y_; }
  bool is_not_empty() const { return min_x_ < max_x_ && min_y_ < max_y_; }

  SkRect bounds() const {
    return (max_x_ > min_x_ && max_y_ > min_y_)
               ? SkRect::MakeLTRB(min_x_, min_y_, max_x_, max_y_)
               : SkRect::MakeEmpty();
  }

 private:
  SkScalar min_x_ = std::numeric_limits<SkScalar>::infinity();
  SkScalar min_y_ = std::numeric_limits<SkScalar>::infinity();
  SkScalar max_x_ = -std::numeric_limits<SkScalar>::infinity();
  SkScalar max_y_ = -std::numeric_limits<SkScalar>::infinity();
};

// This class implements all rendering methods and computes a liberal
// bounds of the rendering operations.
class DisplayListBoundsCalculator final
    : public virtual Dispatcher,
      public virtual IgnoreAttributeDispatchHelper,
      public virtual SkMatrixDispatchHelper,
      public virtual ClipBoundsDispatchHelper,
      DisplayListOpFlags {
 public:
  // Construct a Calculator to determine the bounds of a list of
  // DisplayList dispatcher method calls. Since 2 of the method calls
  // have no intrinsic size because they flood the entire clip/surface,
  // the |cull_rect| provides a bounds for them to include. If cull_rect
  // is not specified or is null, then the unbounded calls will not
  // affect the resulting bounds, but will set a flag that can be
  // queried using |isUnbounded| if an alternate plan is available
  // for such cases.
  // The flag should never be set if a cull_rect is provided.
  DisplayListBoundsCalculator(const SkRect* cull_rect = nullptr);

  void setStrokeCap(SkPaint::Cap cap) override;
  void setStrokeJoin(SkPaint::Join join) override;
  void setStyle(SkPaint::Style style) override;
  void setStrokeWidth(SkScalar width) override;
  void setStrokeMiter(SkScalar limit) override;
  void setBlendMode(SkBlendMode mode) override;
  void setBlender(sk_sp<SkBlender> blender) override;
  void setImageFilter(sk_sp<SkImageFilter> filter) override;
  void setColorFilter(sk_sp<SkColorFilter> filter) override;
  void setPathEffect(sk_sp<SkPathEffect> effect) override;
  void setMaskFilter(sk_sp<SkMaskFilter> filter) override;
  void setMaskBlurFilter(SkBlurStyle style, SkScalar sigma) override;

  void save() override;
  void saveLayer(const SkRect* bounds, bool with_paint) override;
  void restore() override;

  void drawPaint() override;
  void drawColor(SkColor color, SkBlendMode mode) override;
  void drawLine(const SkPoint& p0, const SkPoint& p1) override;
  void drawRect(const SkRect& rect) override;
  void drawOval(const SkRect& bounds) override;
  void drawCircle(const SkPoint& center, SkScalar radius) override;
  void drawRRect(const SkRRect& rrect) override;
  void drawDRRect(const SkRRect& outer, const SkRRect& inner) override;
  void drawPath(const SkPath& path) override;
  void drawArc(const SkRect& bounds,
               SkScalar start,
               SkScalar sweep,
               bool useCenter) override;
  void drawPoints(SkCanvas::PointMode mode,
                  uint32_t count,
                  const SkPoint pts[]) override;
  void drawVertices(const sk_sp<SkVertices> vertices,
                    SkBlendMode mode) override;
  void drawImage(const sk_sp<SkImage> image,
                 const SkPoint point,
                 const SkSamplingOptions& sampling,
                 bool render_with_attributes) override;
  void drawImageRect(const sk_sp<SkImage> image,
                     const SkRect& src,
                     const SkRect& dst,
                     const SkSamplingOptions& sampling,
                     bool render_with_attributes,
                     SkCanvas::SrcRectConstraint constraint) override;
  void drawImageNine(const sk_sp<SkImage> image,
                     const SkIRect& center,
                     const SkRect& dst,
                     SkFilterMode filter,
                     bool render_with_attributes) override;
  void drawImageLattice(const sk_sp<SkImage> image,
                        const SkCanvas::Lattice& lattice,
                        const SkRect& dst,
                        SkFilterMode filter,
                        bool render_with_attributes) override;
  void drawAtlas(const sk_sp<SkImage> atlas,
                 const SkRSXform xform[],
                 const SkRect tex[],
                 const SkColor colors[],
                 int count,
                 SkBlendMode mode,
                 const SkSamplingOptions& sampling,
                 const SkRect* cullRect,
                 bool render_with_attributes) override;
  void drawPicture(const sk_sp<SkPicture> picture,
                   const SkMatrix* matrix,
                   bool with_save_layer) override;
  void drawDisplayList(const sk_sp<DisplayList> display_list) override;
  void drawTextBlob(const sk_sp<SkTextBlob> blob,
                    SkScalar x,
                    SkScalar y) override;
  void drawShadow(const SkPath& path,
                  const SkColor color,
                  const SkScalar elevation,
                  bool transparent_occluder,
                  SkScalar dpr) override;

  // The DisplayList had an unbounded call with no cull rect or clip
  // to contain it. Should only be called after the stream is fully
  // dispatched.
  // Unbounded operations are calls like |drawColor| which are defined
  // to flood the entire surface, or calls that relied on a rendering
  // attribute which is unable to compute bounds (should be rare).
  // In those cases the bounds will represent only the accumulation
  // of the bounded calls and this flag will be set to indicate that
  // condition.
  bool is_unbounded() const {
    FML_DCHECK(layer_infos_.size() == 1);
    return layer_infos_.front()->is_unbounded();
  }

  SkRect bounds() const {
    FML_DCHECK(layer_infos_.size() == 1);
    if (is_unbounded()) {
      FML_LOG(INFO) << "returning partial bounds for unbounded DisplayList";
    }
    return accumulator_->bounds();
  }

 private:
  // current accumulator based on saveLayer history
  BoundsAccumulator* accumulator_;

  // A class that abstracts the information kept for a single
  // |save| or |saveLayer|, including the root information that
  // is kept as a base set of information for the DisplayList
  // at the initial conditions outside of any saveLayer.
  // See |RootLayerData|, |SaveData| and |SaveLayerData|.
  class LayerData {
   public:
    // Construct a LayerData to push on the save stack for a |save|
    // or |saveLayer| call.
    // There does not tend to be an actual layer in the case of
    // a |save| call, but in order to homogenize the handling
    // of |restore| it adds a trivial implementation of this
    // class to the stack of saves.
    // The |outer| parameter is the |BoundsAccumulator| that was
    // in use by the stream before this layer was pushed on the
    // stack and should be returned when this layer is popped off
    // the stack.
    // Some layers may substitute their own accumulator to compute
    // their own local bounds while they are on the stack.
    LayerData(BoundsAccumulator* outer) : outer_(outer), is_unbounded_(false) {}
    virtual ~LayerData() = default;

    // The accumulator to use while this layer is put in play by
    // a |save| or |saveLayer|
    virtual BoundsAccumulator* layer_accumulator() { return outer_; }

    // The accumulator to use after this layer is removed from play
    // via |restore|
    virtual BoundsAccumulator* restore_accumulator() { return outer_; }

    // The bounds of this layer. May be empty for cases like
    // a non-layer |save| call which uses the |outer_| accumulator
    // to accumulate draw calls inside of it
    virtual SkRect layer_bounds() = 0;

    // is_unbounded should be set to true if we ever encounter an operation
    // on a layer that either is unrestricted (|drawColor| or |drawPaint|)
    // or cannot compute its bounds (some effects and filters) and there
    // was no outstanding clip op at the time.
    // When the layer is restored, the outer layer may then process this
    // unbounded state by accumulating its own clip or transferring the
    // unbounded state to its own outer layer.
    // Typically the DisplayList will have been constructed with a cull
    // rect which will act as a default clip for the outermost layer and
    // the unbounded state of all sub layers will eventually be caught by
    // that cull rect so that the overall unbounded state of the entire
    // DisplayList will never be true.
    //
    // SkPicture treats these same conditions as a Nop (they accumulate
    // the SkPicture cull rect, but if it was not specified then it is an
    // empty Rect and so has no effect on the bounds).
    // If the Calculator object accumulates this flag into the root layer,
    // then at least we can make the caller aware of that exceptional
    // condition via the |DisplayListBoundsCalculator::isUnbounded| call.
    //
    // Flutter is unlikely to ever run into this as the Dart mechanisms
    // all supply a non-null cull rect for all Dart Picture objects,
    // even if that cull rect is kGiantRect.
    void set_unbounded() { is_unbounded_ = true; }

    // |is_unbounded| should be called after |getLayerBounds| in case
    // a problem was found during the computation of those bounds,
    // the layer will have one last chance to flag an unbounded state.
    bool is_unbounded() const { return is_unbounded_; }

   private:
    BoundsAccumulator* outer_;
    bool is_unbounded_;

    FML_DISALLOW_COPY_AND_ASSIGN(LayerData);
  };

  // An intermediate implementation class that handles keeping
  // a local accumulator for the layer, used by both |RootLayerData|
  // and |SaveLayerData|.
  class AccumulatorLayerData : public LayerData {
   public:
    BoundsAccumulator* layer_accumulator() override {
      return &layer_accumulator_;
    }

    SkRect layer_bounds() override {
      // Even though this layer might be unbounded, we still
      // accumulate what bounds we have as the unbounded condition
      // may be contained at a higher level and we at least want to
      // account for the bounds that we do have.
      return layer_accumulator_.bounds();
    }

   protected:
    using LayerData::LayerData;
    ~AccumulatorLayerData() = default;

   private:
    BoundsAccumulator layer_accumulator_;
  };

  // Used as the initial default layer info for the Calculator.
  class RootLayerData final : public AccumulatorLayerData {
   public:
    RootLayerData() : AccumulatorLayerData(nullptr) {}
    ~RootLayerData() = default;

   private:
    FML_DISALLOW_COPY_AND_ASSIGN(RootLayerData);
  };

  // Used for |save|
  class SaveData final : public LayerData {
   public:
    using LayerData::LayerData;
    ~SaveData() = default;

    SkRect layer_bounds() override { return SkRect::MakeEmpty(); }

   private:
    FML_DISALLOW_COPY_AND_ASSIGN(SaveData);
  };

  // Used for |saveLayer|
  class SaveLayerData final : public AccumulatorLayerData {
   public:
    SaveLayerData(BoundsAccumulator* outer,
                  sk_sp<SkImageFilter> filter,
                  bool paint_nops_on_transparency)
        : AccumulatorLayerData(outer), layer_filter_(std::move(filter)) {
      if (!paint_nops_on_transparency) {
        set_unbounded();
      }
    }
    ~SaveLayerData() = default;

    SkRect layer_bounds() override {
      SkRect bounds = AccumulatorLayerData::layer_bounds();
      if (!ComputeFilteredBounds(bounds, layer_filter_.get())) {
        set_unbounded();
      }
      return bounds;
    }

   private:
    sk_sp<SkImageFilter> layer_filter_;

    FML_DISALLOW_COPY_AND_ASSIGN(SaveLayerData);
  };

  std::vector<std::unique_ptr<LayerData>> layer_infos_;

  static constexpr SkScalar kMinStrokeWidth = 0.01;

  skstd::optional<SkBlendMode> blend_mode_ = SkBlendMode::kSrcOver;
  sk_sp<SkColorFilter> color_filter_;

  SkScalar half_stroke_width_ = kMinStrokeWidth;
  SkScalar miter_limit_ = 4.0;
  SkPaint::Style style_ = SkPaint::Style::kFill_Style;
  bool join_is_miter_ = true;
  bool cap_is_square_ = false;
  sk_sp<SkImageFilter> image_filter_;
  sk_sp<SkPathEffect> path_effect_;
  sk_sp<SkMaskFilter> mask_filter_;
  SkScalar mask_sigma_pad_ = 0.0;

  bool paint_nops_on_transparency();

  static bool ComputeFilteredBounds(SkRect& rect, SkImageFilter* filter);
  bool AdjustBoundsForPaint(SkRect& bounds, DisplayListAttributeFlags flags);

  void AccumulateUnbounded();
  void AccumulateRect(const SkRect& rect, DisplayListAttributeFlags flags) {
    SkRect bounds = rect;
    AccumulateRect(bounds, flags);
  }
  void AccumulateRect(SkRect& rect, DisplayListAttributeFlags flags);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_DISPLAY_LIST_UTILS_H_
