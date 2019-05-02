/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/** \file
 * \ingroup collada
 */

#include "BLI_math.h"
#include "BLI_sys_types.h"

#include "BKE_object.h"

#include "TransformWriter.h"

void TransformWriter::add_node_transform(COLLADASW::Node &node,
                                         float mat[4][4],
                                         float parent_mat[4][4])
{
  //bool limit_precision = export_settings.limit_precision;
  float loc[3], rot[3], scale[3];
  float local[4][4];

  if (parent_mat) {
    float invpar[4][4];
    invert_m4_m4(invpar, parent_mat);
    mul_m4_m4m4(local, invpar, mat);
  }
  else {
    copy_m4_m4(local, mat);
  }

  double dmat[4][4];
  UnitConverter *converter = new UnitConverter();
  converter->mat4_to_dae_double(dmat, local);
  delete converter;

  bc_decompose(local, loc, rot, NULL, scale);

  if (node.getType() == COLLADASW::Node::JOINT) {
    // XXX Why are joints handled differently ?
    node.addMatrix("transform", dmat);
  }
  else {
    add_transform(node, loc, rot, scale);
  }
}

void TransformWriter::add_node_transform_ob(COLLADASW::Node &node,
                                            Object *ob,
                                            BCExportSettings &export_settings)
{
  BC_export_transformation_type transformation_type =
      export_settings.get_export_transformation_type();
  bool limit_precision = export_settings.get_limit_precision();

  /* Export the local Matrix (relative to the object parent, be it an object, bone or vertex(-tices)) */
  Matrix f_obmat;
  BKE_object_matrix_local_get(ob, f_obmat);

  //if (export_settings.is_export_root(ob)) {
  //  if (export_settings.get_apply_global_orientation()) {
  //      // do nothing. The rotation happens in the object data
  //  }
  //  else {
  bc_add_global_transform(f_obmat, export_settings.get_global_transform());
  //  }
  //}

  switch (transformation_type) {
    case BC_TRANSFORMATION_TYPE_MATRIX: {
      UnitConverter converter;
      double d_obmat[4][4];
      converter.mat4_to_dae_double(d_obmat, f_obmat);
      if (limit_precision)
        bc_sanitize_mat(d_obmat, LIMITTED_PRECISION);
      node.addMatrix("transform", d_obmat);
      break;
    }
    case BC_TRANSFORMATION_TYPE_TRANSROTLOC: {
      float loc[3], rot[3], scale[3];
      bc_decompose(f_obmat, loc, rot, NULL, scale);
      if (limit_precision) {
        bc_sanitize_v3(loc, LIMITTED_PRECISION);
        bc_sanitize_v3(rot, LIMITTED_PRECISION);
        bc_sanitize_v3(scale, LIMITTED_PRECISION);
      }
      add_transform(node, loc, rot, scale);
      break;
    }
  }
}

void TransformWriter::add_node_transform_identity(COLLADASW::Node &node)
{
  float loc[3] = {0.0f, 0.0f, 0.0f}, scale[3] = {1.0f, 1.0f, 1.0f}, rot[3] = {0.0f, 0.0f, 0.0f};
  add_transform(node, loc, rot, scale);
}

void TransformWriter::add_transform(COLLADASW::Node &node,
                                    float loc[3],
                                    float rot[3],
                                    float scale[3])
{
#if 0
  node.addRotateZ("rotationZ", COLLADABU::Math::Utils::radToDegF(rot[2]));
  node.addRotateY("rotationY", COLLADABU::Math::Utils::radToDegF(rot[1]));
  node.addRotateX("rotationX", COLLADABU::Math::Utils::radToDegF(rot[0]));
#endif
  node.addTranslate("location", loc[0], loc[1], loc[2]);
  node.addRotateZ("rotationZ", RAD2DEGF(rot[2]));
  node.addRotateY("rotationY", RAD2DEGF(rot[1]));
  node.addRotateX("rotationX", RAD2DEGF(rot[0]));
  node.addScale("scale", scale[0], scale[1], scale[2]);
}
