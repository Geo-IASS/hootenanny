#ifndef __SCHEMA_CHECKER_H__
#define __SCHEMA_CHECKER_H__

// hoot
#include <hoot/core/schema/OsmSchema.h>

namespace hoot
{

/**
 * The SchemaChecker checks if SchemaVertex is an unknown type or has no geometries listed.
 */
class SchemaChecker
{
public:
  SchemaChecker(OsmSchema& osmSchema);

  ~SchemaChecker() {}

  /**
   * Print out error message if schemavertex is unkonw vertex type.
   */

  void checkUnknownVertexType();

  /**
   * Print out error message if schemavertex has empty geometry.
   */
  void checkEmptyGeometry();

  /**
   * Print out error message if schemavertex has empty geometry or geometries are empty
   */
  void check();

  /**
   * Retrun true if schemavertex is unkonw vertex type.
   */

  static bool isUnknownVertexType(const SchemaVertex& schemaVertex) { return schemaVertex.isValid(); }

  /**
   * Return true if schemavertex has empty geometry.
   */
  static bool isEmptyGeometry(const SchemaVertex& schemaVertex) { return schemaVertex.geometries > 0; }

private:
  vector<SchemaVertex> _schemaVertexList;
};

}

#endif // __SCHEMA_CHECKER_H__
