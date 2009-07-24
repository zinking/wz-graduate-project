#ifndef _LSYSTEMIMPL_H_
#define _LSYSTEMIMPL_H_

class LSystemRenderContext {
public:

  // the rule set (map from id to new LSystemNode)
  const LSystemRuleSet& rules;

  // quadric we use to render cylinders
  GLUquadric* const quadric;

  // maximum depth to render to
  const unsigned int maxDepth;

  // length of a cylinder
  double lengthScale;
  
  // radius of a cylinder
  double radiusScale;

  // the current depth to render to
  unsigned int curDepth;

  const LSystemNode* lookup(char c) const;

  LSystemRenderContext(const LSystemRuleSet& rules, 
		       GLUquadric* q, 
		       unsigned int maxDepth);

};

//////////////////////////////////////////////////////////////////////

class LSystemNode {
public:

  enum NodeType {
    TypeGroup,
    TypeRotation,
    TypeScale,
    TypeSegment,
    TypeLeaf,
  };

  virtual ~LSystemNode();

  virtual NodeType type() const =0;

  virtual void render(LSystemRenderContext& context) const =0;

protected:
  LSystemNode();

};

//////////////////////////////////////////////////////////////////////

class LSystemGroupNode: public LSystemNode {
public:

  LSystemGroupNode();

  // note: deletes children on destruction
  virtual ~LSystemGroupNode();

  virtual NodeType type() const;

  virtual void render(LSystemRenderContext& context) const;
  
  bool restoreState;
  LSystemNodeArray children;

};

//////////////////////////////////////////////////////////////////////

class LSystemRotationNode: public LSystemNode {
public:

  enum RotationAxis {
    XAxis,
    YAxis,
    ZAxis,
  };

  LSystemRotationNode();
  virtual ~LSystemRotationNode();

  virtual NodeType type() const;

  virtual void render(LSystemRenderContext& context) const;

  RotationAxis axis;
  double angleDegrees;

};

//////////////////////////////////////////////////////////////////////

class LSystemScaleNode: public LSystemNode {
public:

  enum ScaleType {
    ScaleLength,
    ScaleRadius,
    ScaleBoth,
  };

  LSystemScaleNode();
  virtual ~LSystemScaleNode();

  virtual NodeType type() const;

  virtual void render(LSystemRenderContext& context) const;

  ScaleType scaleType;
  double scaleFraction;
  
};

//////////////////////////////////////////////////////////////////////

class LSystemSegmentNode: public LSystemNode {
public:

  LSystemSegmentNode();
  virtual ~LSystemSegmentNode();

  virtual NodeType type() const;

  virtual void render(LSystemRenderContext& context) const;

  char id;
  double endScaleFraction;
  
};

//////////////////////////////////////////////////////////////////////

class LSystemLeafNode: public LSystemNode {
public:

  LSystemLeafNode();
  virtual ~LSystemLeafNode();

  virtual NodeType type() const;
  virtual void render(LSystemRenderContext& context) const;

};

#endif
