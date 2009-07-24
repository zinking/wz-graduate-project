#ifndef _LSYSTEM3D_H_
#define _LSYSTEM3D_H_

#include <string>
#include <iostream>
#include <vector>
#include <map>

#ifdef _WIN32
#include <windows.h>
#endif
#include "glsl.h"
#include "GLSLProgram.hpp"
#include <GL/gl.h>
#include <GL/glu.h>


class LSystemNode;

typedef std::vector<LSystemNode*> LSystemNodeArray;
typedef std::map<char, LSystemNode*> LSystemRuleSet;

using namespace cwc;


class LSystem {
public:

  LSystem();

  /** Construct from axiom and rules. */
  LSystem(const std::string& axiom,
	  const std::vector<std::string>& rules,
	  unsigned int maxDepth = 1);

  /** Construct from axiom and rules. */
  LSystem(const char* axiom,
	  const char* const rules[],
	  unsigned int maxDepth = 1);

  virtual ~LSystem();

  /** Parse an L-System description file. */
  void parse(const std::string& axiom,
	     const std::vector<std::string>& rules);

  /** Parse an L-System. 
   *  The rules array must be null-terminated (i.e., the last
   *  string in the rules array must be the NULL pointer.
   */
  void parse(const char* axiom,
	     const char* const rules[]);

  /** The maximum recursion depth (default 1) */
  unsigned int maxDepth() const;

  /** Set the maximum depth of recursion. */
  void setMaxDepth(unsigned int d);

  /** The root L-System node (see lsystem_impl.h) */
  const LSystemNode* root() const;

  /** Set of replacement rules. */
  const LSystemRuleSet& rules() const;

  /** Clear the L-System and delete all private data. */
  void clear();

  /** Render the L-System to the currently specified depth
   *  on the current OpenGL context. */
  void renderGL() const;

  void generatelist()const;
  glShader* getShader()const{ return shader; }

private:

  LSystemNode* _root;
  LSystemRuleSet _rules;
  mutable GLUquadric* _quadric;
  unsigned int _maxDepth;
  GLuint display_index;

  LSystemNode* _parseExpression(std::istream& istr);
  LSystemNode* _parseRule(std::istream& istr, char& id);
  void _initializeShader();


  glShaderManager SM;
  glShader* shader;

  GLuint barktexture;
  GLuint normaltexture;
  GLuint leaftexture;
	

};


#endif
