#include "lsystem.h"
#include "lsystem_impl.h"
#include <sstream>
#include <stdexcept>
#include <ctype.h>
#include <iostream>
#include "nvImage.h"

LSystemRenderContext::LSystemRenderContext(const LSystemRuleSet& r,
					   GLUquadric* q,
					   unsigned int md): 
  rules(r), 
  quadric(q), 
  maxDepth(md), 
  lengthScale(1.0), 
  radiusScale(0.15), 
  curDepth(0) {}

const LSystemNode* LSystemRenderContext::lookup(char c) const {
  LSystemRuleSet::const_iterator i = rules.find(c);
  if (i == rules.end()) { return 0; }
  return i->second;
}

//////////////////////////////////////////////////////////////////////

LSystemNode::LSystemNode() {}
LSystemNode::~LSystemNode() {}

//////////////////////////////////////////////////////////////////////

LSystemGroupNode::LSystemGroupNode() {
  restoreState = false;
}

LSystemGroupNode::~LSystemGroupNode() {
  for (unsigned int i=0; i<children.size(); ++i) {
    delete children[i];
  }
  children.clear();
}

LSystemNode::NodeType LSystemGroupNode::type() const {
  return TypeGroup;
}

//////////////////////////////////////////////////////////////////////

LSystemRotationNode::LSystemRotationNode() {
  axis = XAxis;
  angleDegrees = 0.0;
}

LSystemRotationNode::~LSystemRotationNode() { }

LSystemNode::NodeType LSystemRotationNode::type() const {
  return TypeRotation;
}

//////////////////////////////////////////////////////////////////////

LSystemScaleNode::LSystemScaleNode() {
  scaleType = ScaleBoth;
  scaleFraction = 1.0;
}

LSystemScaleNode::~LSystemScaleNode() { }

LSystemNode::NodeType LSystemScaleNode::type() const {
  return TypeScale;
}

//////////////////////////////////////////////////////////////////////

LSystemSegmentNode::LSystemSegmentNode() {
  id = 0;
  endScaleFraction = 1.0;
}

LSystemSegmentNode::~LSystemSegmentNode() { }

LSystemNode::NodeType LSystemSegmentNode::type() const {
  return TypeSegment;
}

//////////////////////////////////////////////////////////////////////

LSystemLeafNode::LSystemLeafNode() { }
LSystemLeafNode::~LSystemLeafNode() { }

LSystemNode::NodeType LSystemLeafNode::type() const { return TypeLeaf; }

//////////////////////////////////////////////////////////////////////

LSystem::LSystem() { 
  _root = 0;
  _quadric = 0;
  _maxDepth = 1;
  display_index =  glGenLists(1);
}

LSystem::LSystem(const std::string& axiom,
		 const std::vector<std::string>& rulestrs,
		 unsigned int maxDepth) {

  _root = 0;
  _quadric = 0;
  _maxDepth = maxDepth;

  display_index =  glGenLists(1);



  parse(axiom, rulestrs);
  _initializeShader();

}

void LSystem::_initializeShader(){

	shader = SM.loadfromFile("normal_map.vert","normal_map.frag");
	if (shader==0) std::cout << "Error Loading, compiling or linking shader\n";

	/*nm_shader = new GLSLProgram("normal_map.vert","normal_map.frag");
	glBindTexture( GL_TEXTURE_2D, 1 );*/
	//nm_shader->sendUniform("textureMap1",1);
	//nm_shader->sendUniform("normalMap1",2);

	printf("loading texture...\n");


	nv::Image barkTex;
	nv::Image normalTex;
	nv::Image leafTex;

	glGenTextures(1, &barktexture);	
	glGenTextures(1, &normaltexture );
	glGenTextures(1, &leaftexture );
	if( barkTex.loadImageFromFile( "bark.png")   ){
		glBindTexture(GL_TEXTURE_2D, barktexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, barkTex.getWidth(), barkTex.getHeight(), 0, 
			GL_RGB,GL_UNSIGNED_BYTE, barkTex.getLevel(0) );
	}

	if( normalTex.loadImageFromFile("normal.png") ){
		glBindTexture(GL_TEXTURE_2D, normaltexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, normalTex.getWidth(), normalTex.getHeight(), 0, 
			GL_RGB,GL_UNSIGNED_BYTE, normalTex.getLevel(0) );

	}
	if( leafTex.loadImageFromFile("leaf.png") ){
		glBindTexture(GL_TEXTURE_2D, leaftexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, leafTex.getWidth(), leafTex.getHeight(), 0, 
			GL_RGB,GL_UNSIGNED_BYTE, leafTex.getLevel(0) );

	}

}

LSystem::LSystem(const char* axiom,
		 const char* const rulestrs[],
		 unsigned int maxDepth) {
  _root = 0;
  _quadric = 0;
  _maxDepth = maxDepth;

  parse(axiom, rulestrs);
  display_index =  glGenLists(1);
  _initializeShader();

}

void LSystem::parse(const std::string& axiom,
		    const std::vector<std::string>& rulestrs) {

  try {
    std::istringstream a(axiom);
    _root = _parseExpression(a);
    for (unsigned int i=0; i<rulestrs.size(); ++i) {
      char id;
      std::istringstream r(rulestrs[i]);
      LSystemNode* node = _parseRule(r, id);
      _rules[id] = node;
    }
  } catch (...) {
    clear();
    throw;
  }

}

void LSystem::parse(const char* axiom,
		    const char* const rulestrs[]) {

  try {
    std::istringstream a(axiom);
    _root = _parseExpression(a);
    for (unsigned int i=0; rulestrs && rulestrs[i]; ++i) {
      char id;
      std::istringstream r(rulestrs[i]);
      LSystemNode* node = _parseRule(r, id);
      _rules[id] = node;
    }
  } catch (...) {
    clear();
    throw;
  }

}

LSystem::~LSystem() {

  clear();

}


void LSystem::clear() {

  if (_quadric) {
    gluDeleteQuadric(_quadric);
    _quadric = 0;
  }

  delete _root;
  _root = 0;

  for (LSystemRuleSet::iterator i=_rules.begin(); i!=_rules.end(); ++i) {
    delete i->second;
  }
  _rules.clear();

}

LSystemNode* LSystem::_parseExpression(std::istream& istr) {

  double number = 0.0;
  bool haveNumber = false;

  LSystemGroupNode* rval = new LSystemGroupNode();
  std::vector<LSystemGroupNode*> groupStack;
  LSystemGroupNode* curGroup = rval;
  groupStack.push_back(rval);

  // 1 character lookahead tokenizer

  try {

    // while we have characters
    char cur = 0;

    while ( (cur = istr.peek()) && (cur != EOF) && !istr.fail() ) {
      if (isspace(cur)) {
	cur = istr.get(); // ignore whitespace
      } else if (isdigit(cur) || cur == '-') {
	// number. this is basically always allowed, except after a number
	if (haveNumber) {
	  throw std::runtime_error("parse error: expected numeric operation");
	} else if (!(istr >> number)) {
	  throw std::runtime_error("parse error: expected number");
	}
	haveNumber = true;
      } else {
	cur = istr.get();
	switch (cur) {
	case '[':
	  // push -- can't happen after number
	  if (haveNumber) {
	    throw std::runtime_error("parse error: expected numeric operation");
	  } else {
	    LSystemGroupNode* newGroup = new LSystemGroupNode();
	    newGroup->restoreState = true;
	    curGroup->children.push_back(newGroup);
	    curGroup = newGroup;
	    groupStack.push_back(newGroup);
	    //std::cerr << "pushed group!\n";
	  }
	  break;
	case ']':
	  // pop -- can't happen after number
	  if (haveNumber) {
	    throw std::runtime_error("parse error: expected numeric operation");
	  } else if (groupStack.size() <= 1) {
	    throw std::runtime_error("parse error: unbalanced brackets");
	  }
	  groupStack.pop_back();
	  curGroup = groupStack.back();
	  //std::cerr << "popped group!\n";
	  break;
	case 'L':
	case 'R':
	case 'S':
	  // must have number
	  if (!haveNumber) {
	    throw std::runtime_error("parse error: expected number before L/R/S");
	  } else {
	    LSystemScaleNode* scl = new LSystemScaleNode();
	    scl->scaleType = (cur == 'L' ? LSystemScaleNode::ScaleLength :
			      (cur == 'R' ? LSystemScaleNode::ScaleRadius :
			       LSystemScaleNode::ScaleBoth));
	    if (number < 0) {
	      scl->scaleFraction = -100.0 / number;
	    } else {
	      scl->scaleFraction = number / 100.0;
	    }
	    curGroup->children.push_back(scl);
	    haveNumber = false;
	    //std::cerr << "scale = " << scl->scaleFraction << " " << cur << "\n";
	  }
	  break;
	case 'X':
	case 'Y':
	case 'Z':
	  // must have number
	  if (!haveNumber) {
	    throw std::runtime_error("parse error: expected number before L/R/S");
	  } else {
	    LSystemRotationNode* rot = new LSystemRotationNode();
	    rot->axis = (cur == 'X' ? LSystemRotationNode::XAxis :
			 (cur == 'Y' ? LSystemRotationNode::YAxis :
			  LSystemRotationNode::ZAxis));
	    rot->angleDegrees = number;
	    curGroup->children.push_back(rot);
	    haveNumber = false;
	    //std::cerr << "rot = " << number << " " << cur << "\n";
	  }
	  break;
	case '*':
	  if (haveNumber) {
	    throw std::runtime_error("parse error: expected numeric operation");
	  } else {
	    LSystemLeafNode* leaf = new LSystemLeafNode();
	    curGroup->children.push_back(leaf);
	  }
	  break;
	default:
	  if (isalpha(cur) && isupper(cur)) {
	    // number is optional
	    LSystemSegmentNode* segment = new LSystemSegmentNode();
	    segment->id = cur;
	    curGroup->children.push_back(segment);
	    if (haveNumber) {
	      segment->endScaleFraction = number / 100.0;
	      haveNumber = false;
	      //std::cerr << "segment = " << number << " " << cur << "\n";
	    } else {
	      //std::cerr << "segment = " << cur << "\n";
	    }
	  } else {
	    // parse exception
	    throw std::runtime_error("parse error: illegal character");
	  }
	  break;
	}
      }
    }
    
    if (groupStack.size() != 1) {
      throw std::runtime_error("parse error: unbalanced brackets");
    }

  } catch (...) {

    delete rval;
    throw;

  }

  if (rval->children.size() == 1) {
    LSystemNode* single = rval->children.front();
    rval->children.clear();
    delete rval;
    //std::cerr << "returning single child!\n";
    return single;
  }

  //std::cerr << "returning group child!\n";
  return rval;

}

LSystemNode* LSystem::_parseRule(std::istream& istr, char& id) {
  
  // eat WS
  char cur;

  while (isspace(cur = istr.get()) && !istr.fail());

  if (!isalpha(cur) || !isupper(cur)) {
    throw std::runtime_error("parse error: expected ID");
  }
  id = cur;

  while (isspace(cur = istr.get()) && !istr.fail());
  
  if (cur != ':') {
    throw std::runtime_error("parse error: expected colon");
  }

  //std::cerr << "rule id = " << id << "\n";

  return _parseExpression(istr);

}

void LSystem::setMaxDepth(unsigned int i) {
  _maxDepth = i;
}

unsigned int LSystem::maxDepth() const {
	return _maxDepth;
}

